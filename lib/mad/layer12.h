/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2003 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

# ifndef LIBMAD_LAYER12_H
# define LIBMAD_LAYER12_H

# include "stream.h"
# include "frame.h"

int mad_layer_I(struct mad_stream *, struct mad_frame *);
int mad_layer_II(struct mad_stream *, struct mad_frame *);

# endif
