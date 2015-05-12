/*
 * gl-driver-switch.c - GLX driver link management
 * 
 * Copyright 2015 Ikey Doherty <ikey@solus-project.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

#define streq(x,y) strcmp(x,y) == 0

#define GLX_DIR "/usr/lib/glx-provider"
#define TGT_DIR "/usr/lib"
#define DRIVER_DIR "/usr/lib/xorg/modules/extensions"

static inline const char *usage(void)
{
        return "Usage: %s set-link [name]\n";
}

static inline bool path_exists(const char *p)
{
        __attribute__ ((unused)) struct stat st = {0};
        return ((stat(p, &st) == 0));
}

static inline void _frees_(void *p)
{
        void **v = p;
        if (!v || !*v) {
                return;
        }
        free(*v);
}

#define _autofree_str __attribute__ ((cleanup(_frees_)))

/**
 * Swap the GL links out for the named configuration in GLX_DIR
 *
 * @param name Name, i.e. nvidia
 * @return a boolean value indicating success of the operation
 */
static bool update_links(const char *name)
{
        const char *paths[] = {
                "libGL.so.1", "libEGL.so.1", "libGLESv1_CM.so.1", "libGLESv2.so.2", "libglx.so.1"
        };


        for (uint i = 0; i < sizeof(paths)/sizeof(paths[0]); i++) {
                _autofree_str char *p = NULL;
                _autofree_str char *tgt = NULL;
                _autofree_str char *lbuf = NULL;

                const char *tdir = streq(paths[i], "libglx.so.1") ? DRIVER_DIR : TGT_DIR;
                /* Target name is actually libglx.so, but we read from libglx.so.1 */
                const char *lp = streq(paths[i], "libglx.so.1") ? "libglx.so" : paths[i];
                if (!asprintf(&p, "%s/%s/%s", GLX_DIR, name, paths[i])) {
                        fprintf(stderr, "No memory to complete action\n");
                        abort();
                }
                if (!(lbuf = realpath(p, NULL))) {
                        fprintf(stderr, "Cannot read link: %s\n", strerror(errno));
                        return false;
                }

                if (!asprintf(&tgt, "%s/%s", tdir, lp)) {
                        fprintf(stderr, "No memory to complete action\n");
                        abort();
                }
                if (path_exists((const char*)tgt)) {
                        if (unlink(tgt) != 0) {
                                fprintf(stderr, "Unable to remove %s: %s\n", tgt, strerror(errno));
                                return false;
                        }
                }
                if (symlink(lbuf, tgt) != 0) {
                        fprintf(stderr, "Unable to link %s: %s\n", lbuf, strerror(errno));
                        return false;
                }
                memset(&lbuf, 0, sizeof(lbuf));
        }
        return true;
}

int main(int argc, char **argv)
{
        const char *cmd = NULL;
        const char *tgt = NULL;

        if (argc < 3) {
                fprintf(stderr, usage(), argv[0]);
                return EXIT_FAILURE;
        }

        if (geteuid() != 0) {
                fprintf(stderr, "You must be root to use this utility\n");
                return EXIT_FAILURE;
        }

        cmd = argv[1];
        if (!streq(cmd, "set-link")) {
                fprintf(stderr, "Unknown command: %s\n", cmd);
                return EXIT_FAILURE;
        }
        tgt = argv[2];
        if (!streq(tgt, "nvidia")) {
                fprintf(stderr, "Unsupported driver: %s\n", tgt);
                return EXIT_FAILURE;
        }

        if (!update_links(tgt)) {
                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}
