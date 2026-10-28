/*
 * RhythmCat Library Instance Lock Header Declaration
 *
 * rclib-instance.h
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef HAVE_RC_LIB_INSTANCE_H
#define HAVE_RC_LIB_INSTANCE_H

#include <glib.h>

G_BEGIN_DECLS

#define RCLIB_INSTANCE_ERROR rclib_instance_error_quark()

/**
 * RCLibInstanceErrorCode:
 * @RCLIB_INSTANCE_ERROR_ALREADY_RUNNING: another instance is running
 * @RCLIB_INSTANCE_ERROR_LOCK_FAILED: cannot create the lock file
 *
 * The enum type for instance lock error messages.
 */

typedef enum {
    RCLIB_INSTANCE_ERROR_ALREADY_RUNNING = 1,
    RCLIB_INSTANCE_ERROR_LOCK_FAILED = 2
}RCLibInstanceErrorCode;

GQuark rclib_instance_error_quark();
gboolean rclib_instance_lock(const gchar *frontend, GError **error);
void rclib_instance_unlock();
gboolean rclib_instance_is_locked();

G_END_DECLS

#endif

