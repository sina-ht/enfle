/*
 * idct_mmx.h
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

void idct_block_copy_sse (int16_t *block, uint8_t * dest, int stride);
void idct_block_add_sse (int16_t *block, uint8_t * dest, int stride);
void idct_block_copy_mmx (int16_t *block, uint8_t * dest, int stride);
void idct_block_add_mmx (int16_t *block, uint8_t * dest, int stride);
void idct_mmx_init (void);
