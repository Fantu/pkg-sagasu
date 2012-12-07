/*  $Id: ResultPage.h,v 1.4 2012/11/25 00:58:21 sarrazip Exp $
    ResultPage.h - Text buffer containing the results of a search

    sagasu - GNOME tool to find strings in a set of files
    Copyright (C) 2002-2004 Pierre Sarrazin <http://sarrazip.com/>

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

#ifndef _H_ResultPage
#define _H_ResultPage

#include "util.h"

#include <gtk/gtk.h>


class ResultPage
{
public:

    ResultPage();

    ~ResultPage();

    GtkWidget *get_top_widget() const;

    GtkWidget *get_text_view();

    void clear();

    void insert_text(const std::string &s);

    void set_status_label(const char *utf8_string);

    gint get_text_pos_from_window_coords(gdouble ex, gdouble ey);

    gint get_char_count() const;
    /*
	Returns the number of UTF-8 characters in the text buffer.
    */

    std::string get_all_text() const;
    /*
	Returns a UTF-8 string.
    */

    void select_text_region(gint start, gint end);

    gint get_next_search_pos() const;
    void set_next_search_pos(gint pos);

private:

    GtkWidget *vbox;
    GtkWidget *sw;  // scrolled window
    GtkWidget *text_view;  // text view
    GtkWidget *status_label;
    gint next_search_pos;  // index into the text buffer

    // Forbidden operations:
    ResultPage(const ResultPage &);
    ResultPage &operator = (const ResultPage &);
};


#endif  /* _H_ResultPage */
