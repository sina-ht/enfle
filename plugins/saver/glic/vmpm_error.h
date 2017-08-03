/*
 * vmpm_error.h -- Error messages
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Aug  7 17:19:15 2001.
 */

#ifndef _VMPM_ERROR_H
#define _VMPM_ERROR_H

#define OPEN_ERROR 1
#define MEMORY_ERROR 2
#define READ_ERROR 3
#define NO_DECOMPOSER_ERROR 4
#define UNKNOWN_OPTION_ERROR 5
#define NOT_IMPLEMENTED_ERROR 6
#define HASH_ERROR 7
#define INVALID_INPUT_NAME_ERROR 8
#define HEADER_ERROR 9
#define INVALID_INDEX 10
#define DECOMPOSER_SPECIFIED_ERROR 11
#define INVALID_TOKEN_VALUE_ERROR 12
#define INTERNAL_ERROR 255

#ifdef DEBUG
# define debug_message(format, args...) fprintf(stderr, format, ## args)
#else
# define debug_message(format, args...)
#endif

void generic_error(char *, int) __attribute__ ((noreturn));
void memory_error(char *, int) __attribute__ ((noreturn));
void open_error(char *, int) __attribute__ ((noreturn));

#endif
