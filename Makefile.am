EXTRA_DIST = \
	README.md \
	LICENSE

AM_CFLAGS =  -fstack-protector -Wall -Wno-pedantic \
        -Wstrict-prototypes -Wundef -fno-common \
        -Werror-implicit-function-declaration \
        -Wformat -Wformat-security -Werror=format-security \
        -Wno-conversion -Wunreachable-code -std=c99 -Werror

bin_PROGRAMS = gl-driver-switch

gl_driver_switch_SOURCES = \
	gl-driver-switch.c

gl_driver_switch_CFLAGS = \
	$(AM_CFLAGS)
