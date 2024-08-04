#include "tape.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define SEGMENT_SIZE 512

struct tape_segment {
	struct tape_segment *next;
	struct tape_segment *prev;
	size_t buffer_size;

	uint8_t buffer[];
};

#define BUFFER_SIZE (SEGMENT_SIZE - sizeof(struct tape_segment))

struct tape {
	struct tape_segment *head;
	int offset;
};

static struct tape_segment* tape_alloc_seg(void) {
	struct tape_segment* seg = malloc(SEGMENT_SIZE);
	memset(seg, 0, SEGMENT_SIZE);

	seg->next = NULL;
	seg->prev = NULL;
	seg->buffer_size = BUFFER_SIZE;

	return seg;
}

static void tape_stitch(struct tape_segment *left, struct tape_segment *right) {
	left->next = right;
	right->prev = left;
}

int tape_new(tape_handle* tape_h) {
	struct tape *t = malloc(sizeof(struct tape));
	t->offset = 0;
	t->head = tape_alloc_seg();

	*tape_h = t;

	return 0;
}

static int tape_advance(struct tape *t) {
	struct tape_segment *next_head = t->head->next;
	
	if (next_head == NULL) {
		next_head = tape_alloc_seg();

		if (next_head == NULL) { return -1; }

		tape_stitch(t->head, next_head);
	}

	t->head = next_head;

	return 0;
}

static int tape_rewind(struct tape *t) {
	struct tape_segment *next_head = t->head->prev;
	
	if (next_head == NULL) {
		next_head = tape_alloc_seg();

		if (next_head == NULL) { return -1; }

		tape_stitch(next_head, t->head);
	}

	t->head = next_head;

	return 0;
}

int tape_move(tape_handle tape_h, int offset) {
	struct tape *t = tape_h;

	int ret = 0;

	int old_offset = t->offset;
	long long total_offset = (long long)(offset) + (long long)(old_offset);

	while (1) {
		if (total_offset > BUFFER_SIZE) {
			ret = tape_advance(t);
			total_offset -= BUFFER_SIZE;
			//printf("[debug] advance tape\n");
		} else if (total_offset < 0) {
			ret = tape_rewind(t);
			total_offset += BUFFER_SIZE;
			//printf("[debug] rewind tape\n");
		} else {
			break;
		}
		
		if (ret != 0) { return ret; }
	}

	t->offset = (int)(total_offset);

	//printf("[debug] tape offset changed %d -> %d\n", old_offset, t->offset);

	return 0;
}

int tape_get(tape_handle tape_h, uint8_t* value) {
	struct tape *t = tape_h;

	*value = t->head->buffer[t->offset];
	//printf("[debug] get value at %d: %02x\n", t->offset, *value);

	return 0;
}

int tape_set(tape_handle tape_h, uint8_t value) {
	struct tape *t = tape_h;

	//printf("[debug] set value at %d: %02x\n", t->offset, value);
	t->head->buffer[t->offset] = value;

	return 0;
}
