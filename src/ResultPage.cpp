/*  $Id: ResultPage.cpp,v 1.7 2012/11/25 00:58:21 sarrazip Exp $
    ResultPage.cpp - Text buffer containing the results of a search

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

#include "ResultPage.h"

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include <assert.h>
#include <iostream>

using namespace std;


ResultPage::ResultPage()
  : vbox(gtk_vbox_new(FALSE, 0)),
    sw(gtk_scrolled_window_new(NULL, NULL)),
    text_view(gtk_text_view_new()),
    status_label(gtk_label_new(_("No results yet."))),
    next_search_pos(0)
{
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), false);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(sw), text_view);

    gtk_misc_set_alignment(GTK_MISC(status_label), 0.0, 0.5);

    gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(vbox), "ResultPage", this);
	/*
	    We make the vbox point to this object, because in some
	    situations, we will only have a pointer to the vbox,
	    and we will want to retrieve the pointer to the containing
	    ResultPage object.
	*/

    gtk_widget_show_all(GTK_WIDGET(vbox));
}


ResultPage::~ResultPage()
{
}


GtkWidget *
ResultPage::get_top_widget() const
{
    return vbox;
}


GtkWidget *
ResultPage::get_text_view()
{
    return text_view;
}


void
ResultPage::clear()
{
    assert(GTK_IS_TEXT_VIEW(text_view));

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buf), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buf), &end);
    gtk_text_buffer_delete(GTK_TEXT_BUFFER(buf), &start, &end);
}


void
ResultPage::insert_text(const string &s)
{
    assert(GTK_IS_TEXT_VIEW(text_view));

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buf), &iter);
    GCharPtr u(u8(s));
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(buf), &iter, u.get(), -1);
}


void
ResultPage::set_status_label(const char *utf8_string)
{
    gtk_label_set_text(GTK_LABEL(status_label), utf8_string);
}


gint
ResultPage::get_text_pos_from_window_coords(gdouble ex, gdouble ey)
{
    gint x, y;
    gtk_text_view_window_to_buffer_coords(
		    GTK_TEXT_VIEW(text_view),
		    GTK_TEXT_WINDOW_TEXT,
		    (gint) ex, (gint) ey,
		    &x, &y);
    GtkTextIter it;
    gint line_top = -1;
    gtk_text_view_get_line_at_y(
		    GTK_TEXT_VIEW(text_view), &it, y, &line_top);
    return gtk_text_iter_get_offset(&it);
}


gint
ResultPage::get_char_count() const
{
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    return gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(buf));
}


string
ResultPage::get_all_text() const
{
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(buf), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buf), &end);
    return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buf), &start, &end, false);
}


void
ResultPage::select_text_region(gint start, gint end)
{
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    GtkTextIter start_it, end_it;
    gtk_text_buffer_get_iter_at_offset(buf, &start_it, start);
    gtk_text_buffer_place_cursor(buf, &start_it);

    gtk_text_buffer_get_iter_at_offset(buf, &end_it, end);
    GtkTextMark *mark = gtk_text_buffer_get_selection_bound(buf);
    gtk_text_buffer_move_mark(buf, mark, &end_it);

    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view),
				&start_it, 0.0, false, 0.0, 0.0);
}


gint
ResultPage::get_next_search_pos() const
{
    return next_search_pos;
}


void
ResultPage::set_next_search_pos(gint pos)
{
    assert(pos >= 0);
    next_search_pos = pos;
}
