/*
 * x11.c -- X11 interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Wed Jun 13 02:52:54 2001.
 * $Id: x11.c,v 1.8 2001/06/12 17:59:24 sian Exp $
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

#include "common.h"

#ifdef USE_SHM
#include <X11/extensions/XShm.h>
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

  return x11;
}

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

    debug_message("x11: " __FUNCTION__ ": Depth 24 TrueColor visual available\n");

    XFree(xvi);
  } else {
    x11->visual = DefaultVisual(x11_display(x11), x11_screen(x11));
    x11->depth = DefaultDepth(x11_display(x11), x11_screen(x11));

    debug_message("x11: " __FUNCTION__ ": using default visual\n");
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
  debug_message("x11: " __FUNCTION__ ": bits_per_pixel = %d\n", x11_bpp(x11));
  if (x11_bpp(x11) == 32)
    debug_message("x11: " __FUNCTION__ ": Good, you have 32bpp ZPixmap.\n");
#endif

#ifdef USE_SHM
  if (XShmQueryExtension(x11_display(x11))) {
    x11->extensions |= X11_EXT_SHM;
    debug_message("x11: " __FUNCTION__ ": MIT-SHM Extension OK\n");
  }
#endif

#ifdef USE_XV
  {
    int result;
    X11Xv *xv;

    xv = &x11->xv;
    if ((result = XvQueryExtension(x11_display(x11), &xv->ver, &xv->rev,
				   &xv->req_base, &xv->ev_base,
				   &xv->err_base)) == Success) {
      x11->extensions |= X11_EXT_XV;
      debug_message("x11: " __FUNCTION__ ": Xv Extension version %d.%d OK\n", xv->ver, xv->rev);
    } else if (result == XvBadExtension) {
      debug_message("x11: " __FUNCTION__ ": Xv Extension unavailable.\n");
    } else if (result == XvBadAlloc) {
      debug_message("x11: " __FUNCTION__ ": XvQueryExtension() failed to allocate memory.\n");
    } else {
      debug_message("x11: " __FUNCTION__ ": unknown result code = %d.\n", result);
    }

    if (x11->extensions & X11_EXT_XV) {
      if ((result = XvQueryAdaptors(x11_display(x11), x11_root(x11),
				    &xv->nadaptors, &xv->adaptor_infos)) == Success) {
	if (xv->nadaptors) {
	  int i, j, k, l;

	  for (i = 0; i < xv->nadaptors; i++) {
	    debug_message("x11: " __FUNCTION__ ": Xv: adaptor#%d[%s]: %ld ports\n", i, xv->adaptor_infos[i].name, xv->adaptor_infos[i].num_ports);
	    debug_message("x11: " __FUNCTION__ ": Xv:  operations: ");
	    switch (xv->adaptor_infos[i].type & (XvInputMask | XvOutputMask)) {
	    case XvInputMask:
             if(xv->adaptor_infos[i].type & XvVideoMask)
                debug_message("PutVideo ");
             if(xv->adaptor_infos[i].type & XvStillMask)
                debug_message("PutStill ");
             if(xv->adaptor_infos[i].type & XvImageMask)
                debug_message("PutImage ");
             break;
	    case XvOutputMask:
             if(xv->adaptor_infos[i].type & XvVideoMask) 
                debug_message("GetVideo ");
             if(xv->adaptor_infos[i].type & XvStillMask) 
                debug_message("GetStill ");
             break;
	    default:
	      debug_message("None");
	      break;
	    }
	    debug_message("\n");
#if 0
	    for (j = 0; j < xv->adaptor_infos[i].num_formats; j++)
	      debug_message("x11: " __FUNCTION__ ": Xv:  format#%02d: depth %d visual id %d\n", j, xv->adaptor_infos[i].formats[j].depth, xv->adaptor_infos[i].formats[j].visual_id);
#endif
	    /* XXX: Information of the last port is only stored. */
	    for (j = 0; j < xv->adaptor_infos[i].num_ports; j++) {
	      debug_message("x11: " __FUNCTION__ ": Xv:  port#%d\n", j);
	      if ((result = XvQueryEncodings(x11_display(x11),
					     xv->adaptor_infos[i].base_id + j,
					     &xv->nencodings,
					     &xv->encoding_infos)) == Success) {
		for (k = 0; k < xv->nencodings; k++) {
		  debug_message("x11: " __FUNCTION__ ": Xv:   encoding#%d[%s]\n", k, xv->encoding_infos[k].name);
		  if (!strcmp(xv->encoding_infos[k].name, "XV_IMAGE")) {
		    xv->formats = XvListImageFormats(x11_display(x11),
						     xv->adaptor_infos[i].base_id + j,
						     &xv->nformats);
		    for (l = 0; l < xv->nformats; l++) {
		      int m;
		      char name[5] = { 0, 0, 0, 0, 0 };

		      debug_message("x11: " __FUNCTION__ ": Xv:    format#%d[", l);
		      memcpy(name, &xv->formats[l].id, 4);
		      for (m = 0; m < 4; m++)
			debug_message("%c", isprint(name[m]) ? name[m] : '.');
		      debug_message("]: %d bpp %d planes type %s (%s)\n",
				    xv->formats[l].bits_per_pixel,
				    xv->formats[l].num_planes,
				    xv->formats[l].type == XvRGB ? "RGB" : "YUV",
				    xv->formats[l].format == XvPacked ? "packed" : "planar");
		    }
		  }
		}
	      }
	    }
	  }
	} else {
	  debug_message("x11: " __FUNCTION__ ": Xv: there are no adaptors found.\n");
	}
      } else {
	debug_message("x11: " __FUNCTION__ ": Xv: XvQueryAdaptors() failed.\n");
	x11->extensions &= ~X11_EXT_XV;
      }
    }
  }
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
  free(x11);
}
