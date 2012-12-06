/*  $Id: util.h,v 1.8 2010/05/19 01:12:10 sarrazip Exp $
    util.h - Various utilities

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

#ifndef _H_util
#define _H_util

#include <glib/gconvert.h>
#include <glib/gmem.h>
#include <gtk/gtkwidget.h>
#include <string>


class GCharPtr
/*
    Container for a pointer to dynamically allocated memory that
    automates the call to g_free().
*/
{
public:
    GCharPtr(gchar *s = NULL) : ptr(s) {}
    ~GCharPtr() { g_free(ptr); }
    const gchar *get() const { return ptr; }
    gchar *get() { return ptr; }
    void release() { ptr = NULL; }

private:
    gchar *ptr;

    // Forbidden operations:
    GCharPtr(const GCharPtr &);
    GCharPtr &operator = (const GCharPtr &);
};


inline
gchar *
u8(const gchar *latin1_string)
/*
    Converts a string from Latin-1 to UTF-8.
    The returned string must be freed with g_free().
*/
{
    return g_locale_to_utf8(latin1_string, -1, NULL, NULL, NULL);
}


inline
gchar *
u8(const std::string &latin1_string)
/*
    Converts a string from Latin-1 to UTF-8.
    The returned string must be freed with g_free().
*/
{
    return u8(latin1_string.c_str());
}


inline
std::string
u8_string(const char *latin1_string)
/*
    Converts a string from Latin-1 to UTF-8.
*/
{
    return GCharPtr(u8(latin1_string)).get();
}


inline
std::string
u8_string(const std::string &latin1_string)
/*
    Converts a string from Latin-1 to UTF-8.
*/
{
    return GCharPtr(u8(latin1_string)).get();
}


gchar *latin1(const gchar *utf8_string);
/*
    Converts a string from UTF-8 to Latin-1 .
    In case of a conversion error, returns a copy of utf8_string
    allocated with g_strdup().
    In all cases, the returned pointer must be submitted to
    g_free() to liberate the associated memory.
*/


std::string latin1_string(const char *utf8_string);
/*
    Converts a string from UTF-8 to Latin-1 .
    In case of a conversion error, returns a copy of utf8_string.
*/


std::string latin1_string(const std::string &utf8_string);
/*
    Converts a string from UTF-8 to Latin-1 .
    In case of a conversion error, returns a copy of utf8_string.
*/


std::string &chomp(std::string &s, char c = '\n');
/*
    If the last character of 's' is 'c', then this character is deleted
    from 's'.  Otherwise, 's' is not changed.
    Returns a reference to 's'.
    Assumes that 's' and 'c' are in Latin-1, not UTF-8.
*/


std::string &substitute(std::string &s,
			const std::string &target,
			const std::string &replacement);
/*
    Applies s/target/replacement/ to 's'.
    Assumes Latin-1, not UTF-8.
*/


std::string &quotechars(std::string &s,
			const char *metachars = "!\"#$&'()*+-./?@[\\]^`{|}~");
/*
    Inserts a backslash (\) before each character of 's' that is in
    'metachars'.
    For example, "foo$bar!" becomes "foo\$bar\!".
    Returns a reference to 's'.
    Assumes Latin-1, not UTF-8.
*/


std::string get_dir(const char *default_value, const char *env_var_name);
/*
    If the environment variable whose name is in 'env_var_name' is
    defined, its value is returned.
    Otherwise, a copy of 'default_value' is returned.
    In all cases, this function makes sure that the returned directory
    finishes with a '/'.
*/


void entry_set_text(GtkWidget *entry, const std::string &utf8_string);
/*
    Convenience function for gtk_entry_set_text().
*/


void error_dialog(GtkWidget *window,
			const std::string &utf8_message);
/*
    Displays a dialog (with a Close button only) that shows the given
    message and waits for the user to click the Close button.
    'window' will be used as the dialog's parent.
*/


void errno_dialog(GtkWidget *window,
			const std::string &utf8_first_line,
			int errnum);
/*
    Displays a dialog (with a Close button only) that shows the given
    string as the first line of the message, and the strerror(3) value
    for 'errnum' as the second line of the message, and waits for the user
    to click the Close button.
    'window' will be used as the dialog's parent.
    Typically, a copy of the global variable 'errno' is passed as the
    'errnum' argument.
*/


int show_url(const std::string &url,
		GtkWidget *window,
		const std::string &utf8_error_message);
/*
    Asks GNOME to show the contents of the given url in an appropriate
    viewer, using gnome_url_show().
    If an error occurs, 'utf8_error_message' will be used as the first
    line of the error dialog.  This function will add a colon (':')
    to the end of this line.
    The second line of the dialog will be the 'message' field of the
    GError structure filled by gnome_url_show().
    'window' will be used as the dialog's parent.
    Returns 0 on success, -1 on error.
*/


#endif  /* _H_util */
