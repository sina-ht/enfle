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
 * Andrea Graziani (Ag)
 *
 * DivX Advanced Research Center <darc@projectmayo.com>
**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define GLOBAL
#include "mp4_decoder.h"
#include "global.h"

/**
 *
**/

void get_mp4picture (unsigned char *bmp, int render_flag); 
void initdecoder (void);
void closedecoder (void);

/***/

extern int dec_init(char *, int, int);
extern int dec_reinit(void);
extern int dec_release(void);
extern int dec_frame(void);

/***/

int dec_init(char *infilename, int hor_size, int ver_size)
{
	// open input file
	if ((base.infile = open (infilename, O_RDONLY | O_BINARY)) < 0) {
		printf ("Input file %s not found\n", infilename);
		exit(91);
	}
	// init global stuff
  ld = &base;
	coeff_pred = &ac_dc;

  initbits ();

	// read first vol and vop
	if (juice_flag)
	{
			mp4_hdr.width = juice_hor;
			mp4_hdr.height = juice_ver;

			mp4_hdr.quant_precision = 5;
			mp4_hdr.bits_per_pixel = 8;

			mp4_hdr.quant_type = 0;

			mp4_hdr.time_increment_resolution = 15;
			mp4_hdr.complexity_estimation_disable = 1;
	}
	else
	{
		getvolhdr(); // get vol header
	}

	mp4_hdr.picnum = 0;
	mp4_hdr.mb_xsize = mp4_hdr.width / 16;
	mp4_hdr.mb_ysize = mp4_hdr.height / 16;
	mp4_hdr.mba_size = mp4_hdr.mb_xsize * mp4_hdr.mb_ysize;

	getvophdr(); // get vop header

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

	return 1;
}

/***/

int dec_frame(void)
{
	// decoded vop
	get_mp4picture(NULL, 0);
	mp4_hdr.picnum++;

	// read next vop header
	return getvophdr(); 
}

/***/

int dec_release(void)
{
  close (base.infile);
	closedecoder();

	return 1;
}

/***/

int dec_reinit(void)
{
  if (base.infile != 0)
    lseek (base.infile, 0l, 0);
  initbits ();

	return 1;
}

/***/

void initdecoder (void)
{
  int i, j, cc, size;

  /* clip table */
	clp = (unsigned char *) malloc (1024);
  if (! clp) {
    printf ("malloc failed\n");
		exit(0);
	}
  clp += 384;
  for (i = -384; i < 640; i++)
    clp[i] = (i < 0) ? 0 : ((i > 255) ? 255 : i);

	/* dc prediction border */
	for (i = 0; i < (2*MBC+1); i++)
		coeff_pred->dc_store_lum[0][i] = 1024;

	for (i = 1; i < (2*MBR+1); i++)
		coeff_pred->dc_store_lum[i][0] = 1024;

	for (i = 0; i < (MBC+1); i++) {
		coeff_pred->dc_store_chr[0][0][i] = 1024;
		coeff_pred->dc_store_chr[1][0][i] = 1024;
	}

	for (i = 1; i < (MBR+1); i++) {
		coeff_pred->dc_store_chr[0][i][0] = 1024;
		coeff_pred->dc_store_chr[1][i][0] = 1024;
	}

	/* ac prediction border */
	for (i = 0; i < (2*MBC+1); i++)
		for (j = 0; j < 7; j++)	{
			coeff_pred->ac_left_lum[0][i][j] = 0;
			coeff_pred->ac_top_lum[0][i][j] = 0;
		}

	for (i = 1; i < (2*MBR+1); i++)
		for (j = 0; j < 7; j++)	{
			coeff_pred->ac_left_lum[i][0][j] = 0;
			coeff_pred->ac_top_lum[i][0][j] = 0;
		}

	/* 
			[Review] too many computation to access to the 
			correct array value, better use two different
			pointer for Cb and Cr components
	*/
	for (i = 0; i < (MBC+1); i++)
		for (j = 0; j < 7; j++)	{
			coeff_pred->ac_left_chr[0][0][i][j] = 0; 
			coeff_pred->ac_top_chr[0][0][i][j] = 0;
			coeff_pred->ac_left_chr[1][0][i][j] = 0;
			coeff_pred->ac_top_chr[1][0][i][j] = 0;
		}

	for (i = 1; i < (MBR+1); i++)
		for (j = 0; j < 7; j++)	{
			coeff_pred->ac_left_chr[0][i][0][j] = 0;
			coeff_pred->ac_top_chr[0][i][0][j] = 0;
			coeff_pred->ac_left_chr[1][i][0][j] = 0;
			coeff_pred->ac_top_chr[1][i][0][j] = 0;
		}

	/* mode border */
	for (i = 0; i < mb_width + 1; i++)
		modemap[0][i] = INTRA;
	for (i = 0; i < mb_height + 1; i++) {
		modemap[i][0] = INTRA;
		modemap[i][mb_width+1] = INTRA;
	}

	// edged forward and reference frame
  for (cc = 0; cc < 3; cc++)
  {
    if (cc == 0)
    {
      size = coded_picture_width * coded_picture_height;

			edged_ref[cc] = (unsigned char *) malloc (size);
      if (! edged_ref[cc]) {
        printf ("malloc failed\n");
				exit(0);
			}
			edged_for[cc] = (unsigned char *) malloc (size);
      if (! edged_for[cc]) {
        printf ("malloc failed\n");
				exit(0);
			}
      frame_ref[cc] = edged_ref[cc] + coded_picture_width * 32 + 32;
      frame_for[cc] = edged_for[cc] + coded_picture_width * 32 + 32;
    } 
    else
    {
      size = chrom_width * chrom_height;

			edged_ref[cc] = (unsigned char *) malloc (size);
      if (! edged_ref[cc]) {
        printf ("malloc failed\n");
				exit(0);
			}
			edged_for[cc] = (unsigned char *) malloc (size);
      if (! edged_for[cc]) {
        printf ("malloc failed\n");
				exit(0);
			}
      frame_ref[cc] = edged_ref[cc] + chrom_width * 16 + 16;
      frame_for[cc] = edged_for[cc] + chrom_width * 16 + 16;
    }
  }

	// display frame
	for (cc = 0; cc < 3; cc++) 
	{
		if (cc == 0)
			size = horizontal_size * vertical_size;
		else
			size = (horizontal_size * vertical_size) >> 2;

		display_frame[cc] = (unsigned char *) malloc (size);
		if (! display_frame[cc]) {
			printf("malloc failed\n");
			exit(0);
		}
	}

#ifndef _MMX_IDCT
  init_idct ();
#endif // _DEBUG
}

/***/

void closedecoder ()
{
	int cc;
	for (cc = 0; cc < 3; cc++) {
		free(display_frame[cc]);
		free(edged_ref[cc]);
		free(edged_for[cc]);
	}
}
