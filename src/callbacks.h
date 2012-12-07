/*  $Id: callbacks.h,v 1.7 2012/11/25 00:58:22 sarrazip Exp $
    callbacks.h - partial list of callbacks formatted for preprocessor tricks

    sagasu - GNOME tool to find strings in a set of files
    Copyright (C) 2002-2012 Pierre Sarrazin <http://sarrazip.com/>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

WIDGET_AND_POINTER_CALLBACK(create_new_result_page_cb)
WIDGET_AND_POINTER_CALLBACK(exit_cb)
WIDGET_AND_POINTER_CALLBACK(first_tab_cb)
WIDGET_AND_POINTER_CALLBACK(last_tab_cb)
WIDGET_AND_POINTER_CALLBACK(previous_tab_cb)
WIDGET_AND_POINTER_CALLBACK(next_tab_cb)
WIDGET_AND_POINTER_CALLBACK(erase_search_string_cb)
WIDGET_AND_POINTER_CALLBACK(tab_close_clicked_cb)
WIDGET_AND_POINTER_CALLBACK(close_current_tab_cb)
WIDGET_AND_POINTER_CALLBACK(search_cb)
WIDGET_AND_POINTER_CALLBACK(default_file_patterns_cb)
WIDGET_AND_POINTER_CALLBACK(browse_search_dir_cb)
WIDGET_AND_POINTER_CALLBACK(default_excl_dirs_cb)
WIDGET_AND_POINTER_CALLBACK(default_editor_cmd_cb)
WIDGET_AND_POINTER_CALLBACK(entry_changed_cb)
WIDGET_AND_POINTER_CALLBACK(result_search_entry_changed_cb)
WIDGET_AND_POINTER_CALLBACK(result_search_button_clicked_cb)
WIDGET_AND_POINTER_CALLBACK(home_page_cb)
