/*
 * x11.c -- X11 interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Jun  1 01:45:11 2002.
 * $Id: x11.c,v 1.18 2002/06/13 14:32:30 sian Exp $
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifdef USE_SHM
#  include <X11/extensions/XShm.h>
#endif

#ifdef USE_XV
#  include <X11/extensions/Xvlib.h>
#endif

#ifdef DEBUG
#  include <stdio.h>
#endif

#include "x11.h"

static int open(X11 *, char *);
static int close(X11 *);
static void destroy(X11 *);

static X11 template = {
  open: open,
  close: close,
  destroy: destroy
};

X11 *
x11_create(void)
{
  X11 *x11;

  if ((x11 = calloc(1, sizeof(X11))) == NULL)
    return NULL;
  memcpy(x11, &template, sizeof(X11));
#ifdef USE_XV
  if ((x11->xv = calloc(1, sizeof(X11Xv))) == NULL) {
    free(x11);
    return NULL;
  }
#endif

  return x11;
}

#ifdef USE_XV
static void
get_xvinfo(X11 *x11)
{
  int result;
  X11Xv *xv;
  XvAdaptorInfo *adaptor_infos;
  unsigned int nencodings;
  XvEncodingInfo *encoding_infos;
  int nformats;
  XvImageFormatValues *formats;

  xv = x11->xv;
  if ((result = XvQueryExtension(x11_display(x11), &xv->ver, &xv->rev,
				 &xv->req_base, &xv->ev_base,
				 &xv->err_base)) == Success) {
    x11->extensions |= X11_EXT_XV;
    debug_message_fnc("Xv Extension version %d.%d OK\n", xv->ver, xv->rev);
  } else if (result == XvBadExtension) {
    debug_message_fnc("Xv Extension unavailable.\n");
  } else if (result == XvBadAlloc) {
    debug_message_fnc("XvQueryExtension() failed to allocate memory.\n");
  } else {
    debug_message_fnc("unknown result code = %d.\n", result);
  }

  if (x11->extensions & X11_EXT_XV) {
    if ((result = XvQueryAdaptors(x11_display(x11), x11_root(x11),
				  &xv->nadaptors, &adaptor_infos)) == Success) {
      if (xv->nadaptors) {
	int l, is_putimage;
	unsigned int i, j, k;

	for (i = 0; i < xv->nadaptors; i++) {
	  debug_message_fnc("Xv: adaptor#%d[%s]: %ld ports\n", i, adaptor_infos[i].name, adaptor_infos[i].num_ports);
	  debug_message_fnc("Xv:  operations: ");
	  is_putimage = 0;
	  switch (adaptor_infos[i].type & (XvInputMask | XvOutputMask)) {
	  case XvInputMask:
	    if (adaptor_infos[i].type & XvVideoMask)
	      debug_message("PutVideo ");
	    if (adaptor_infos[i].type & XvStillMask)
	      debug_message("PutStill ");
	    if (adaptor_infos[i].type & XvImageMask) {
	      is_putimage = 1;
	      debug_message("PutImage ");
	    }
	    break;
	  case XvOutputMask:
	    if (adaptor_infos[i].type & XvVideoMask) 
	      debug_message("GetVideo ");
	    if (adaptor_infos[i].type & XvStillMask) 
	      debug_message("GetStill ");
	    break;
	  default:
	    debug_message("None");
	    break;
	  }
	  debug_message("\n");
	  if (!is_putimage) {
	    debug_message("Xv: No PutImage operation...\n");
	    continue;
	  }
#if 0
	  for (j = 0; j < adaptor_infos[i].num_formats; j++)
	    debug_message_fnc("Xv:  format#%02d: depth %d visual id %d\n", j, adaptor_infos[i].formats[j].depth, adaptor_infos[i].formats[j].visual_id);
#endif
	  /* XXX: Information of the last port is only stored. */
	  for (j = 0; j < adaptor_infos[i].num_ports; j++) {
	    debug_message_fnc("Xv:  port#%d(%ld)\n", j, adaptor_infos[i].base_id + j);
	    if ((result = XvQueryEncodings(x11_display(x11),
					   adaptor_infos[i].base_id + j,
					   &nencodings,
					   &encoding_infos)) == Success) {
	      for (k = 0; k < nencodings; k++) {
		debug_message_fnc("Xv:   encoding#%d[%s] (%ld x %ld)\n", k, encoding_infos[k].name, encoding_infos[k].width, encoding_infos[k].height);
		xv->image_width  = encoding_infos[k].width;
		xv->image_height = encoding_infos[k].height;

		/* XXX: XvVideo, XvStill are unsupported */
		xv->image_port = adaptor_infos[i].base_id + j;
		debug_message_fnc("Xv:   Image port %d detected\n", xv->image_port);
		formats = XvListImageFormats(x11_display(x11),
					     adaptor_infos[i].base_id + j,
					     &nformats);
		xv->capable_format = 0;
		for (l = 0; l < nformats; l++) {
		  int m, c;
		  char name[5] = { 0, 0, 0, 0, 0 };

		  debug_message_fnc("Xv:    format#%d[", l);
		  memcpy(name, &formats[l].id, 4);
		  for (m = 0; m < 4; m++)
		    debug_message("%c", isprint(name[m]) ? name[m] : '.');
		  debug_message("]: %d bpp %d planes type %s %s %s ",
				formats[l].bits_per_pixel,
				formats[l].num_planes,
				formats[l].type == XvRGB ? "RGB" : "YUV",
				formats[l].format == XvPacked ? "packed" : "planar",
				formats[l].byte_order == LSBFirst ? "LSBFirst" : "MSBFirst");
		  if (formats[l].format == XvPlanar) {
		    debug_message("component order: ");
		    for (m = 0; m < formats[l].num_planes; m++)
		      debug_message("%c", formats[l].component_order[m]);
		    debug_message(" ");
		  }
		  c = -1;
		  /*
		    name fcc        bpp        description
		    YUY2 0x32595559 16  packed YUV 4:2:2 byte order: YUYV
		    YV12 0x32315659 12  planar 8 bit Y followed by 8 bit 2x2 subsampled V and U
		    I420 0x30323449 12  planar 8 bit Y followed by 8 bit 2x2 subsampled U and V
		    UYVY 0x59565955 16  packed YUV 4:2:2 byte order: UYVY
		  */
		  if (strcmp(name, "YUY2") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_YUY2;
		  } else if (strcmp(name, "YV12") == 0) {
		    if (formats[l].format != XvPlanar)
		      continue;
		    c = XV_YV12;
		  } else if (strcmp(name, "I420") == 0) {
		    if (formats[l].format != XvPlanar)
		      continue;
		    c = XV_I420;
		  } else if (strcmp(name, "UYVY") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_UYVY;
		  } else if (strcmp(name, "RV15") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_RV15;
		  } else if (strcmp(name, "RV16") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_RV16;
		  } else if (strcmp(name, "RV24") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_RV24;
		  } else if (strcmp(name, "RV32") == 0) {
		    if (formats[l].format != XvPacked)
		      continue;
		    c = XV_RV32;
		  }
		  if (c != -1) {
		    xv->capable_format |= (1 << c);
		    xv->format_ids[c] =  formats[l].id;
		    xv->bits_per_pixel[c] = formats[l].bits_per_pixel;
		    xv->prefer_msb[c] = (formats[l].byte_order == MSBFirst);
		  } else {
		    debug_message("unsupported");
		  }
		  debug_message("\n");
		  if (formats[l].type == XvRGB) {
		    debug_message_fnc("Xv: %d RGB mask %04X,%04X,%04X\n", formats[l].depth, formats[l].red_mask, formats[l].green_mask, formats[l].blue_mask);
		  } else {
		    debug_message_fnc("Xv:     bits %d %d %d horz %d %d %d vert %d %d %d order %s\n",
				      formats[l].y_sample_bits,
				      formats[l].u_sample_bits,
				      formats[l].v_sample_bits,
				      formats[l].horz_y_period,
				      formats[l].horz_u_period,
				      formats[l].horz_v_period,
				      formats[l].vert_y_period,
				      formats[l].vert_u_period,
				      formats[l].vert_v_period,
				      formats[l].scanline_order == XvTopToBottom ? "TopToBottom" : "BottomToTop");
		  }
		}
		XFree(formats);
	      }
	      XvFreeEncodingInfo(encoding_infos);
	    }
	  }
	}
	XvFreeAdaptorInfo(adaptor_infos);
      } else {
	debug_message_fnc("Xv: there are no adaptors found.\n");
      }
    } else {
      debug_message_fnc("Xv: XvQueryAdaptors() failed.\n");
      x11->extensions &= ~X11_EXT_XV;
    }
  }
}
#endif

/* methods */

static int
open(X11 *x11, char *dispname)
{
  XVisualInfo *xvi, xvit;
  XPixmapFormatValues *xpfv;
  int i, count;

  if ((x11->disp = XOpenDisplay(dispname)) == NULL)
    return 0;

  x11->root = DefaultRootWindow(x11_display(x11));
  x11->screen = DefaultScreen(x11_display(x11));
  x11->sc = ScreenOfDisplay(x11_display(x11), x11_screen(x11));
  x11->white = WhitePixel(x11_display(x11), x11_screen(x11));
  x11->black = BlackPixel(x11_display(x11), x11_screen(x11));

  xvit.screen = x11_screen(x11);
  xvit.depth = 24;
  xvit.class = TrueColor;
  xvi = XGetVisualInfo(x11_display(x11), VisualScreenMask | VisualDepthMask | VisualClassMask,
		       &xvit, &count);
  if (count) {
    x11->visual = xvi[0].visual;
    x11->depth = 24;

    debug_message_fnc("Depth 24 TrueColor visual available\n");

    XFree(xvi);
  } else {
    x11->visual = DefaultVisual(x11_display(x11), x11_screen(x11));
    x11->depth = DefaultDepth(x11_display(x11), x11_screen(x11));

    debug_message_fnc("using default visual\n");
  }

  xpfv = XListPixmapFormats(x11_display(x11), &count);
  if (x11->depth == 24) {
    x11->bits_per_pixel = 0;
    for (i = 0; i < count; i++) {
      if (xpfv[i].depth == 24 && x11->bits_per_pixel < xpfv[i].bits_per_pixel)
	x11->bits_per_pixel = xpfv[i].bits_per_pixel;
    }
  } else {
    for (i = 0; i < count; i++) {
      if (xpfv[i].depth == x11->depth) {
	x11->bits_per_pixel = xpfv[i].bits_per_pixel;
	break;
      }
    }
  }
  XFree(xpfv);

#ifdef DEBUG
  debug_message_fnc("bits_per_pixel = %d\n", x11_bpp(x11));
  if (x11_bpp(x11) == 32)
    debug_message_fnc("Good, you have 32bpp ZPixmap.\n");
#endif

#ifdef USE_SHM
  if (XShmQueryExtension(x11_display(x11))) {
    x11->extensions |= X11_EXT_SHM;
    debug_message_fnc("MIT-SHM Extension OK\n");
  }
#endif

#ifdef USE_XV
  get_xvinfo(x11);
#endif

  {
    XImage *xi;

    xi = XCreateImage(x11_display(x11), x11_visual(x11), x11_depth(x11),
		      ZPixmap, 0, NULL, 16, 16, 8, 0);
    x11->prefer_msb = (xi->byte_order == LSBFirst) ? 0 : 1;
    XDestroyImage(xi);
  }

  return 1;
}

static int
close(X11 *x11)
{
  XCloseDisplay(x11_display(x11));
  x11_display(x11) = NULL;
  free(x11);

  return 1;
}

static void
destroy(X11 *x11)
{
  if (x11_display(x11))
    close(x11);
#ifdef USE_XV
  if (x11->xv)
    free(x11->xv);
#endif
  free(x11);
}
