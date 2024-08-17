cmake_minimum_required(VERSION 3.10)

project(brainfxxk C)

add_executable(brainfxxk
	src/brainfxxk.c
	src/main.c
	src/log.c
	src/tape.c)

target_compile_options(brainfxxk
	PRIVATE -Wall -Wextra -Wpedantic -Werror
)
