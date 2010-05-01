/*
 * Tag (ID3) Declaration
 * This segment of codes (modified) is got from the QuePlayer, whose 
 * author is windwhinny, e-mail: windwhinny@gmail.com.
 */

#include "global.h"
#include <glib/gprintf.h>

gchar *extra_encoding = NULL;
gboolean skip_id3_reading = FALSE;

gchar **tag_get_id3(gchar *);

extern RCSetting *get_setting();
