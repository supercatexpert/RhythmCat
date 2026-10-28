/*
 * RhythmCat Library Instance Lock Module
 * Make sure only one frontend of the player is running.
 *
 * rclib-instance.c
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
#include "rclib-instance.h"
#include "rclib-common.h"
#include <glib/gstdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/**
 * SECTION: rclib-instance
 * @Short_description: The single instance lock
 * @Title: Instance Lock
 * @Include: rclib-instance.h
 *
 * Only one frontend of the player can be run at the same time. The
 * frontend should call rclib_instance_lock() before initializing the
 * library, if it fails, another instance is already running. The lock
 * is an advisory lock (flock) on a file in the user runtime directory,
 * so it will be released automatically by the system even if the
 * player crashed.
 */

static gint instance_lock_fd = -1;
static gchar *instance_lock_file = NULL;

GQuark rclib_instance_error_quark()
{
    return g_quark_from_static_string("rclib-instance-error-quark");
}

/**
 * rclib_instance_lock:
 * @frontend: the name of the frontend which is going to run
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Try to get the instance lock.
 *
 * Returns: Whether the lock is acquired, if another instance is already
 *     running, %FALSE will be returned and @error will be set to
 *     #RCLIB_INSTANCE_ERROR_ALREADY_RUNNING.
 */

gboolean rclib_instance_lock(const gchar *frontend, GError **error)
{
    gchar buffer[256];
    gchar *info;
    gssize size;
    gint fd;
    if(instance_lock_fd>=0) return TRUE;
    if(instance_lock_file==NULL)
    {
        instance_lock_file = g_build_filename(g_get_user_runtime_dir(),
            "RhythmCat3.lock", NULL);
    }
    fd = g_open(instance_lock_file, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
    if(fd<0)
    {
        g_set_error(error, RCLIB_INSTANCE_ERROR,
            RCLIB_INSTANCE_ERROR_LOCK_FAILED,
            _("Cannot open lock file %s: %s"), instance_lock_file,
            g_strerror(errno));
        return FALSE;
    }
    if(flock(fd, LOCK_EX | LOCK_NB)!=0)
    {
        if(errno==EWOULDBLOCK)
        {
            size = read(fd, buffer, sizeof(buffer)-1);
            if(size<0) size = 0;
            buffer[size] = '\0';
            g_strchomp(buffer);
            if(buffer[0]!='\0')
            {
                g_set_error(error, RCLIB_INSTANCE_ERROR,
                    RCLIB_INSTANCE_ERROR_ALREADY_RUNNING,
                    _("There is already one instance of RhythmCat running "
                    "(%s), only one instance can be run at the same time."),
                    buffer);
            }
            else
            {
                g_set_error(error, RCLIB_INSTANCE_ERROR,
                    RCLIB_INSTANCE_ERROR_ALREADY_RUNNING,
                    _("There is already one instance of RhythmCat running, "
                    "only one instance can be run at the same time."));
            }
        }
        else
        {
            g_set_error(error, RCLIB_INSTANCE_ERROR,
                RCLIB_INSTANCE_ERROR_LOCK_FAILED,
                _("Cannot lock file %s: %s"), instance_lock_file,
                g_strerror(errno));
        }
        close(fd);
        return FALSE;
    }
    info = g_strdup_printf(_("%s, PID %d"), frontend!=NULL ? frontend :
        "unknown", (gint)getpid());
    if(ftruncate(fd, 0)==0)
    {
        if(write(fd, info, strlen(info))<0)
            g_warning("Cannot write instance information to lock file!");
    }
    g_free(info);
    instance_lock_fd = fd;
    return TRUE;
}

/**
 * rclib_instance_unlock:
 *
 * Release the instance lock.
 */

void rclib_instance_unlock()
{
    if(instance_lock_fd<0) return;
    if(ftruncate(instance_lock_fd, 0)!=0)
        g_warning("Cannot clear lock file!");
    flock(instance_lock_fd, LOCK_UN);
    close(instance_lock_fd);
    instance_lock_fd = -1;
}

/**
 * rclib_instance_is_locked:
 *
 * Check whether this process holds the instance lock.
 *
 * Returns: Whether the lock is held by this process.
 */

gboolean rclib_instance_is_locked()
{
    return instance_lock_fd>=0;
}

