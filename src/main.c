#include <stdint.h>
#include <stdio.h>
#include "brainfxxk.h"

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
		printf("Missing argument: file\n");
		return -1;
	}

	const char *fname = argv[1];

	FILE *f = fopen(fname, "r");

	if (f == NULL) {
		printf("File not found: \"%s\"\n", fname);
	}

	bf_handle hndl;
	int ret = 0;

	ret = bf_init(&hndl, &stdio_iface);

	if (ret != 0) {
		printf("Error while initializing brainfxxk: %i\n", ret);
	}

	ret = bf_load(hndl, f);

	if (ret != 0) {
		printf("Error while loading file: %i\n", ret);
	}

	ret = bf_execute(hndl);

	if (ret != 0) {
		printf("Error while executing: %i\n", ret);
	}

}
