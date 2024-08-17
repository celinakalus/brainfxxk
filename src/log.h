#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

#include <stdint.h>

typedef enum {
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_WARN,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG
} log_level;

typedef struct {
	char* module_name;
	uint32_t log_flags;
} log_module;

void io_log(log_module *log_mod, log_level ll, const char *format, ...);

#define LOG_MODULE(mname) \
	static log_module local_log_module = { \
		.module_name = #mname, \
	}

#define LOG_INF(...) io_log(&local_log_module, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_DBG(...) io_log(&local_log_module, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_WRN(...) io_log(&local_log_module, LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERR(...) io_log(&local_log_module, LOG_LEVEL_ERROR, __VA_ARGS__)

void io_set_log_level(log_level ll);

#endif
