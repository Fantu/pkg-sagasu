/*  $Id: util.cpp,v 1.8 2010/05/31 00:11:49 sarrazip Exp $
    util.cpp - Various utilities

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/

#include "util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include <gtk/gtk.h>
#include <libgnome/gnome-url.h>

#include <cstring>

using namespace std;


gchar *
latin1(const gchar *utf8_string)
{
    gchar *result = g_locale_from_utf8(utf8_string, -1, NULL, NULL, NULL);
    if (result == NULL)
	return g_strdup(utf8_string);
    return result;
}


string
latin1_string(const char *utf8_string)
{
    g_return_val_if_fail(utf8_string != NULL, "");
    GCharPtr latin1_string(latin1(utf8_string));
    return latin1_string.get();
}


string
latin1_string(const string &utf8_string)
{
    return latin1_string(utf8_string.c_str());
}


string &
chomp(string &s, char c)
{
    string::size_type len = s.length();
    if (len > 0 && s[len - 1] == c)
	s.erase(len - 1, 1);
    return s;
}


string &
substitute(string &s, const string &target, const string &replacement)
{
    string::size_type targetLen = target.length();
    string::size_type pos;
    while (pos = s.find(target), pos != string::npos)
	s.replace(pos, targetLen, replacement);
    return s;
}


string &
quotechars(string &s, const char *metachars)
{
    string result;
    string::size_type len = s.length();
    for (string::size_type i = 0; i < len; i++)
    {
	if (strchr(metachars, s[i]) != NULL)
	    result += '\\';
	result += s[i];
    }
    s = result;
    return s;
}


string
get_dir(const char *default_value, const char *env_var_name)
{
    const char *s = getenv(env_var_name);
    string dir = (s != NULL ? s : default_value);

    if (!dir.empty() && dir[dir.length() - 1] != '/')
	dir += '/';

    return dir;
}


void
entry_set_text(GtkWidget *entry, const string &utf8_string)
{
    gtk_entry_set_text(GTK_ENTRY(entry), utf8_string.c_str());
}


void
error_dialog(GtkWidget *window, const string &utf8_message)
{
    GtkWidget *dlg = gtk_message_dialog_new(
			GTK_WINDOW(window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			utf8_message.c_str());
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}


void
errno_dialog(GtkWidget *window, const string &utf8_first_line, int errnum)
{
    string utf8_msg = utf8_first_line + ":\n" + u8_string(strerror(errnum));
    error_dialog(window, utf8_msg);
}


int
show_url(const string &url, GtkWidget *window, const string &utf8_error_message)
{
    GError *err = NULL;
    if (gnome_url_show(url.c_str(), &err))
	return 0;
    string msg = utf8_error_message + ":\n"
		+ err->message + "\n"
		+ _("URL:") + " " + url;
    error_dialog(window, msg);
    g_error_free(err);
    return -1;
}
