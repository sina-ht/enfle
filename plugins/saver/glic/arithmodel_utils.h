/*
 * arithmodel_utils.h -- Utility functions
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Apr  3 18:46:30 2001.
 */

#ifndef _ARITHMODEL_UTILS_H
#define _ARITHMODEL_UTILS_H

int arithmodel_encode_bits(Arithmodel *, int, int, Index, Index);
int arithmodel_decode_bits(Arithmodel *, int *, int, Index, Index);
int arithmodel_encode_gamma(Arithmodel *, unsigned int, Index, Index);
int arithmodel_encode_delta(Arithmodel *, unsigned int, Index, Index);
int arithmodel_decode_gamma(Arithmodel *, unsigned int *, Index, Index);
int arithmodel_decode_delta(Arithmodel *, unsigned int *, Index, Index);
int arithmodel_encode_cbt(Arithmodel *, unsigned int, unsigned int, Index, Index);
int arithmodel_decode_cbt(Arithmodel *, unsigned int *, unsigned int, Index, Index);

#define arithmodel_encode_bit(am, b , b0, b1) arithmodel_encode_bits((am), (b) , 1, (b0), (b1))
#define arithmodel_decode_bit(am, br, b0, b1) arithmodel_decode_bits((am), (br), 1, (b0), (b1))

#endif
