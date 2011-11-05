/*
 * Signals
 * Handle signals on POSIX enviroment
 *
 * signal.c
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - Mike Manilone, license: GPL v3
 *
 * <RhythmCat> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * <RhythmCat> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <RhythmCat>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
#include <glib.h>

#ifdef G_OS_UNIX
# include <unistd.h>
# include <signal.h>
#else
# include <windows.h>
# include <signal.h>
#endif

#include "player.h"

void 
rc_signal_register (void)
{
    signal (SIGINT, rc_player_exit);
#ifdef G_OS_UNIX
    signal (SIGTERM, rc_player_exit);
#endif
}



