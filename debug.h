/*
 * Debug Declaration
 */

#ifndef HAVE_DEBUG_H
#define HAVE_DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <glib.h>
#include "global.h"

#define DEBUG_MODE 0

int rc_debug_get_flag();
void rc_debug_set_mode(int);
gint rc_debug_print(const gchar *, ...);


#endif

