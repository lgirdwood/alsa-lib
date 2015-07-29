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
 * \defgroup topology Topology Interface
 * \{
 */

/*! \page topology ALSA Topology Interface
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
 * <h3>Topology File Format</h3>
 *
 * The topology text format uses the standard ALSA configuration file format to
 * describe each topology object type. This allows topology objects to include
 * other topology objects as part of thier definition. i.e. a TLV data object
 * can be shared amongst many control objects that use the same TLV data.
 *
 *
 * <h4>Controls</h4>
 * Topology audio controls can belong to three different types :-
 *   * Mixer control
 *   * Enumerated control
 *   * Byte control
 *
 * Each control type can contain TLV data, private data, operations and also
 * belong to widget objects.<br>
 *
 * <h5>Control Operations</h5>
 * Driver Kcontrol callback info(), get() and put() operations are mapped with
 * the CTL ops section in topology configuration files. The ctl ops section can
 * assign operations using the standard names (listed below) for the standard
 * kcontrol types or use ID numbers (>256) to map to bespoke driver controls.<br>
 *
 * <pre>
 *
 *	ops."ctl" {
 *		info "volsw"
 *		get "257"
 *		put "257"
 *	}
 *
 * </pre>
 *
 * This mapping shows info() using the standard "volsw" info callback whilst
 * the get() and put() are mapped to bespoke driver callbacks. <br>
 *
 * The Standard operations names for control get(), put() and info calls
 * are :-
 *  * volsw
 *  * volsw_sx
 *  * volsw_xr_sx
 *  * enum
 *  * bytes
 *  * enum_value
 *  * range
 *  * strobe
 *
 * <h5>Control TLV Data</h5>
 * Controls can also use TLV data to represent dB information. This can be done
 * by defining a TLV section and using the TLV section within the control.
 * The TLV data for DBScale types are defined as follows :-
 *
 * <pre>
 *	scale {
 *		min "-9000"
 *		step "300"
 *		mute "1"
 *	}
 * </pre>
 *
 * Where the meanings and values for min, step and mute are exactly the same
 * as defined in driver code.
 *
 * <h5>Control Channel Mapping</h5>
 * Controls can also specify which channels they are mapped with. This is useful
 * for userspace as it allows applications to determine the correct control
 * channel for Left and Right etc. Channel maps are defined as follows :-
 *
 * <pre>
 *	channel."name" {
 *		reg "0"
 *		shift "0"
 *	}
 * </pre>
 *
 * The channel map reg is the register offset for the control, shift is the
 * bit shift within the register for the channel and the section name is the
 * channel name and can be one of the following :-
 *
 * <pre>
 *  * mono		# mono stream
 *  * fl 		# front left
 *  * fr		# front right
 *  * rl		# rear left
 *  * rr		# rear right
 *  * fc		# front center
 *  * lfe		# LFE
 *  * sl		# side left
 *  * sr		# side right
 *  * rc		# rear center
 *  * flc		# front left center
 *  * frc		# front right center
 *  * rlc		# rear left center
 *  * rrc		# rear right center
 *  * flw		# front left wide
 *  * frw		# front right wide
 *  * flh		# front left high
 *  * fch		# front center high
 *  * frh		# front right high
 *  * tc		# top center
 *  * tfl		# top front left
 *  * tfr		# top front right
 *  * tfc		# top front center
 *  * trl		# top rear left
 *  * trr		# top rear right
 *  * trc		# top rear center
 *  * tflc		# top front left center
 *  * tfrc		# top front right center
 *  * tsl		# top side left
 *  * tsr		# top side right
 *  * llfe		# left LFE
 *  * rlfe		# right LFE
 *  * bc		# bottom center
 *  * blc		# bottom left center
 *  * brc		# bottom right center
 * </pre>
 *
 *  <h5>Control Private Data</h5>
 * Controls can also have private data. This can be done by defining a private
 * data section and including the section within the control. The private data
 * section is defined as follows :-
 *
 * <pre>
 * SectionData."pdata for EQU1" {
 *	file "/path/to/file"
 *	bytes "0x12,0x34,0x56,0x78"
 *	shorts "0x1122,0x3344,0x5566,0x7788"
 *	words "0xaabbccdd,0x11223344,0x66aa77bb,0xefef1234"
 * };
 * </pre>
 * The file, bytes, shorts and words keywords are all mutulally exclusive as
 * the private data should only be taken from one source.  The private data can
 * either be read from a separate file or defined in the topology file using
 * the bytes, shorts or words keywords.
 *
 * <h5>Mixer Controls</h5>
 * A mixer control is defined as a new section that can include channel mapping,
 * TLV data, callback operations and private data. The mixer section also
 * includes a few other config options that are shown here :-
 *
 * <pre>
 * SectionControlMixer."mixer name" {
 *	comment "optional comments"
 *
 *	index "1"			# Index number
 *
 *	channel."name" {		# Channel maps
 *	   ....
 *	}
 *
 *	ops."ctl" {			# Ops callback functions
 *	   ....
 *	}
 *
 *	max "32"			# Max control value
 *	invert "0"			# Whether control values are inverted
 *
 *	tlv "tld_data"			# optional TLV data
 *
 *	data "pdata for mixer1"		# optional private data
 * }
 * </pre>
 *
 * The section name is used to define the mixer name. The index number can be
 * used to identify topology objects groups. This allows driver operations on
 * objects with index number N and can be used to add/remove pipelines of
 * objects whilst other objects are unaffected.
 *
 * <h5>Byte Controls</h5>
 * A byte control is defined as a new section that can include channel mapping,
 * TLV data, callback operations and private data. The bytes section also
 * includes a few other config options that are shown here :-
 *
 * <pre>
 * SectionControlBytes."name" {
 *	comment "optional comments"
 *
 *	index "1"			# Index number
 *
 *	channel."name" {		# Channel maps
 *	   ....
 *	}
 *
 *	ops."ctl" {			# Ops callback functions
 *	   ....
 *	}
 *
 *	base "0"			# Register base
 *	num_regs "16"			# Number of registers
 *	mask "0xff"			# Mask
 *	max "255"			# Maximum value
 *
 *	tlv "tld_data"			# optional TLV data
 *
 *	data "pdata for mixer1"		# optional private data
 * }
 * </pre>
 *
 * <h5>Enumerated Controls</h5>
 * A enumerated control is defined as a new section (like mixer and byte) that
 * can include channel mapping, callback operations, private data and
 * text strings to represent the enumerated control options.<br>
 *
 * The text strings for the enumerated controls are defined in a seperate
 * section as follows :-
 *
 * <pre>
 * SectionText."name" {
 *
 *		Values [
 *			"value1"
 *			"value2"
			"value3"
 *		]
 * }
 * </pre>
 *
 * All the enumerated text values are listed in the values list.<br>
 * The enumerated control is similar to the other controls and defined as
 * follows :-
 *
 * <pre>
 * SectionControlMixer."name" {
 *	comment "optional comments"
 *
 *	index "1"			# Index number
 *
 *	texts "EQU1"			# Enumerated text items
 *
 *	channel."name" {		# Channel maps
 *	   ....
 *	}
 *
 *	ops."ctl" {			# Ops callback functions
 *	   ....
 *	}
 *
 *	data "pdata for mixer1"		# optional private data
 * }
 * </pre>
 *
 * <h4>DAPM Graph</h4>
 * DAPM graphs can easily be defined using the topology file. The format is
 * very similar to the DAPM graph kernel format. :-
 *
 * <pre>
 * SectionGraph."dsp" {
 *	index "1"			# Index number
 *
 *	lines [
 *		"sink1, control, source1"
 *		"sink2, , source2"
 *	]
 * }
 * </pre>
 *
 * The lines in the graph are defined as a variable size list of sinks,
 * controls and sources. The control name is optional as some graph lines have
 * no associated controls. The section name can be used to differentiate the
 * graph with other graphs, it's not used by the kernel atm.
 *
 * <h4>DAPM Widgets</h4>
 * DAPM wigets are similar to controls in that they can include many other
 * objects. Widgets can contain private data, mixer controls and enum controls.
 *
 * The following widget types are supported and match the driver types :-
 *
 *  * input
 *  * output
 *  * mux
 *  * mixer
 *  * pga
 *  * out_drv
 *  * adc
 *  * dac
 *  * switch
 *  * pre
 *  * post
 *  * aif_in
 *  * aif_out
 *  * dai_in
 *  * dai_out
 *  * dai_link
 *
 * Widgets are defined as follows :-
 *
 * <pre>
 * SectionWidget."name" {
 *
 *	index "1"			# Index number
 *
 *	type "aif_in"			# Widget type - detailed above
 *
 *	no_pm "true"			# No PM control bit.
 *	reg "20"			# PM bit register offset
 *	shift "0"			# PM bit register shift
 *	invert "1			# PM bit is inverted
 *	subseq "8"			# subsequence number
 *
 *	event_type "1"			# DAPM widget event type
 *	event_flags "1"			# DAPM widget event flags
 *
 *	mixer "name"			# Optional Mixer Control
 *	enum "name"			# Optional Enum Control
 *
 *	data "name"			# optional private data
 * }
 * </pre>
 *
 * The section name is the widget name. The mixer and enum fields are mutually
 * exclusive and used to include controls into the widget. The index and data
 * fields are the same for widgets as they are for controls whilst the other
 * fields map on very closely to the driver widget fields.
 *
 * <h4>PCM Capabilities</h4>
 * Topology can also define the capabilities of FE and BE PCMs. Capabilities
 * can be defined with the following section :-
 *
 * <pre>
 * SectionPCMCapabilities."name" {
 *
 *	formats "S24_LE,S16_LE"		# Supported formats
 *	rate_min "48000"		# Max supported sample rate
 *	rate_max "48000"		# Min suppoprted sample rate
 *	channels_min "2"		# Min number of channels
 *	channels_max "2"		# max number of channels
 * }
 * </pre>
 * The supported formats use the same naming convention as the driver macros.
 * The PCM capabilities name can be reffered to and included by BE, PCM and
 * Codec <-> codec topology sections.
 *
 * <h4>PCM Configurations</h4>
 * PCM runtime configurations can be defined for playback and capture stream
 * directions with the following section  :-
 *
 * <pre>
 * SectionPCMConfig."name" {
 *
 *	config."playback" {		# playback config
 *		format "S16_LE"		# playback format
 *		rate "48000"		# playback sample rate
 *		channels "2"		# playback channels
 *		tdm_slot "0xf"		# playback TDM slot
 *	}
 *
 *	config."capture" {		# capture config
 *		format "S16_LE"		# capture format
 *		rate "48000"		# capture sample rate
 *		channels "2"		# capture channels
 *		tdm_slot "0xf"		# capture TDM slot
 *	}
 * }
 * </pre>
 *
 * The supported formats use the same naming convention as the driver macros.
 * The PCM configuration name can be reffered to and included by BE, PCM and
 * Codec <-> codec topology sections.
 *
 * <h4>PCM Configurations</h4>
 * PCM, BE and Codec to Codec link sections define the supported capabilities
 * and configurations for supported playback and capture streams. The
 * definitions and content for PCMs, BE and Codec links are the same with the
 * exception of the section type :-
 *
 * <pre>
 * SectionPCM."name" {
 *	....
 * }
 * SectionBE."name" {
 *	....
 * }
 * SectionCC."name" {
 *	....
 * }
 * </pre>
 *
 * The section types above should be used for PCMs, Back Ends and Codec to Codec
 * links respectively.<br>
 *
 * The data for each section is defined as follows :-
 *
 * <pre>
 * SectionPCM."name" {
 *
 *	index "1"			# Index number
 *
 *	id "0"				# used for binding to the PCM
 *
 *	pcm."playback" {
 *		capabilities "capabilities1"	# capbilities for playback
 *
 *		configs [		# supported configs for playback
 *			"config1"
 *			"config2"
 *		]
 *	}
 *
 *	pcm."capture" {
 *		capabilities "capabilities2"	# capabilities for capture
 *
 *		configs [		# supported configs for capture
 *			"config1"
 *			"config2"
 *			"config3"
 *		]
 *	}
 * }
 * </pre>
 *
 */

/** Topology context */
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
 * \param verbose Enable verbose output level if non zero
 */
void snd_tplg_verbose(snd_tplg_t *tplg, int verbose);

/* \} */

#ifdef __cplusplus
}
#endif

#endif /* __ALSA_TOPOLOGY_H */
