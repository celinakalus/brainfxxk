#include "brainfxxk.h"
#include "tape.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct bf_instance {
	tape_handle tape;
	bf_interface *iface;
	char *code;
	long code_size;
	int instr_pointer;
	int stack[64];
	int stack_pointer;
};

int bf_init(bf_handle *hndl, bf_interface *iface) {
	struct bf_instance *inst = malloc(sizeof(struct bf_instance));
	*hndl = inst;

	if (inst == NULL) {
		return -1;
	}

	inst->code = NULL;

	tape_new(&(inst->tape));

	inst->iface = iface;

	return 0;
}

int bf_load(bf_handle hndl, FILE *file) {
	struct bf_instance *inst = hndl;
	int ret = 0;

	if (inst->code != NULL) {
		free(inst->code);
	}

	ret = fseek(file, 0l, SEEK_END);

	if (ret != 0) {
		return -1;
	}

	inst->code_size = ftell(file);
	inst->code = malloc(inst->code_size);

	if (inst->code == NULL) {
		return -1;
	}

	fseek(file, 0l, SEEK_SET);
	ret = fread(inst->code, sizeof(char), inst->code_size, file);

	if (ret != inst->code_size) {
		return -1;
	}

	return 0;
}

static int bf_reset_internal(struct bf_instance *inst) {
	inst->instr_pointer = 0;
	inst->stack_pointer = 0;

	return 0;
}

uint8_t bf_read_char(struct bf_instance *inst) {
	bf_interface *iface = inst->iface;
	uint8_t value;
	int ret = 0;

	if (iface == NULL || iface->read == NULL) {
		return 0;
	}

	ret = iface->read(&value);

	if (ret != 0) {
		return 0;
	}

	return value;
}

void bf_write_char(struct bf_instance *inst, uint8_t value) {
	bf_interface *iface = inst->iface;
	int ret = 0;

	if (iface == NULL || iface->read == NULL) {
		return;
	}

	ret = iface->write(value);
}

static int bf_step_internal(struct bf_instance *inst) {
	char instr = inst->code[inst->instr_pointer];
	printf("[debug] instruction %c at %i\n", instr, inst->instr_pointer);

	switch (instr) {
		case '+':
		case '-': {
			uint8_t value;
			tape_get(inst->tape, &value);
			value = instr == '+' ? value + 1 : value - 1;
			tape_set(inst->tape, value);
		} break;
		case '<':
		case '>': {
			int offset = instr == '>' ? 1 : -1;
			tape_move(inst->tape, offset);
		} break;
		case '.': {
			uint8_t value;
			tape_get(inst->tape, &value);
			bf_write_char(inst, value);
		} break;
		case ',': {
			uint8_t value;
			value = bf_read_char(inst);
			tape_set(inst->tape, value);
		} break;
		case '[': {
			inst->stack[inst->stack_pointer++] = inst->instr_pointer;
			printf("[debug] stack push %d @ %d\n", inst->stack[inst->stack_pointer - 1], inst->stack_pointer);
		} break;
		case ']': {
			if (inst->stack_pointer <= 0) {
				printf("Illegal stack access!\n");
				return -1;
			}

			int stack_value = inst->stack[inst->stack_pointer - 1];
			uint8_t tape_value;

			tape_get(inst->tape, &tape_value);

			if (tape_value > 0) {
				printf("[debug] stack peek %d @ %d\n", stack_value, inst->stack_pointer);
				inst->instr_pointer = stack_value;
			} else {
				printf("[debug] stack  pop %d @ %d\n", stack_value, inst->stack_pointer);
				inst->stack_pointer--;
			}
		} break;
	}

	inst->instr_pointer++;

	return 0;
}

int bf_execute(bf_handle hndl) {
	struct bf_instance *inst = hndl;

	while (inst->instr_pointer < inst->code_size) {
		bf_step_internal(inst);
	}

	return 0;
}

