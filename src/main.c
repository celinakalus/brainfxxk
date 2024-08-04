#include <stdint.h>
#include <stdio.h>
#include "brainfxxk.h"

#include "log.h"

LOG_MODULE(main);

int stdio_read(uint8_t *value) {
	int ret = 0;

	ret = getc(stdin);

	if (ret == EOF) {
		return -1;
	}

	*value = (uint8_t)(ret);

	return 0;
}

int stdio_write(uint8_t value) {
	int ret = 0;

	ret = putc(value, stdout);

	if (ret == EOF) {
		return -1;
	}

	return 0;
}

bf_interface stdio_iface = {
	.read = stdio_read,
	.write = stdio_write
};

int main(int argc, char **argv) {
	if (argc < 2) {
		LOG_ERR("Missing argument: file\n");
		return -1;
	}

	const char *fname = argv[1];

	FILE *f = fopen(fname, "r");

	if (f == NULL) {
		LOG_ERR("File not found: \"%s\"\n", fname);
	}

	bf_handle hndl;
	int ret = 0;

	ret = bf_init(&hndl, &stdio_iface);

	if (ret != 0) {
		LOG_ERR("Error while initializing brainfxxk: %i\n", ret);
		return -1;
	}

	ret = bf_load(hndl, f);

	if (ret != 0) {
		LOG_ERR("Error while loading file: %i\n", ret);
		return -1;
	}

	ret = bf_execute(hndl);

	if (ret != 0) {
		LOG_ERR("Error while executing: %i\n", ret);
		return -1;
	}

	return 0;
}
