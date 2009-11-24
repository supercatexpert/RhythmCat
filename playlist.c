/*
 *  Play List
 */

#include "declaration.h"

int plist_initial_playlist()
{
    list_group = g_list_alloc();
    gui_list_file_view_append(list_file_tree_view, "[Default]");
    return TRUE;
}
