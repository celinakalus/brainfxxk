#ifndef INCLUDE_BRAINFXXK_H
#define INCLUDE_BRAINFXXK_H

#include <stdint.h>
#include <stdio.h>

typedef void* bf_handle;

typedef int (*bf_read_cb)(uint8_t*);
typedef int (*bf_write_cb)(uint8_t);

/** read/write interface for ./, instructions */
typedef struct {
	bf_read_cb read;
	bf_write_cb write;
} bf_interface;

int bf_init(bf_handle*, bf_interface*);
int bf_load(bf_handle, FILE*);
int bf_load_from_buffer(bf_handle, const char*, const size_t);
int bf_step(bf_handle);
int bf_execute(bf_handle);
int bf_reset(bf_handle);
int bf_get_ip(bf_handle);
int bf_get_sp(bf_handle);

#endif
