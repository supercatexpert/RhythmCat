/*
 * GUI Style
 * Set the style (theme, skin, etc...) of the main window. 
 */

#include "gui_style.h"

void gui_style_reflush()
{
    RCSetting *setting = get_setting();
    if(setting->skin_rc_file==NULL) return;
    gtk_rc_parse(setting->skin_rc_file);
}


