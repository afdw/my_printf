cmake_minimum_required(VERSION 3.18)
project(my_printf C)

set(CMAKE_C_STANDARD 11)

add_library(my_printf src/library.c src/library.h src/parsing.c src/parsing.h)

add_custom_command(OUTPUT libftprintf.o
        COMMAND ld -o libftprintf.o -r --whole-archive libmy_printf.a --defsym=ft_printf=my_printf
        DEPENDS my_printf
        )
add_custom_command(OUTPUT libftprintf.a
        COMMAND ar rcs libftprintf.a libftprintf.o
        DEPENDS libftprintf.o
        )
add_custom_target(ftprintf ALL DEPENDS libftprintf.a)

add_executable(example example/main.c)
target_link_libraries(example my_printf)
