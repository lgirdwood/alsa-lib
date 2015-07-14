/*
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright (C) 2015 Intel Corporation
 *
 */

#ifndef __ALSA_TOPOLOGY_H
#define __ALSA_TOPOLOGY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \defgroup topology Topology Interface
 *  The topology interface.
 *  See \ref Topology page for more details.
 *  \{
 */

/*! \page topology ALSA Topology Interface
 *
 * ALSA Topology Interface
 *
 * The topology interface allows developers to define DSP topologies in a text
 * file format and to convert the text topology to a binary topology
 * representation that can be understood by the kernel. The topology core
 * currently recognises the following object types :-
 *
 *  * Controls (mixer, enumerated and byte) including TLV data.
 *  * PCMs (FE and BE configurations and capabilities)
 *  * DAPM widgets
 *  * DAPM graph elements.
 *  * Private data for each object type.
 *  * Manifest (containing count of each object type)
 *
 * The text format uses the standard ALSA configuration file format to
 * describe each topology object type :-
 *
 * Controls can contain TLV data, private data and enumerated controls and can
 * are defined in the following sections.
 *
 * Control Operations can come from the standard names (listed below) or
 * bespoke driver controls with numbers >= 256
 *
 * ops."name" {
 *	info "volsw"
 *	get "256"
 *	put "256"
 * }
 *
 * The Standrd operations names for control get(), put() and info calls are
 * "volsw","volsw_sx","volsw_xr_sx","enum","bytes","enum_value","range","strobe".

 ** SectionControlMixer."control name" {
 *	comment "optional comments"
 *
 *	index "1"
 *
 *	channel."name" {
 *	}
 *
 *	ops."ctl" {
 *	}
 *
 *	max "32"
 *	invert "0"
 *
 *
 *	tlv "hsw_vol_tlv"
 * }
 */

typedef struct snd_tplg snd_tplg_t;

/**
 * \brief Create a new topology parser instance.
 * \return New topology parser instance
 */
snd_tplg_t *snd_tplg_new(void);

/**
 * \brief Free a topology parser instance.
 * \param tplg Topology parser instance
 */
void snd_tplg_free(snd_tplg_t *tplg);

/**
 * \brief Parse and build topology text file into binary file.
 * \param tplg Topology instance.
 * \param infile Topology text input file to be parsed
 * \param outfile Binary topology output file.
 * \return Zero on sucess, otherwise a negative error code
 */
int snd_tplg_build_file(snd_tplg_t *tplg, const char *infile,
	const char *outfile);

/**
 * \brief Enable verbose reporting of binary file output
 * \param tplg Topology Instance
 * \param verbose Enable verbose output if non zero
 */
void snd_tplg_verbose(snd_tplg_t *tplg, int verbose);

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_TOPOLOGY_H */
