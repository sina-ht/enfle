/*
 * vmpm_hash.h -- Hash related routines header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Apr 20 18:43:11 2001.
 */

#ifndef _VMPM_HASH_H
#define _VMPM_HASH_H

#define HASH_SIZE 1102967 /* 2365537, 3737323, 159437 */
typedef unsigned int Hash_value;

#define EMPTY_FLAG 0
#define USED_FLAG  (1 << 0)
#define EXTRA_FLAG (1 << 1)

#define is_used_token(t) ((t)->flag & USED_FLAG)
#define set_token_used(t) (t)->flag |= USED_FLAG
#define is_extra_token(t) ((t)->flag & EXTRA_FLAG)
#define unset_token_extra(t) ((t)->flag &= ~EXTRA_FLAG)

int register_to_token_hash(VMPM *, int, unsigned int, int, Token_value, Token **);

#endif
