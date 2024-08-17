#include <stdint.h>
#include <tap.h>

#include "tape.h"

int main(int argc, char **argv) {
	plan(22);

	tape_handle tape = NULL;
	tape_new(&tape);

	if (tape == NULL) {
		BAIL_OUT("tape is still NULL!");
	}

	uint8_t value;
	tape_get(tape, &value);

	/* tape value should be initialized to zero */
	ok(value == 0);

	/* tape should remember value at current cell */
	ok(tape_set(tape, 0x79u) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0x79u);

	/* tape should remember cell value after moving */
	ok(tape_move(tape, 1) >= 0);
	ok(tape_move(tape, -1) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0x79u);

	/* tape should be able to move far to the right */
	ok(tape_move(tape, 10000) >= 0);
	ok(tape_set(tape, 0x34u) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0x34u);

	/* tape should remember previous values */
	ok(tape_move(tape, -10000) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0x79u);

	/* tape should be able to move far to the left */
	ok(tape_move(tape, -10000) >= 0);
	ok(tape_set(tape, 0xA8u) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0xA8u);

	/* tape should remember value on far right */
	ok(tape_move(tape, 20000) >= 0);
	ok(tape_get(tape, &value) >= 0);
	ok(value == 0x34u);

	done_testing();
}
