#include <stdio.h>
#include <stdarg.h>
#include "log.h"

static log_level global_log_level = LOG_LEVEL_INFO;

void io_log(log_module *log_mod, log_level ll, const char *format, ...) {
	if (ll <= global_log_level) {
		switch (ll) {
			case LOG_LEVEL_INFO: {
				printf("\x1B[38;5;2m[info]\x1B[39m <%s> ", log_mod->module_name);
			} break;
			case LOG_LEVEL_DEBUG: {
				printf("\x1B[38;5;3m[debug]\x1B[39m <%s> ", log_mod->module_name);
			} break;
			case LOG_LEVEL_WARN: {
				printf("\x1B[38;5;9m[warn]\x1B[39m <%s> ", log_mod->module_name);
			} break;
			case LOG_LEVEL_ERROR: {
				printf("\x1B[38;5;1m[error]\x1B[39m <%s> ", log_mod->module_name);
			} break;
		}

		va_list args;

		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

void io_set_log_level(log_level ll) {
	global_log_level = ll;
}
