/*
 * decode.c
 * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "compat.h"
#include "common.h"

#define VIDEO_OUT_NEED_VO_SETUP
#include "video_out.h"
#include "mpeg2.h"
#include "mpeg2_internal.h"
#include "mm_accel.h"
#include "attributes.h"
#include "mmx.h"

mpeg2_config_t config;

void mpeg2_init (mpeg2dec_t * mpeg2dec, uint32_t _mm_accel,
		 vo_instance_t * output)
{
    static int do_init = 1;

    if (do_init) {
	do_init = 0;
	config.flags = _mm_accel;
	idct_init ();
	motion_comp_init ();
    }

    mpeg2dec->chunk_buffer = memalign (16, 224 * 1024 + 4);
    mpeg2dec->picture = memalign (16, sizeof (picture_t));

    mpeg2dec->shift = 0;
    mpeg2dec->is_sequence_needed = 1;
    mpeg2dec->drop_flag = 0;
    mpeg2dec->drop_frame = 0;
    mpeg2dec->in_slice = 0;
    mpeg2dec->output = output;
    mpeg2dec->chunk_ptr = mpeg2dec->chunk_buffer;
    mpeg2dec->code = 0xff;

    memset (mpeg2dec->picture, 0, sizeof (picture_t));

    /* initialize supstructures */
    header_state_init (mpeg2dec->picture);
}

static int parse_chunk (mpeg2dec_t * mpeg2dec, int code, uint8_t * buffer)
{
    picture_t * picture;
    int is_frame_done;

    /* wait for sequence_header_code */
    if (mpeg2dec->is_sequence_needed && (code != 0xb3))
	return 0;

    stats_header (code, buffer);

    picture = mpeg2dec->picture;
    is_frame_done = mpeg2dec->in_slice && ((!code) || (code >= 0xb0));

    if (is_frame_done) {
	mpeg2dec->in_slice = 0;

	if (((picture->picture_structure == FRAME_PICTURE) ||
	     (picture->second_field)) &&
	    (!(mpeg2dec->drop_frame))) {
	    vo_draw ((picture->picture_coding_type == B_TYPE) ?
		     picture->current_frame :
		     picture->forward_reference_frame);
#ifdef __i386__
	    if (config.flags & MM_ACCEL_X86_MMX)
		emms ();
#endif
	}
    }

    switch (code) {
    case 0x00:	/* picture_start_code */
	if (header_process_picture_header (picture, buffer)) {
	    fprintf (stderr, "bad picture header\n");
	    exit (1);
	}
	mpeg2dec->drop_frame =
	    mpeg2dec->drop_flag && (picture->picture_coding_type == B_TYPE);
	break;

    case 0xb3:	/* sequence_header_code */
	if (header_process_sequence_header (picture, buffer)) {
	    fprintf (stderr, "bad sequence header\n");
	    exit (1);
	}
	if (mpeg2dec->is_sequence_needed) {
	    mpeg2dec->is_sequence_needed = 0;
	    if (vo_setup (mpeg2dec->output, picture->coded_picture_width,
			  picture->coded_picture_height)) {
		fprintf (stderr, "display setup failed\n");
		exit (1);
	    }
	    picture->forward_reference_frame =
		vo_get_frame (mpeg2dec->output,
			      VO_PREDICTION_FLAG | VO_BOTH_FIELDS);
	    picture->backward_reference_frame =
		vo_get_frame (mpeg2dec->output,
			      VO_PREDICTION_FLAG | VO_BOTH_FIELDS);
	}

	mpeg2dec->frame_rate_code = picture->frame_rate_code;
	switch (mpeg2dec->frame_rate_code) {
	case 1:
	  mpeg2dec->frame_rate = 23.976;
	  break;
	case 2:
	  mpeg2dec->frame_rate = 24.0;
	  break;
	case 3:
	  mpeg2dec->frame_rate = 25.0;
	  break;
	case 4:
	  mpeg2dec->frame_rate = 29.97;
	  break;
	case 5:
	  mpeg2dec->frame_rate = 30.0;
	  break;
	case 6:
	  mpeg2dec->frame_rate = 50.0;
	  break;
	case 7:
	  mpeg2dec->frame_rate = 59.94;
	  break;
	case 8:
	  mpeg2dec->frame_rate = 60.0;
	  break;
	default:
	  fprintf(stderr, "invalid or unknown frame rate code: %d\n", mpeg2dec->frame_rate_code);
	  break;
	}
	break;

    case 0xb5:	/* extension_start_code */
	if (header_process_extension (picture, buffer)) {
	    fprintf (stderr, "bad extension\n");
	    exit (1);
	}
	break;

    default:
	if (code >= 0xb9)
	    fprintf (stderr, "stream not demultiplexed ?\n");

	if (code >= 0xb0)
	    break;

	if (!(mpeg2dec->in_slice)) {
	    mpeg2dec->in_slice = 1;

	    if (picture->second_field) {
	      vo_field (picture->current_frame, picture->picture_structure);
	    } else {
		if (picture->picture_coding_type == B_TYPE)
		    picture->current_frame =
			vo_get_frame (mpeg2dec->output,
				      picture->picture_structure);
		else {
		    picture->current_frame =
			vo_get_frame (mpeg2dec->output,
				      (VO_PREDICTION_FLAG |
				       picture->picture_structure));
		    picture->forward_reference_frame =
			picture->backward_reference_frame;
		    picture->backward_reference_frame = picture->current_frame;
		}
	    }
	}

	if (!(mpeg2dec->drop_frame)) {
	    slice_process (picture, code, buffer);
#ifdef __i386__
	    if (config.flags & MM_ACCEL_X86_MMX)
		emms ();
#endif
	}
    }

    return is_frame_done;
}

int mpeg2_decode_data (mpeg2dec_t * mpeg2dec, uint8_t * current, uint8_t * end)
{
    uint32_t shift;
    uint8_t * chunk_ptr;
    uint8_t byte;
    int ret = 0;

    shift = mpeg2dec->shift;
    chunk_ptr = mpeg2dec->chunk_ptr;

    while (current != end) {
	while (1) {
	    byte = *current++;
	    if (shift != 0x00000100) {
		*chunk_ptr++ = byte;
		shift = (shift | byte) << 8;
		if (current != end)
		    continue;
		mpeg2dec->chunk_ptr = chunk_ptr;
		mpeg2dec->shift = shift;
		return ret;
	    }
	    break;
	}

	/* found start_code following chunk */

	ret += parse_chunk (mpeg2dec, mpeg2dec->code, mpeg2dec->chunk_buffer);

	/* done with header or slice, prepare for next one */

	mpeg2dec->code = byte;
	chunk_ptr = mpeg2dec->chunk_buffer;
	shift = 0xffffff00;
    }
    mpeg2dec->chunk_ptr = chunk_ptr;
    mpeg2dec->shift = shift;
    return ret;
}

void mpeg2_close (mpeg2dec_t * mpeg2dec)
{
    static uint8_t finalizer[] = {0,0,1,0};

    mpeg2_decode_data (mpeg2dec, finalizer, finalizer+4);

    if (! (mpeg2dec->is_sequence_needed))
	vo_draw (mpeg2dec->picture->backward_reference_frame);

    free (mpeg2dec->chunk_buffer);
    free (mpeg2dec->picture);
}

void mpeg2_drop (mpeg2dec_t * mpeg2dec, int flag)
{
    mpeg2dec->drop_flag = flag;
}
