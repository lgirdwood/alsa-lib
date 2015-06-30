/*
  Copyright(c) 2014-2015 Intel Corporation
  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  Authors: Mengdong Lin <mengdong.lin@intel.com>
           Yao Jin <yao.jin@intel.com>
           Liam Girdwood <liam.r.girdwood@linux.intel.com>
*/

#include "list.h"
#include "tplg_local.h"

/* verbose output detailing each object size and file position */
static void verbose(snd_tplg_t *tplg, const char *fmt, ...)
{
	int offset;
	va_list va;

	if (!tplg->verbose)
		return;

	offset = lseek(tplg->out_fd, 0, SEEK_CUR);

	va_start(va, fmt);
	fprintf(stdout, "0x%6.6x/%6.6d -", offset, offset);
	vfprintf(stdout, fmt, va);
	va_end(va);
}

/* write out block header to output file */
static int write_block_header(snd_tplg_t *tplg, unsigned int type,
	unsigned int vendor_type, unsigned int version, unsigned int index,
	size_t payload_size, int count)
{
	struct snd_soc_tplg_hdr hdr;
	size_t bytes;
	int offset = lseek(tplg->out_fd, 0, SEEK_CUR);

	memset(&hdr, 0, sizeof(hdr));
	hdr.magic = SND_SOC_TPLG_MAGIC;
	hdr.abi = SND_SOC_TPLG_ABI_VERSION;
	hdr.type = type;
	hdr.vendor_type = vendor_type;
	hdr.version = version;
	hdr.payload_size = payload_size;
	hdr.index = index;
	hdr.size = sizeof(hdr);
	hdr.count = count;

	/* make sure file offset is aligned with the calculated HDR offset */
	if ((unsigned int)offset != tplg->next_hdr_pos) {
		fprintf(stderr, "error: New header is at offset 0x%x but file"
			" offset 0x%x is %s by %d bytes\n",
			tplg->next_hdr_pos, offset,
			(unsigned int)offset > tplg->next_hdr_pos ? "ahead" : "behind",
			abs(offset - tplg->next_hdr_pos));
		exit(-EINVAL);
	}

	verbose(tplg, " header type %d size 0x%lx/%ld vendor %d "
		"version %d\n", type, (long unsigned int)payload_size,
		(long int)payload_size, vendor_type, version);

	tplg->next_hdr_pos += hdr.payload_size + sizeof(hdr);

	bytes = write(tplg->out_fd, &hdr, sizeof(hdr));
	if (bytes != sizeof(hdr)) {
		fprintf(stderr, "error: can't write section header %lu\n",
			(long unsigned int)bytes);
		return bytes;
	}

	return bytes;
}

static int write_elem_block(snd_tplg_t *tplg,
	struct list_head *base, int size, int tplg_type, const char *obj_name)
{
	struct list_head *pos, *npos;
	struct tplg_elem *elem;
	int ret, wsize = 0, count = 0;

	/* count number of elements */
	list_for_each_safe(pos, npos, base)
		count++;

	/* write the header for this block */
	ret = write_block_header(tplg, tplg_type, 0,
		SND_SOC_TPLG_ABI_VERSION, 0, size, count);
	if (ret < 0) {
		fprintf(stderr, "error: failed to write %s block %d\n",
			obj_name, ret);
		return ret;
	}

	/* write each elem to block */
	list_for_each_safe(pos, npos, base) {

		elem = list_entry(pos, struct tplg_elem, list);

		/* compound elems have already been copied to other elems */
		if (elem->compound_elem)
			continue;

		if (elem->type != PARSER_TYPE_DAPM_GRAPH)
			verbose(tplg, " %s '%s': write %d bytes\n",
				obj_name, elem->id, elem->size);
		else
			verbose(tplg, " %s '%s': write %d bytes\n",
				obj_name, elem->route->source, elem->size);

		count = write(tplg->out_fd, elem->obj, elem->size);
		if (count < 0) {
			fprintf(stderr, "error: failed to write %s %d\n",
				obj_name, ret);
			return ret;
		}

		wsize += count;
	}

	/* make sure we have written the correct size */
	if (wsize != size) {
		fprintf(stderr, "error: size mismatch. Expected %d wrote %d\n",
			size, wsize);
		return -EIO;
	}

	return 0;
}

static int calc_block_size(struct list_head *base)
{
	struct list_head *pos, *npos;
	struct tplg_elem *elem;
	int size = 0;

	list_for_each_safe(pos, npos, base) {

		elem = list_entry(pos, struct tplg_elem, list);

		/* compound elems have already been copied to other elems */
		if (elem->compound_elem)
			continue;

		size += elem->size;
	}

	return size;
}

static int write_block(snd_tplg_t *tplg, struct list_head *base,
	int type)
{
	int size;

	/* calculate the block size in bytes for all elems in this list */
	size = calc_block_size(base);
	if (size <= 0)
		return size;

	verbose(tplg, " block size for type %d is %d\n", type, size);

	/* write each elem for this block */
	switch (type) {
	case PARSER_TYPE_MIXER:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_MIXER, "mixer");
	case PARSER_TYPE_BYTES:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_BYTES, "bytes");
	case PARSER_TYPE_ENUM:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_ENUM, "enum");
	case PARSER_TYPE_DAPM_GRAPH:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_DAPM_GRAPH, "route");
	case PARSER_TYPE_DAPM_WIDGET:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_DAPM_WIDGET, "widget");
	case PARSER_TYPE_PCM:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_PCM, "pcm");
	case PARSER_TYPE_BE:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_DAI_LINK, "be");
	case PARSER_TYPE_CC:
		return write_elem_block(tplg, base, size,
			SND_SOC_TPLG_TYPE_DAI_LINK, "cc");
	default:
		return -EINVAL;
	}

	return 0;
}

int tplg_write_data(snd_tplg_t *tplg)
{
	int ret;

	/* write mixer elems. */
	ret = write_block(tplg, &tplg->mixer_list,
		PARSER_TYPE_MIXER);
	if (ret < 0) {
		fprintf(stderr, "failed to write control elems %d\n", ret);
		return ret;
	}

	/* write enum control elems. */
	ret = write_block(tplg, &tplg->enum_list,
		PARSER_TYPE_ENUM);
	if (ret < 0) {
		fprintf(stderr, "failed to write control elems %d\n", ret);
		return ret;
	}

	/* write bytes extended control elems. */
	ret = write_block(tplg, &tplg->bytes_ext_list,
		PARSER_TYPE_BYTES);
	if (ret < 0) {
		fprintf(stderr, "failed to write control elems %d\n", ret);
		return ret;
	}

	/* write widget elems */
	ret = write_block(tplg, &tplg->widget_list,
		PARSER_TYPE_DAPM_WIDGET);
	if (ret < 0) {
		fprintf(stderr, "failed to write widget elems %d\n", ret);
		return ret;
	}

	/* write pcm elems */
	ret = write_block(tplg, &tplg->pcm_list,
		PARSER_TYPE_PCM);
	if (ret < 0) {
		fprintf(stderr, "failed to write pcm elems %d\n", ret);
		return ret;
	}

	/* write be elems */
	ret = write_block(tplg, &tplg->be_list,
		PARSER_TYPE_BE);
	if (ret < 0) {
		fprintf(stderr, "failed to write be elems %d\n", ret);
		return ret;
	}

	/* write cc elems */
	ret = write_block(tplg, &tplg->cc_list,
		PARSER_TYPE_CC);
	if (ret < 0) {
		fprintf(stderr, "failed to write cc elems %d\n", ret);
		return ret;
	}

	/* write route elems */
	ret = write_block(tplg, &tplg->route_list,
		PARSER_TYPE_DAPM_GRAPH);
	if (ret < 0) {
		fprintf(stderr, "failed to write graph elems %d\n", ret);
		return ret;
	}

	return 0;
}
