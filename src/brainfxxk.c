#include "brainfxxk.h"
#include "tape.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

LOG_MODULE(brainfxxk);

struct bf_instance {
	tape_handle tape;
	bf_interface *iface;
	char *code;
	long code_size;
	int instr_pointer;
	int stack[64];
	int stack_pointer;
	enum {
		STATUS_OK,
		STATUS_ERR
	} status;
};

static int bf_reset_internal(struct bf_instance *inst) {
	inst->instr_pointer = 0;
	inst->stack_pointer = 0;

	inst->status = STATUS_OK;

	return 0;
}

int bf_init(bf_handle *hndl, bf_interface *iface) {
	struct bf_instance *inst = malloc(sizeof(struct bf_instance));
	*hndl = inst;

	if (inst == NULL) {
		LOG_ERR("Could not allocate memory for instance.\n");
		return -1;
	}

	inst->code = NULL;

	tape_new(&(inst->tape));

	inst->iface = iface;

	bf_reset_internal(inst);

	return 0;
}

int bf_load(bf_handle hndl, FILE *file) {
	struct bf_instance *inst = hndl;
	int ret = 0;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

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

int bf_load_from_buffer(bf_handle hndl, const char *buf, const size_t size) {
	struct bf_instance *inst = hndl;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

	if (buf == NULL) {
		return -1;
	}

	inst->code_size = size;
	inst->code = malloc(size);

	memcpy(inst->code, buf, size);

	return 0;
}

static uint8_t bf_read_char(struct bf_instance *inst) {
	bf_interface *iface = inst->iface;
	uint8_t value;
	int ret = 0;

	if (iface == NULL || iface->read == NULL) {
		LOG_DBG("No interface given, returning 0.\n");
		return 0;
	}

	ret = iface->read(&value);

	if (ret != 0) {
		LOG_WRN("Error while reading; Error code: %i\n", ret);
		return 0;
	}

	LOG_DBG("Read value %c (%02x)\n", value, value);
	return value;
}

static void bf_write_char(struct bf_instance *inst, uint8_t value) {
	bf_interface *iface = inst->iface;
	int ret = 0;

	if (iface == NULL || iface->read == NULL) {
		LOG_DBG("No interface given, returning 0.\n");
		return;
	}

	ret = iface->write(value);

	if (ret != 0) {
		LOG_WRN("Error while reading; Error code: %i\n", ret);
	}

	LOG_DBG("Written value %c (%02x)\n", value, value);
}

static int bf_step_internal(struct bf_instance *inst) {
	char instr = inst->code[inst->instr_pointer];
	LOG_DBG("instruction %c at %i\n", instr, inst->instr_pointer);

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
			uint8_t value;
			tape_get(inst->tape, &value);
			if (value != 0) {
				inst->stack[inst->stack_pointer++] = inst->instr_pointer;
				LOG_DBG("stack push %d @ %d\n", inst->stack[inst->stack_pointer - 1], inst->stack_pointer);
			} else {
				int rec_height = 1;
				int next_instr_pointer = inst->instr_pointer;

				while (next_instr_pointer < inst->code_size) {
					char next_inst = inst->code[++next_instr_pointer];

					if (next_inst == '[') {
						rec_height++;
					} else if (next_inst == ']') {
						rec_height--;
					}

					if (rec_height == 0) { break; }
				}

				if (rec_height != 0) {
					LOG_ERR("Unmatched open bracket at %i\n",
							inst->instr_pointer);
					return -1;
				}

				LOG_DBG("Jumping forward from %i to %i\n",
						inst->instr_pointer, next_instr_pointer);
				inst->instr_pointer = next_instr_pointer;
			}
		} break;
		case ']': {
			if (inst->stack_pointer <= 0) {
				LOG_ERR("Illegal stack access! (ip: %i, sp: %i)\n",
						inst->instr_pointer, inst->stack_pointer);
				return -1;
			}

			int stack_value = inst->stack[inst->stack_pointer - 1];
			uint8_t tape_value;

			tape_get(inst->tape, &tape_value);

			if (tape_value > 0) {
				LOG_DBG("stack peek %d @ %d\n", stack_value, inst->stack_pointer);
				inst->instr_pointer = stack_value;
			} else {
				LOG_DBG("stack  pop %d @ %d\n", stack_value, inst->stack_pointer);
				inst->stack_pointer--;
			}
		} break;
	}

	inst->instr_pointer++;

	return 0;
}

int bf_step(bf_handle hndl) {
	struct bf_instance *inst = hndl;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

	if (inst->code == NULL) {
		LOG_ERR("No code loaded.\n");
		return -1;
	}

	if (inst->instr_pointer >= inst->code_size) {
		LOG_ERR("Instruction pointer outside code! (ip: %i, sp: %i)\n",
				inst->instr_pointer, inst->stack_pointer);
		return -1;
	}

	if (inst->status != STATUS_OK) {
		LOG_ERR("Cannot execute further due to previous error.\n");
		return -1;
	}

	int ret = bf_step_internal(inst);

	if (ret != 0) {
		inst->status = STATUS_ERR;
		LOG_INF("Encountered error during step.\n");
	}

	return ret;
}

int bf_execute(bf_handle hndl) {
	struct bf_instance *inst = hndl;
	int ret = 0;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

	if (inst->code == NULL) {
		LOG_ERR("No code loaded.\n");
		return -1;
	}

	if (inst->status != STATUS_OK) {
		LOG_ERR("Cannot execute further due to previous error.\n");
		return -1;
	}

	while (inst->instr_pointer < inst->code_size) {
		ret = bf_step_internal(inst);

		if (ret != 0) {
			LOG_INF("Aborting execution due to error.\n");
			inst->status = STATUS_ERR;
			break;
		}
	}

	return ret;
}

int bf_reset(bf_handle hndl) {
	struct bf_instance *inst = hndl;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

	return bf_reset_internal(inst);
}

int bf_get_ip(bf_handle hndl) {
	struct bf_instance *inst = hndl;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}
	
	return inst->instr_pointer;
}

int bf_get_sp(bf_handle hndl) {
	struct bf_instance *inst = hndl;

	if (inst == NULL) {
		LOG_ERR("Given instance invalid.\n");
		return -1;
	}

	return inst->stack_pointer;
}
