/*
 * Settings Declaration
 */

#ifndef HAVE_SETTINGS_H
#define HAVE_SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <malloc.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include "global.h"

/* Variables */
RCSetting *rc_setting;

/* Functions */
void set_initial_setting();
RCSetting *get_setting();
void set_load_system_setting();
void set_load_user_setting();

/* Extern Functions */
extern gchar *rc_get_app_dir();
#endif
