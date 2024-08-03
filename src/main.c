#include <stdint.h>
#include <stdio.h>
#include "tape.h"

const char code[] = "-,+[-[>>++++[>++++++++<-]<+<-[>+>+>-[>>>]<[[>+<-]>>+>]<<<<<-]]>>>[-]+>--[-[<->+++[-]]]<[++++++++++++<[>-[>+>>]>[+[<+>-]>+>>]<<<<<-]>>[<+>-]>[-[-<<[-]>>]<<[<<->>-]>>]<<[<<+>>-]]<[-]<.[-]<-,+]";

int main(int argc, char **argv) {
	int ret = 0;
	tape_handle tape;

	tape_new(&tape);

	int stack[64] = { 0, };
	int stack_pointer = 0;

	int instr_pointer = 0;

	while (1) {
		char instr = code[instr_pointer];
		//printf("[debug] instruction %c at %i\n", instr, instr_pointer);

		switch (instr) {
			case '+':
			case '-': {
				uint8_t value;
				tape_get(tape, &value);
				value = instr == '+' ? value + 1 : value - 1;
				tape_set(tape, value);
			} break;
			case '<':
			case '>': {
				int offset = instr == '>' ? 1 : -1;
				tape_move(tape, offset);
			} break;
			case '.': {
				uint8_t value;
				tape_get(tape, &value);
				putc((int)(value), stdout);
			} break;
			case ',': {
				uint8_t value;
				value = (char)getc(stdin);
				tape_set(tape, value);
			} break;
			case '[': {
				stack[stack_pointer++] = instr_pointer;
				//printf("[debug] stack push %d @ %d\n", stack[stack_pointer - 1], stack_pointer);
			} break;
			case ']': {
				if (stack_pointer <= 0) {
					//printf("Illegal stack access!\n");
					return -1;
				}

				int stack_value = stack[stack_pointer - 1];
				uint8_t tape_value;

				tape_get(tape, &tape_value);

				if (tape_value > 0) {
					//printf("[debug] stack peek %d @ %d\n", stack_value, stack_pointer);
					instr_pointer = stack_value;
				} else {	
					//printf("[debug] stack  pop %d @ %d\n", stack_value, stack_pointer);
					stack_pointer--;
				}
			} break;
		}

		instr_pointer++;

		if (instr_pointer >= sizeof(code)) {
			break;
		}
	}
	return 0;
}
