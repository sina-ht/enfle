/**************************************************************************
 *                                                                        *
 * This code has been developed by Andrea Graziani. This software is an   *
 * implementation of a part of one or more MPEG-4 Video tools as          *
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this    *
 * software module in hardware or software products are advised that its  *
 * use may infringe existing patents or copyrights, and any such use      *
 * would be at such party's own risk.  The original developer of this     *
 * software module and his/her company, and subsequent editors and their  *
 * companies (including Project Mayo), will have no liability for use of  *
 * this software or modifications or derivatives thereof.                 *
 *                                                                        *
 * Project Mayo gives users of the Codec a license to this software       *
 * module or modifications thereof for use in hardware or software        *
 * products claiming conformance to the MPEG-4 Video Standard as          *
 * described in the Open DivX license.                                    *
 *                                                                        *
 * The complete Open DivX license can be found at                         *
 * http://www.projectmayo.com/opendivx/license.php                        *
 *                                                                        *
 **************************************************************************/
/**
*  Copyright (C) 2001 - Project Mayo
 *
 * Andrea Graziani
 * Adam Li
 *
 * DivX Advanced Research Center <darc@projectmayo.com>
*
**/
// decore.c //

#include "mp4_decoder.h"
#include "global.h"

#include "postprocess.h"
#include "yuv2rgb.h"
#include "decore.h"

/**
 *
**/

extern unsigned char *decore_stream;
extern int decore_length;

static int decore_init(int hor_size, int ver_size, unsigned long color_depth);
static int decore_release(void);
static int decore_frame(unsigned char *stream, int length, unsigned char *bmp, int render_flag);

/***/

int decore(unsigned long handle, unsigned long dec_opt,
	void *param1, void *param2)
{
	switch (dec_opt)
	{
		case DEC_OPT_INIT:
		{
			DEC_PARAM *dec_param = (DEC_PARAM *) param1;
 			int x_size = dec_param->x_dim;
 			int y_size = dec_param->y_dim;
			unsigned long color_depth = dec_param->color_depth;

			juice_flag = 1;
			post_flag = 0;

			decore_init(x_size, y_size, color_depth); // init decoder resources

			return DEC_OK;
		}
		break; 
		case DEC_OPT_RELEASE:
		{
			decore_release();

			return DEC_OK;
		}
		break;
		case DEC_OPT_SETPP:
		{
			DEC_SET *dec_set = (DEC_SET *) param1;
			int postproc_level = dec_set->postproc_level;

			if ((postproc_level < 0) | (postproc_level > 100))
				return DEC_BAD_FORMAT;

			if (postproc_level < 1) {
				post_flag = 0;
				return DEC_OK;
			}
			else 
			{
				post_flag = 1;

				if (postproc_level < 10) {
					pp_options = PP_DEBLOCK_Y_H;
				}
				else if (postproc_level < 20) {
					pp_options = PP_DEBLOCK_Y_H | PP_DEBLOCK_Y_V;
				}
				else if (postproc_level < 30) {
					pp_options = PP_DEBLOCK_Y_H | PP_DEBLOCK_Y_V | PP_DERING_Y;
				}
				else if (postproc_level < 40) {
					pp_options = PP_DEBLOCK_Y_H | PP_DEBLOCK_Y_V | PP_DERING_Y | PP_DEBLOCK_C_H;
				}
				else if (postproc_level < 50) {
					pp_options = PP_DEBLOCK_Y_H | PP_DEBLOCK_Y_V | PP_DERING_Y | 
						PP_DEBLOCK_C_H | PP_DEBLOCK_C_V;
				}
				else if (postproc_level < 60) {
					pp_options = PP_DEBLOCK_Y_H | PP_DEBLOCK_Y_V | PP_DERING_Y | 
						PP_DEBLOCK_C_H | PP_DEBLOCK_C_V | PP_DERING_C;
				}
			}

			return DEC_OK;
		}
		break;
		default:
		{
			DEC_FRAME *dec_frame = (DEC_FRAME *) param1;
			unsigned char *bmp = dec_frame->bmp;
			unsigned char *stream = dec_frame->bitstream;
			int length = dec_frame->length;
			int render_flag = dec_frame->render_flag;

			decore_frame(stream, length, bmp, render_flag);

			return DEC_OK;
		}
		break;
	}
}

/***/

static int decore_init(int hor_size, int ver_size, unsigned long color_depth)
{
	// init global stuff
	ld = &base;
	coeff_pred = &ac_dc;
	
	initbits ();
	
	// read first vol and vop
	mp4_hdr.width = hor_size;
	mp4_hdr.height = ver_size;
	
	mp4_hdr.quant_precision = 5;
	mp4_hdr.bits_per_pixel = 8;
	
	mp4_hdr.quant_type = 0;
	
	mp4_hdr.time_increment_resolution = 15;
	mp4_hdr.complexity_estimation_disable = 1;
	
	mp4_hdr.picnum = 0;
	mp4_hdr.mb_xsize = mp4_hdr.width / 16;
	mp4_hdr.mb_ysize = mp4_hdr.height / 16;
	mp4_hdr.mba_size = mp4_hdr.mb_xsize * mp4_hdr.mb_ysize;

	// set picture dimension global vars
	{
		horizontal_size = mp4_hdr.width;
		vertical_size = mp4_hdr.height;

		mb_width = horizontal_size / 16;
		mb_height = vertical_size / 16;

		coded_picture_width = horizontal_size + 64;
		coded_picture_height = vertical_size + 64;
		chrom_width = coded_picture_width >> 1;
		chrom_height = coded_picture_height >> 1;
	}

	// init decoder
	initdecoder();

	switch (color_depth)
		{
		case 32:
			convert_yuv = yuv2rgb_32;
			break;
		case 16:
			convert_yuv = yuv2rgb_16;
			break;
		default:
			convert_yuv = yuv2rgb_24;
			break;
		}

	return 1;
}

/***/

static int decore_frame(unsigned char *stream, int length, unsigned char *bmp, int render_flag)
{
	decore_stream = stream;
	decore_length = length;

	initbits ();

	getvophdr(); // read vop header
	get_mp4picture(bmp, render_flag); // decode vop

	mp4_hdr.picnum++;

	return 1;
}

/***/

static int decore_release(void)
{
/***
  close (base.infile);
***/
	closedecoder();

	return 1;
}

/***/
