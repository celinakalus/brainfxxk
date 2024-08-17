cmake_minimum_required(VERSION 3.10)

project(brainfxxk C)

include(CTest)

add_executable(brainfxxk
	src/brainfxxk.c
	src/main.c
	src/log.c
	src/tape.c
)

target_compile_options(brainfxxk
	PRIVATE -Wall -Wextra -Wpedantic -Werror
)

add_executable(test_tape
	src/tape.c
	src/log.c
	test/t_tape.c
)

include_directories(src)

find_library(LIBTAP_PATH NAMES tap libtap)
target_link_libraries(test_tape PRIVATE ${LIBTAP_PATH})

add_test(test_tape test_tape)
set_property(
	TEST test_tape
	PROPERTY FAIL_REGULAR_EXPRESSION "not ok"
)
