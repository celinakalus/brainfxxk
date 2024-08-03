#ifndef INCLUDE_TAPE_H
#define INCLUDE_TAPE_H

#include <stdint.h>

typedef void* tape_handle;

int tape_new(tape_handle*);
int tape_move(tape_handle, int);
int tape_get(tape_handle, uint8_t*);
int tape_set(tape_handle, uint8_t);

#endif
