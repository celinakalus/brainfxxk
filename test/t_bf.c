#include <stdint.h>
#include <string.h>
#include <tap.h>

#include "brainfxxk.h"

uint8_t next_read = 0;
uint8_t last_write = 0;

int mock_read(uint8_t *value) {
	*value = (uint8_t)(next_read);

	return 0;
}

int mock_write(uint8_t value) {
	last_write = value;

	return 0;
}

bf_interface mock_iface = {
	.read = mock_read,
	.write = mock_write
};

int main(int argc, char **argv) {
	int ret = 0;

	plan(32);

	bf_handle bf = NULL;

	ret = bf_init(&bf, &mock_iface);

	if (ret != 0) {
		BAIL_OUT("Could not allocate brainfxxk instance.\n");
	}

	const char jump_test[] = ",[,]";
	const size_t jump_test_len = strlen(jump_test);

	ret = bf_load_from_buffer(bf, jump_test, jump_test_len);

	if (ret != 0) {
		BAIL_OUT("Could not load code.\n");
	}

	bf_reset(bf);

	/* TEST 1: skip loop execution on 0 */
	ok(bf_get_ip(bf) == 0);
	ok(bf_get_sp(bf) == 0);

	/* read zero into current tape cell */
	next_read = 0;

	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 1);
	ok(bf_get_sp(bf) == 0);

	/* since current tape cell is zero,
	 * jump behind next closing bracket */
	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 4);
	ok(bf_get_sp(bf) == 0);

	/* end of code, bf_step should error */
	ok(bf_step(bf) != 0);

	bf_reset(bf);
	ok(bf_get_ip(bf) == 0);
	ok(bf_get_sp(bf) == 0);

	bf_reset(bf);

	/* TEST 2: Execute while non-zero */
	ok(bf_get_ip(bf) == 0);
	ok(bf_get_sp(bf) == 0);

	/* read one into current tape cell */
	next_read = 1;

	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 1);
	ok(bf_get_sp(bf) == 0);

	/* since current tape cell is non-zero,
	 * enter into the loop */
	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 2);
	ok(bf_get_sp(bf) == 1);

	/* read another 1 */
	next_read = 1;

	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 3);
	ok(bf_get_sp(bf) == 1);

	/* since current tape cell is non-zero,
	 * jump back to the start of the loop */
	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 2);
	ok(bf_get_sp(bf) == 1);

	/* Now read a zero instead */
	next_read = 0;

	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 3);
	ok(bf_get_sp(bf) == 1);

	/* since current tape cell is zero,
	 * exit the loop */
	ok(bf_step(bf) == 0);
	ok(bf_get_ip(bf) == 4);
	ok(bf_get_sp(bf) == 0);

	/* end of code, bf_step should error */
	ok(bf_step(bf) != 0);

	done_testing();
}
