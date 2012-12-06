/*  $Id: SagasuApp.cpp,v 1.23 2010/05/31 00:11:49 sarrazip Exp $
    SagasuApp.cpp - Class representing the main window

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

#include "SagasuApp.h"

#include <assert.h>
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include <libgnomeui/libgnomeui.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libgnome/gnome-config.h>
#include <libgnome/gnome-sound.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>  /* required by g++ 2.95.3 to define min() */

using namespace std;


///////////////////////////////////////////////////////////////////////////////


static const char *SEARCH_STRING_PATH = "Search/String";
static const char *FILE_PATTERNS_PATH = "Search/FilePatterns";
static const char *SEARCH_DIR_PATH = "Search/Directory";
static const char *EDITOR_COMMAND_PATH = "Commands/Editor";
static const char *MATCH_WHOLE_WORDS_PATH = "Options/MatchWord";
static const char *MATCH_CASE_PATH = "Options/MatchCase";
static const char *USE_PERL_REGEX_PATH = "Options/UsePerlRegex";
static const char *DIR_RECURSION_DEPTH_PATH = "Options/DirRecursionDepth";
static const char *EXCLUDE_CVS_DIRS_PATH = "Options/ExcludeCVSDirs";
static const char *EXCLUDE_SYMLINKED_DIRS_PATH = "Options/ExcludeSymlinkedDirs";

static const guint BORDER = 4;
static const gint SPACING = 4;

static const gint MAX_DIR_RECURSION_DEPTH = 20;


///////////////////////////////////////////////////////////////////////////////


/*static*/ SagasuApp *SagasuApp::instance = NULL;


///////////////////////////////////////////////////////////////////////////////


void
reaper(int)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
	;
}


#define WIDGET_AND_POINTER_CALLBACK(cb) \
	static void cb(GtkWidget *w = NULL, gpointer data = NULL) \
		{ SagasuApp::get_instance().cb(w, data); }
#include "callbacks.h"
#undef WIDGET_AND_POINTER_CALLBACK


/*static*/
gboolean
input_channel_ready_cb(GIOChannel * /*source*/,
			GIOCondition condition,
			gpointer data)
{
    return ((SagasuApp *) data)->input_channel_ready(condition);
}


gboolean
result_page_button_press_cb(GtkWidget * /*text_view*/,
			    GdkEventButton *e,
			    gpointer data)
{
    ResultPage *page = (ResultPage *) data;
    SagasuApp &app = SagasuApp::get_instance();

    if (e->button == 1 && e->type == GDK_2BUTTON_PRESS)
	app.launch_editor_for_position(
			page->get_text_pos_from_window_coords(e->x, e->y));

    return false;
}


bool
is_entry_empty(GtkWidget *entry)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    g_return_val_if_fail(text != NULL, true);
    return text[0] == '\0';
}


gboolean
key_press_in_result_search_entry_cb(GtkWidget *, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, true);
    return SagasuApp::get_instance().key_press_in_result_search_entry(event);
}


gboolean
key_press_in_search_field_cb(GtkWidget *, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, true);
    return SagasuApp::get_instance().key_press_in_search_field(event);
}


static
gboolean
onKeyPressInAbout(GtkWidget *about, GdkEventKey *event, gpointer)
{
    g_return_val_if_fail(event != NULL, TRUE);

    switch (event->keyval)
    {
	case GDK_Escape:
	    gtk_dialog_response(GTK_DIALOG(about), GTK_RESPONSE_OK);
	    return TRUE;

	default:
	    return FALSE;
    }
}


void
about_cb(GtkWidget * /*widget*/, gpointer /*data*/)
{
    static GtkWidget *dialog = NULL;

    if (dialog != NULL)
    {
	g_assert(GTK_WIDGET_REALIZED(dialog));
	gdk_window_show(dialog->window);
	gdk_window_raise(dialog->window);
    }
    else
    {
	const gchar *authors[] =
	{
	    "Pierre Sarrazin <http://sarrazip.com/>",
	    NULL
	};

	string logo_filename =
		get_dir(PIXMAPDIR, "PIXMAPDIR") + PACKAGE ".png";

	GdkPixbuf *logo = gdk_pixbuf_new_from_file(
					logo_filename.c_str(), NULL);
	string copyright = string(
	     "Copyright (C) 2002-2006 Pierre Sarrazin <http://sarrazip.com/>\n")
			+ _("Distributed under the GNU General Public License");

	GtkWidget *about = gtk_about_dialog_new();
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about), _("Sagasu"));
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), copyright.c_str());
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
			    _("GNOME tool to find strings in a set of files"));
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
	if (logo != NULL)
	    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), logo);

	{
	    ifstream licenseFile(
		    (get_dir(PKGDATADIR, "PKGDATADIR") + "COPYING").c_str());
	    if (licenseFile.good())
	    {
		stringstream ss;
		ss << licenseFile.rdbuf();
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about),
							ss.str().c_str());
	    }
	}

	if (logo != NULL)
	    gdk_pixbuf_unref(logo);

	g_signal_connect(G_OBJECT(about), "key-press-event",
			    G_CALLBACK(onKeyPressInAbout), NULL);

	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_hide(about);
	gtk_widget_destroy(about);
    }
}


static
GtkWidget *
label_new(const gchar *utf8_text, GtkWidget *mnemonic_widget)
{
    GtkWidget *label = gtk_label_new_with_mnemonic(utf8_text);
    if (mnemonic_widget != NULL)
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), mnemonic_widget);
    return label;
}


///////////////////////////////////////////////////////////////////////////////

#define GNOMEUIINFO_ITEM_ACCEL(label, tooltip, callback, ac_key, ac_mods) \
        { GNOME_APP_UI_ITEM, label, tooltip, (gpointer)callback, NULL, NULL, \
                GNOME_APP_PIXMAP_NONE, NULL, \
		(guint) (ac_key), (GdkModifierType) (ac_mods), NULL }


GnomeUIInfo file_menu[] =
{
    GNOMEUIINFO_MENU_EXIT_ITEM(::exit_cb, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo tabs_menu[] =
{
    GNOMEUIINFO_MENU_NEW_ITEM(
	N_("_New tab"),
	N_("Create a new tab in which to display search results"),
	::create_new_result_page_cb,
	NULL),
    GNOMEUIINFO_ITEM_NONE(
	N_("Go to _first tab"),
	N_("Go to the leftmost tab"),
	::first_tab_cb),
    GNOMEUIINFO_ITEM_ACCEL(
	N_("P_revious tab"),
	N_("Go to the tab at the left of the current one"),
	previous_tab_cb,
	GDK_r,
	GDK_CONTROL_MASK),
    GNOMEUIINFO_ITEM_ACCEL(
	N_("Nex_t tab"),
	N_("Go to the tab at the right of the current one"),
	next_tab_cb,
	GDK_t,
	GDK_CONTROL_MASK),
    GNOMEUIINFO_ITEM_NONE(
	N_("Go to _last tab"),
	N_("Go to the rightmost tab"),
	::last_tab_cb),
    GNOMEUIINFO_ITEM_ACCEL(
	N_("_Close tab"),
	N_("Close the current result page"),
	::close_current_tab_cb,
	GDK_w,
	GDK_CONTROL_MASK),
    GNOMEUIINFO_END
};

GnomeUIInfo help_menu[] =
{
    GNOMEUIINFO_HELP(PACKAGE),
    GNOMEUIINFO_ITEM_NONE(
	N_("_Say the word \"sagasu\""),
	N_("Play a sound file that pronounces the word \"sagasu\""),
	::say_sagasu_cb),
    GNOMEUIINFO_ITEM_NONE(
	N_("Go to Sagasu _Home Page"),
	N_("Open the Sagasu Home Page in a browser"),
	::home_page_cb),
    GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo menu[] =
{
    GNOMEUIINFO_MENU_FILE_TREE(file_menu),
    { GNOME_APP_UI_SUBTREE_STOCK, N_("_Tabs"), NULL, tabs_menu, NULL, NULL,
                (GnomeUIPixmapType) 0, NULL, 0, (GdkModifierType) 0, NULL },
    GNOMEUIINFO_MENU_HELP_TREE(help_menu),
    GNOMEUIINFO_END
};


GnomeUIInfo toolbar[] =
{
    GNOMEUIINFO_ITEM_STOCK(
	_("New tab"),
	_("Create a new tab in which to display search results"),
	::create_new_result_page_cb,
	GTK_STOCK_NEW),
    GNOMEUIINFO_ITEM_STOCK(
	_("First tab"),
	_("Go to the leftmost tab"),
	::first_tab_cb,
	GTK_STOCK_GOTO_FIRST),
    GNOMEUIINFO_ITEM_STOCK(
	_("Previous tab"),
	_("Go to the tab at the left of the current one"),
	previous_tab_cb,
	GTK_STOCK_GO_BACK),
    GNOMEUIINFO_ITEM_STOCK(
	_("Next tab"),
	_("Go to the tab at the right of the current one"),
	next_tab_cb,
	GTK_STOCK_GO_FORWARD),
    GNOMEUIINFO_ITEM_STOCK(
	_("Last tab"),
	_("Go to the rightmost tab"),
	::last_tab_cb,
	GTK_STOCK_GOTO_LAST),
    GNOMEUIINFO_ITEM_STOCK(
	_("New search"),
	_("Empty the search string field"),
	::erase_search_string_cb,
	GTK_STOCK_CLEAR),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(
	_("Quit"),
	_("Quit the program"),
	::exit_cb,
	GTK_STOCK_QUIT),
    GNOMEUIINFO_END
};


///////////////////////////////////////////////////////////////////////////////


/*static*/
void
SagasuApp::create_instance(const string &init_search_expr,
				const string &init_search_dir)
{
    instance = new SagasuApp(init_search_expr, init_search_dir);
}


/*static*/
SagasuApp &
SagasuApp::get_instance()
{
    return *instance;
}


SagasuApp::SagasuApp(const string &init_search_expr,
			const string &init_search_dir)
  : appwin(NULL),

    search_string_entry(NULL),
    file_patterns_entry(NULL),
    search_dir_entry(NULL),
    editor_cmd_entry(NULL),

    search_button(NULL),
    default_file_patterns_button(NULL),
    browse_search_dir_button(NULL),
    default_editor_cmd_button(NULL),

    match_whole_words_button(NULL),
    match_case_button(NULL),
    use_perl_regex_button(NULL),
    dir_recursion_depth_spin_button(NULL),
    exclude_cvs_dirs_button(NULL),
    exclude_symlinked_dirs_button(NULL),

    result_notebook(NULL),
    next_notebook_page_num(1),
    default_file_patterns("*.c *.cc *.C *.cpp *.cxx *.h"),
    default_editor_command("gnome-terminal -e \"vi +%n '%f'\""),
    input_channel(NULL),
    input_fd(-1),
    search_pid(0),
    result_page_of_current_search(NULL),
    pending_input(),
    num_matching_lines(0),
    num_matching_files(0),
    last_matching_filename(),
    status(NULL),

    result_search_entry(NULL),
    result_search_button(NULL),

    search_dir_file_sel_dlg(NULL),
    search_dir_file_sel_label(NULL)
{
    build_user_interface(init_search_expr, init_search_dir);

    /*  Set up a "reaper" callback which will be call when any child of
	this application terminates.  This avoids having <defunct> Perl
	interpreters.
    */
    signal(SIGCHLD, reaper);
}


SagasuApp::~SagasuApp()
{
}


string
SagasuApp::make_next_page_tab_label()
{
    char temp[128];
    snprintf(temp, sizeof(temp), "  %s%lu  ",
    		next_notebook_page_num < 10 ? "_" : "",
    		next_notebook_page_num);
    next_notebook_page_num++;
    return temp;
}


GtkWidget *
SagasuApp::make_next_page_tab(ResultPage *page)
/*
    Creates an hbox that contains a label and a button.
    The label shows a unique number that identifies that result page.
    The button shows a "close" icon.  When clicked, tab_close_clicked_cb()
    is called with 'page' as the data.
*/
{
    GtkWidget *label = gtk_label_new_with_mnemonic(
				make_next_page_tab_label().c_str());

    string xpm_filename = get_dir(PIXMAPDIR, "PIXMAPDIR") + "close.xpm";
    GtkWidget *image = gtk_image_new_from_file(xpm_filename.c_str());
    assert(image != NULL);
    GtkWidget *button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_container_add(GTK_CONTAINER(button), image);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(hbox));

    g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(::tab_close_clicked_cb), page);

    return hbox;
}


void
SagasuApp::show_working_state(bool busy)
{
    const char *utf8_label_text = (busy ? _("_Stop") : _("_Search"));

    gtk_button_set_label(GTK_BUTTON(search_button), utf8_label_text);

    gtk_widget_set_sensitive(
		GTK_WIDGET(default_file_patterns_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(browse_search_dir_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(default_editor_cmd_button), !busy);

    gtk_widget_set_sensitive(
		GTK_WIDGET(match_whole_words_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(match_case_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(use_perl_regex_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(dir_recursion_depth_spin_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(exclude_cvs_dirs_button), !busy);
    gtk_widget_set_sensitive(
		GTK_WIDGET(exclude_symlinked_dirs_button), !busy);

    if (busy)
    {
	display_wait_cursor();
	gnome_appbar_push(GNOME_APPBAR(status), _("Searching..."));
    }
    else
    {
	remove_wait_cursor();
	gnome_appbar_pop(GNOME_APPBAR(status));
    }
}


void
SagasuApp::display_wait_cursor()
{
    assert(appwin != NULL);

    if (appwin->window != NULL)
    {
	GdkCursor *cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(appwin->window, cursor);
	gdk_cursor_unref(cursor);
    }
}


void
SagasuApp::remove_wait_cursor()
{
    assert(appwin != NULL);
    if (appwin->window != NULL)
	gdk_window_set_cursor(appwin->window, NULL);
}


void
SagasuApp::build_user_interface(const std::string &init_search_expr,
				const std::string &init_search_dir)
{
    // Create the application:

    appwin = gnome_app_new(PACKAGE, _("Sagasu"));
    gtk_window_set_title(GTK_WINDOW(appwin), _("Sagasu"));

    g_signal_connect(appwin, "delete_event", G_CALLBACK(::exit_cb), NULL);

    gtk_window_set_resizable(GTK_WINDOW(appwin), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(appwin), 600, 500);
    gtk_window_set_wmclass(GTK_WINDOW(appwin), PACKAGE, PACKAGE);


    // Fill the main window:

    GtkWidget *vbox = gtk_vbox_new(FALSE, SPACING);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), BORDER);

    //////////////////////////////////////////////////////////////////////
    //
    // Search parameter fields
    //

    GtkWidget *parameters_table = gtk_table_new(4, 3, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(parameters_table), 2);
    gtk_table_set_col_spacings(GTK_TABLE(parameters_table), 2);

    search_string_entry = gtk_entry_new();
    file_patterns_entry = gtk_entry_new();
    search_dir_entry = gtk_entry_new();
    editor_cmd_entry = gtk_entry_new();

    struct
    {
	GtkWidget **button;
	GtkWidget *entry;
	const char *prompt_text;
	const char *button_text;
	void (*cb)(GtkWidget *, gpointer);
    }
    params[] =
    {
	{ &search_button, search_string_entry, _("Search strin_g:"),
				"", ::search_cb },
	{ &default_file_patterns_button, file_patterns_entry,
						_("File _patterns:"),
				_("_Defaults"), ::default_file_patterns_cb },
	{ &browse_search_dir_button, search_dir_entry, _("Se_arch directory:"),
				_("_Browse..."), ::browse_search_dir_cb },
	{ &default_editor_cmd_button, editor_cmd_entry, _("Edit_or command:"),
				_("D_efault"), ::default_editor_cmd_cb },
	{ NULL, NULL, NULL, NULL, NULL },
    };

    size_t i;
    for (i = 0; params[i].entry != NULL; i++)
    {
	GtkWidget *label = label_new(params[i].prompt_text, params[i].entry);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

	gtk_entry_set_max_length(GTK_ENTRY(params[i].entry), 1023);

	GtkWidget *button = gtk_button_new_with_mnemonic(params[i].button_text);

	assert(*params[i].button == NULL);
	*params[i].button = button;

	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(params[i].cb), NULL);

	gtk_table_attach(GTK_TABLE(parameters_table), label,
		0, 1, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(parameters_table), params[i].entry,
		1, 2, i, i + 1,
		    GtkAttachOptions(GTK_EXPAND | GTK_FILL), GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(parameters_table), button,
		2, 3, i, i + 1, GTK_FILL, GTK_SHRINK, 0, 0);

	g_signal_connect(G_OBJECT(params[i].entry), "key-press-event",
			G_CALLBACK(key_press_in_search_field_cb), NULL);
    }


    //////////////////////////////////////////////////////////////////////
    //
    // Search options check buttons
    //

    GtkWidget *options_table = gtk_table_new(2, 3, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(options_table), SPACING * 4);

    struct
    {
	GtkWidget **check_button;
	const char *button_text;
    }
    buttons[] =
    {
	{ &match_whole_words_button, _("Match _whole words") },
	{ &match_case_button, _("Match _case") },
	{ &use_perl_regex_button, _("Use Perl rege_x") },
	{ NULL, "" },
	{ &exclude_cvs_dirs_button, _("Exclude C_VS dirs") },
	{ &exclude_symlinked_dirs_button, _("Exclude sym_linked dirs") },
	{ NULL, NULL }
    };

    for (i = 0; buttons[i].button_text != NULL; i++)
    {
	GtkWidget *w;
	if (buttons[i].check_button == NULL)
	{
	    /*
		Exception: in this slot, we now put a spin button that
		allows the user to select the depth of directory recursion,
		instead of just allowing or disallowing recursion.
	    */
	    GtkAdjustment *adj = (GtkAdjustment *) gtk_adjustment_new(
			MAX_DIR_RECURSION_DEPTH, 0.0, MAX_DIR_RECURSION_DEPTH,
			1.0, 5.0, 0.0);
	    dir_recursion_depth_spin_button = gtk_spin_button_new(adj, 0, 0);
	    gtk_spin_button_set_numeric(
			GTK_SPIN_BUTTON(dir_recursion_depth_spin_button), true);

	    GtkWidget *label = label_new(_("Dir _recursion depth:"),
					dir_recursion_depth_spin_button);
	    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	    w = gtk_hbox_new(FALSE, 2);
	    gtk_box_pack_start(GTK_BOX(w), label, FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(w),
			dir_recursion_depth_spin_button, FALSE, FALSE, 0);
	}
	else
	{
	    w = gtk_check_button_new_with_mnemonic(
					    buttons[i].button_text);
	    assert(*buttons[i].check_button == NULL);
	    *buttons[i].check_button = w;
	}

	gtk_table_attach(GTK_TABLE(options_table), w,
			i % 3, i % 3 + 1, i / 3, i / 3 + 1,
			GTK_FILL, GTK_SHRINK, 0, 0);
    }


    //////////////////////////////////////////////////////////////////////
    //
    // Notebook containing result pages
    //

    result_notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(result_notebook), TRUE);
    create_new_result_page_cb();


    //////////////////////////////////////////////////////////////////////
    //
    // Entry and button to search inside a result page
    //

    GtkWidget *result_search_box = gtk_hbox_new(FALSE, SPACING);
    result_search_entry = gtk_entry_new();
    GtkWidget *result_search_prompt =
			label_new(_("Find in res_ults:"), result_search_entry);
    result_search_button = gtk_button_new_with_mnemonic(_("F_ind"));

    gtk_box_pack_start(GTK_BOX(result_search_box),
			result_search_prompt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(result_search_box),
			result_search_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(result_search_box),
			result_search_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(result_search_entry), "changed",
		    G_CALLBACK(::result_search_entry_changed_cb), this);
    g_signal_connect(G_OBJECT(result_search_entry), "key-press-event",
		    G_CALLBACK(::key_press_in_result_search_entry_cb), NULL);
    g_signal_connect(G_OBJECT(result_search_button), "clicked",
		    G_CALLBACK(::result_search_button_clicked_cb), NULL);

    result_search_entry_changed_cb();


    //////////////////////////////////////////////////////////////////////

    gtk_box_pack_start(GTK_BOX(vbox), parameters_table, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), options_table, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), result_notebook, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), result_search_box, FALSE, FALSE, 0);

    gnome_app_set_contents(GNOME_APP(appwin), vbox);


    status = gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_NEVER);
    gnome_app_set_statusbar(GNOME_APP(appwin), status);

    // This call needs to appear twice, for some reason:
    gnome_appbar_push(GNOME_APPBAR(status), _("Ready."));
    gnome_appbar_push(GNOME_APPBAR(status), _("Ready."));


    menu[1].label = gettext(menu[1].label);  // PATCH

    gnome_app_create_menus_with_data(GNOME_APP(appwin), menu, appwin);
    gnome_app_create_toolbar_with_data(GNOME_APP(appwin), toolbar, appwin);
    gnome_app_install_menu_hints(GNOME_APP(appwin), menu);


    load_configuration();

    // TODO: do not convert the two parameters if command line is in UTF-8.
    if (!init_search_expr.empty())
	entry_set_text(search_string_entry, u8_string(init_search_expr));
    if (!init_search_dir.empty())
	entry_set_text(search_dir_entry, u8_string(init_search_dir));

    entry_changed_cb();


    // These signals must be connected after the initialization of the entries:
    g_signal_connect(G_OBJECT(search_string_entry), "changed",
			G_CALLBACK(::entry_changed_cb), NULL);
    g_signal_connect(G_OBJECT(file_patterns_entry), "changed",
			G_CALLBACK(::entry_changed_cb), NULL);
    g_signal_connect(G_OBJECT(search_dir_entry), "changed",
			G_CALLBACK(::entry_changed_cb), NULL);


    show_working_state(false);

    gtk_widget_show_all(appwin);
}


ResultPage *
SagasuApp::get_current_result_page() const
{
    gint page_num = gtk_notebook_get_current_page(
				GTK_NOTEBOOK(result_notebook));
    g_return_val_if_fail(page_num >= 0, NULL);

    GtkWidget *child_widget = gtk_notebook_get_nth_page(
				GTK_NOTEBOOK(result_notebook), page_num);
    g_return_val_if_fail(child_widget != NULL, NULL);

    ResultPage *page = (ResultPage *)
			g_object_get_data(G_OBJECT(child_widget), "ResultPage");
    g_return_val_if_fail(page != NULL, NULL);

    return page;
}


void
SagasuApp::search_cb(GtkWidget *, gpointer)
{
    SagasuApp::get_instance().save_configuration();

    string target_expr = latin1_string(gtk_entry_get_text(
					GTK_ENTRY(search_string_entry)));
    if (target_expr.empty())
	return;

    string raw_target_expr = target_expr;

    string search_dir = latin1_string(gtk_entry_get_text(
					GTK_ENTRY(search_dir_entry)));
    if (search_dir.empty())
	return;
    if (search_dir != "/")
	chomp(search_dir, '/');

    ResultPage *cur_page = get_current_result_page();
    g_return_if_fail(cur_page != NULL);

    cur_page->set_next_search_pos(0);


    if (input_channel != NULL)  // if a search is already in progress
    {
	cur_page->insert_text("\n\n" "*** "
		    + string(_("Interrupted")) + " ***" "\n");

	close_input();
	return;
    }

    cur_page->clear();


    if (use_regex())
	quotechars(target_expr, "#/@");
    else
	quotechars(target_expr);

    if (match_whole_words())
	target_expr = "\\b(" + target_expr + ")\\b";

    input_channel = NULL;
    create_search_process(
			target_expr,
			raw_target_expr,
			search_dir,
			get_file_patterns(),
			match_case(),
			dir_recursion_depth(),
			exclude_cvs_dirs(),
			exclude_symlinked_dirs());
    if (input_fd == -1)
	return;

    input_channel = g_io_channel_unix_new(input_fd);
    g_return_if_fail(input_channel != NULL);
    if (g_io_channel_set_encoding(input_channel, NULL, NULL)
				    != G_IO_STATUS_NORMAL)
    {
	g_return_if_reached();
    }

    (void) g_io_add_watch(input_channel,
		GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL),
		input_channel_ready_cb, this);

    show_working_state(true);
}


std::string
SagasuApp::get_file_patterns() const
{
    return latin1_string(gtk_entry_get_text(
				    GTK_ENTRY(file_patterns_entry)));
}


bool
SagasuApp::match_whole_words() const
{
    return gtk_toggle_button_get_active(
		    GTK_TOGGLE_BUTTON(match_whole_words_button));
}


bool
SagasuApp::match_case() const
{
    return gtk_toggle_button_get_active(
		    GTK_TOGGLE_BUTTON(match_case_button));
}


bool
SagasuApp::use_regex() const
{
    return gtk_toggle_button_get_active(
		    GTK_TOGGLE_BUTTON(use_perl_regex_button));
}


gint
SagasuApp::dir_recursion_depth() const
{
    return gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(dir_recursion_depth_spin_button));
}


bool
SagasuApp::exclude_cvs_dirs() const
{
    return gtk_toggle_button_get_active(
		    GTK_TOGGLE_BUTTON(exclude_cvs_dirs_button));
}


bool
SagasuApp::exclude_symlinked_dirs() const
{
    return gtk_toggle_button_get_active(
		    GTK_TOGGLE_BUTTON(exclude_symlinked_dirs_button));
}


void
SagasuApp::create_search_process(const string &target_expr,
				const string &raw_target_expr,
				const string &search_dir,
				const string &file_patterns,
				bool match_case,
				gint recursion_depth,
				bool exclude_cvs_dirs,
				bool exclude_symlinked_dirs)
/*
    Creates a process that will search for the regular expression
    given in 'target_expr' and returns the file descriptor that can
    be read to obtain the results written by this process.

    'target_expr' must not be empty.

    Sets members 'input_fd' and 'search_pid'.  On error, these members
    become -1 and 0 respectively.
*/
{
    assert(result_page_of_current_search == NULL);
    assert(num_matching_lines == 0);
    assert(num_matching_files == 0);
    assert(!target_expr.empty());

    errno = 0;
    input_fd = -1;
    search_pid = 0;

    /*
	Check that the search directory is accessible.
    */
    if (access(search_dir.c_str(), R_OK | X_OK) != 0)
    {
	int e = errno;
	errno_dialog(appwin, _("Search directory is not accessible"), e);
	return;
    }


    /*
	Create a pipe and receive the reading and writing file descriptors
	in pipefds[0] and pipefds[1] respectively.
    */
    int pipefds[2];
    if (pipe(pipefds) != 0)
    {
	int e = errno;
	errno_dialog(appwin, _("Error when creating a pipe"), e);
	return;
    }

    /*
	Duplicate the current process.
	The child's stdout and stderr are redirected to the pipe,
	and the pipe's reading file description is returned.
	The child executes a Perl script that does the actual searching.
    */
    pid_t pid = fork();
    if (pid == -1)
    {
	int e = errno;
	errno_dialog(appwin, _("Error when creating the search process"), e);
	close(pipefds[0]);
	close(pipefds[1]);
	return;
    }

    if (pid == 0)  // if this is the child
    {
	close(pipefds[0]);  // child does not read from this GNOME app

	// redirect child's stdout and stderr to the writing fd of the pipe:
	if (dup2(pipefds[1], STDOUT_FILENO) == -1)
	{
	    int e = errno;
	    cerr << "dup2 failed for stdout: " << strerror(e) << endl;
	    _exit(127);
	}
	if (dup2(pipefds[1], STDERR_FILENO) == -1)
	{
	    int e = errno;
	    cerr << "dup2 failed for stderr: " << strerror(e) << endl;
	    _exit(127);
	}
	close(pipefds[1]);  // not needed anymore

	string script = get_dir(PKGDATADIR, "PKGDATADIR") + "sagasu-helper.pl";

	char depth[128];
	snprintf(depth, sizeof(depth), "%d", (int) recursion_depth);

	execl("/usr/bin/perl", "perl", script.c_str(),
		target_expr.c_str(),
		search_dir.c_str(),
		file_patterns.c_str(),
		match_case ? "1" : "0",
		exclude_cvs_dirs ? "1" : "0",
		exclude_symlinked_dirs ? "1" : "0",
		depth,
		NULL);

	cerr << "exec() returned: " << strerror(errno) << endl;
	exit(EXIT_FAILURE);
    }


    /*
	This is the parent.
	Close the writing end of the pipe.
	Remember in which page the results are to be written.
	Start that page with the search string.
    */
    close(pipefds[1]);
    result_page_of_current_search = get_current_result_page();
    result_page_of_current_search->insert_text(
		    _("Search string:") + string(" ") + raw_target_expr + "\n");
    pending_input.erase();

    last_matching_filename.erase();
    num_matching_lines = 0;
    num_matching_files = 0;
    show_num_matching_lines();

    input_fd = pipefds[0];
    search_pid = pid;
}


void
SagasuApp::close_input()
/*
    If there is an active input channel, shuts it down, closes the pipe's
    file descriptor, kills the search process with SIGTERM, sets input_fd
    to -1 and search_pid to 0.

*/
{
    if (input_channel != NULL)
    {
	g_io_channel_shutdown(input_channel, FALSE, NULL);
	g_io_channel_unref(input_channel);
	input_channel = NULL;

	close(input_fd);
	input_fd = -1;

	kill(search_pid, SIGTERM);
	search_pid = 0;
    }
    else
    {
	assert(input_fd == -1);
	assert(search_pid == 0);
    }

    show_num_matching_lines();


    result_page_of_current_search = NULL;
    num_matching_lines = 0;
    num_matching_files = 0;

    show_working_state(false);
}


void
SagasuApp::show_num_matching_lines()
{
    if (result_page_of_current_search == NULL)
	return;
    char status[2048];
    snprintf(status, sizeof(status),
	    "%s: %u; %s: %u %s",
		_("Matching lines"), (unsigned) num_matching_lines,
		_("matching files"), (unsigned) num_matching_files,
		(input_channel == NULL ? "" : _("(still searching)"))
		);
    result_page_of_current_search->set_status_label(status);
}


gboolean
SagasuApp::input_channel_ready(GIOCondition /*condition*/)
{
    if (input_fd == -1)
    {
	assert(input_channel == NULL);
	return FALSE;  // event source must be removed
    }

    char buffer[1024];
    ssize_t bytes_read = read(input_fd, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
	int e = errno;
	if (bytes_read < 0)
	{
	    g_warning("read() failed on source fd %d: %s", input_fd, strerror(e));
	}

	close_input();
	return FALSE;  // event source must be removed
    }

    pending_input.append(buffer, bytes_read);
    process_pending_input();
    show_num_matching_lines();
    return TRUE;
}


void
SagasuApp::process_pending_input()
/*
    Pass all newline terminated strings in 'pendingInput'.
    Count those that do not start with the error prefix as result lines.
    The passed lines are inserted in the current result buffer
    and then are removed from 'pendingInput'.

    We expect 'pendingInput' to sometimes end with characters that are
    not followed by a newline.  This should mean that more input is
    expected from the pipe.

    In addition, filenames are extracted from result lines and distinct
    filenames are counted as matching files.  This method assumes that
    all occurrences of a filename will be consecutive.
*/
{
    string::size_type pos_newline, search_pos;
    for (search_pos = 0;
	(pos_newline = pending_input.find('\n', search_pos)) != string::npos;
	search_pos = pos_newline + 1)
    {
	if (memcmp(pending_input.data() + search_pos, "*** ", 4) != 0)
	{
	    num_matching_lines++;

	    string::size_type pos_colon = pending_input.find(':', search_pos);
	    if (pos_colon < pos_newline)
	    {
		string filename(pending_input,
				search_pos, pos_colon - search_pos);
		if (filename != last_matching_filename)
		{
		    num_matching_files++;
		    last_matching_filename = filename;
		}
	    }
	}

    }

    if (search_pos > 0)
    {
	string inserted(pending_input, 0, search_pos);
	result_page_of_current_search->insert_text(inserted);
	pending_input.erase(0, search_pos);
    }
}


string
SagasuApp::get_result_text_line_for_pos(gint pos_in_chars) const
/*
    Returns a Latin-1 string.
*/
{
    ResultPage *page = get_current_result_page();
    GtkWidget *view = page->get_text_view();
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    // Get an iterator on the desired position:
    GtkTextIter it;
    gtk_text_buffer_get_iter_at_offset(buf, &it, pos_in_chars);

    // Get the number (>=0) of the line that contains the desired position:
    gint line_num = gtk_text_iter_get_line(&it);
    if (line_num == 0)  // first line shows search string; it is not a result
	return "";

    // Get an iterator on the start of that line:
    GtkTextIter line_start_it;
    gtk_text_buffer_get_iter_at_line_offset(buf, &line_start_it, line_num, 0);

    // Get an iterator on the end of that line, using the length:
    gint line_length = gtk_text_iter_get_chars_in_line(&it);
    GtkTextIter line_end_it = line_start_it;
    gtk_text_iter_forward_chars(&line_end_it, line_length);

    // Get the line itself, and convert it from UTF-8 to Latin-1:
    GCharPtr utf8_line(gtk_text_buffer_get_text(
				buf, &line_start_it, &line_end_it, true));
    return latin1_string(utf8_line.get());
}


int
get_filename_and_line_num_from_line(
				const string &line,
				string &filename,
				unsigned long &line_num)
/*
    'line' must be in Latin-1.
*/
{
    string::size_type pos_colon1 = line.find(':');
    if (pos_colon1 == string::npos || pos_colon1 == 0)
	return -1;
    string::size_type pos_colon2 = line.find(':', pos_colon1 + 1);
    if (pos_colon2 == string::npos || pos_colon2 == line.length() - 1)
	return -2;

    // Check that all characters between the two colons are decimal digits:
    if (pos_colon1 + 1 == pos_colon2)
	return -3;
    for (string::size_type i = pos_colon1 + 1; i < pos_colon2; i++)
	if (!isdigit(line[i]))
	    return -4;
    errno = 0;
    line_num = strtoul(line.c_str() + pos_colon1 + 1, NULL, 10);
    if (line_num == ULONG_MAX || errno == ERANGE)
	return -5;
    filename = string(line, 0, pos_colon1);
    return 0;
}


void
SagasuApp::launch_editor_for_position(gint pos_in_chars)
{
    string line = get_result_text_line_for_pos(pos_in_chars);

    string filename;
    unsigned long line_num = 0;
    if (get_filename_and_line_num_from_line(line, filename, line_num) != 0)
	return;


    const gchar *utf8_cmd = gtk_entry_get_text(GTK_ENTRY(editor_cmd_entry));
    string cmd = latin1_string(utf8_cmd);
    char ln[128];
    snprintf(ln, sizeof(ln), "%lu", line_num);
    substitute(cmd, "%n", ln);
    substitute(cmd, "%f", filename);


    // Start a child process to run system(), which is blocking:
    pid_t pid = fork();
    if (pid == -1)
    {
	int e = errno;
	errno_dialog(appwin, _("Error when creating the editor process"), e);
	return;
    }
    if (pid == 0)  // if in child
    {
	system(cmd.c_str());

	/*
	    We must exit the child with _exit() instead of exit()
	    to avoid crashing with the following error message
	    when the user forcibly kills the terminal program
	    (e.g., xterm) started by system():

	    Gdk-ERROR **: X connection to :0.0 broken
	    (explicit kill or server shutdown).
	*/
	_exit(EXIT_SUCCESS);
    }

    // In parent: nothing
}


string
get_config_var_path(const string &var)
{
    return string("/") + PACKAGE + "/" + var;
}


string
get_config_var(const string &var, const string &utf8_default)
/*
    gnome_config_sync() must be called afterwards to save the changes
    in the configuration file.
    Parameters must be Latin-1 strings.
    Returns a UTF-8 string.
*/
{
    string path = get_config_var_path(var);
    GCharPtr s(gnome_config_get_string(path.c_str()));
    if (s.get() != NULL)
	return s.get();
    gnome_config_set_string(path.c_str(), utf8_default.c_str());
    return utf8_default;
}


bool
get_bool_config_var(const string &var, bool deFault)
{
    return get_config_var(var, deFault ? "1" : "0") == "1";
}


void
set_config_var(const string &var, const string &utf8_value)
/*
    gnome_config_sync() must be called afterwards to save the changes
    in the configuration file.
    Parameters must be Latin-1 strings.
*/
{
    gnome_config_set_string(
		get_config_var_path(var).c_str(), utf8_value.c_str());
}


void
SagasuApp::load_configuration()
{
    entry_set_text(search_string_entry,
	    get_config_var(SEARCH_STRING_PATH, ""));
    entry_set_text(file_patterns_entry,
	    get_config_var(FILE_PATTERNS_PATH, default_file_patterns));
    const char *home = getenv("HOME");
    if (home == NULL)
	home = "/";
    entry_set_text(search_dir_entry,
		get_config_var(SEARCH_DIR_PATH, u8_string(home).c_str()));
    entry_set_text(editor_cmd_entry,
		get_config_var(EDITOR_COMMAND_PATH, default_editor_command));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(match_whole_words_button),
		get_bool_config_var(MATCH_WHOLE_WORDS_PATH, false));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(match_case_button),
		get_bool_config_var(MATCH_CASE_PATH, true));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_perl_regex_button),
		get_bool_config_var(USE_PERL_REGEX_PATH, false));

    string d = get_config_var(DIR_RECURSION_DEPTH_PATH, "-1");
    long depth = strtol(d.c_str(), NULL, 10);
    if (depth < 0 || depth > MAX_DIR_RECURSION_DEPTH)
	depth = MAX_DIR_RECURSION_DEPTH;
    gtk_spin_button_set_value(
		GTK_SPIN_BUTTON(dir_recursion_depth_spin_button),
		(gint) depth);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(exclude_cvs_dirs_button),
		get_bool_config_var(EXCLUDE_CVS_DIRS_PATH, true));
    gtk_toggle_button_set_active(
			    GTK_TOGGLE_BUTTON(exclude_symlinked_dirs_button),
		get_bool_config_var(EXCLUDE_SYMLINKED_DIRS_PATH, true));

    gnome_config_sync();
}


void
SagasuApp::save_configuration()
{
    set_config_var(SEARCH_STRING_PATH,
			gtk_entry_get_text(GTK_ENTRY(search_string_entry)));
    set_config_var(FILE_PATTERNS_PATH,
			gtk_entry_get_text(GTK_ENTRY(file_patterns_entry)));
    set_config_var(SEARCH_DIR_PATH,
			gtk_entry_get_text(GTK_ENTRY(search_dir_entry)));
    set_config_var(EDITOR_COMMAND_PATH,
			gtk_entry_get_text(GTK_ENTRY(editor_cmd_entry)));

    #define GET_ACTIVE(b) \
	    (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)) ? "1" : "0")

    char depth[128];
    snprintf(depth, sizeof(depth), "%d", (int) dir_recursion_depth());

    set_config_var(MATCH_WHOLE_WORDS_PATH,
		GET_ACTIVE(match_whole_words_button));
    set_config_var(MATCH_CASE_PATH,
		GET_ACTIVE(match_case_button));
    set_config_var(USE_PERL_REGEX_PATH,
		GET_ACTIVE(use_perl_regex_button));
    set_config_var(DIR_RECURSION_DEPTH_PATH,
		depth);
    set_config_var(EXCLUDE_CVS_DIRS_PATH,
		GET_ACTIVE(exclude_cvs_dirs_button));
    set_config_var(EXCLUDE_SYMLINKED_DIRS_PATH,
		GET_ACTIVE(exclude_symlinked_dirs_button));

    #undef GET_ACTIVE

    gnome_config_sync();
}


void
SagasuApp::entry_changed_cb(GtkWidget *, gpointer)
{
    bool ss_empty = is_entry_empty(search_string_entry);
    bool fp_empty = is_entry_empty(file_patterns_entry);
    bool sd_empty = is_entry_empty(search_dir_entry);
    gtk_widget_set_sensitive(search_button,
				!ss_empty && !fp_empty && !sd_empty);
}


void
SagasuApp::result_search_entry_changed_cb(GtkWidget *, gpointer)
{
    gtk_widget_set_sensitive(result_search_button,
					!is_entry_empty(result_search_entry));
}


const gchar *
utf8_stristr(const gchar *haystack, const gchar *needle)
{
    const gunichar needle_start = g_utf8_get_char(needle);
    const gunichar needle_start_upper = g_unichar_toupper(needle_start);
    const gunichar needle_start_lower = g_unichar_tolower(needle_start);

    for (;;)
    {
	// If we are looking for "pizza", then look for a 'P' and for 'p':

	const gchar *found_upper = g_utf8_strchr(
					haystack, -1, needle_start_upper);
	const gchar *found_lower = g_utf8_strchr(
					haystack, -1, needle_start_lower);

	// If both searches failed, declare a failure:
	if (found_upper == NULL && found_lower == NULL)
	    return NULL;

	// If one of the two searches succeeded, point to the position found;
	// if the two searches succeeded, point to the leftmost position:

	const gchar *found;
	if (found_upper != NULL && found_lower == NULL)
	    found = found_upper;
	else if (found_upper == NULL && found_lower != NULL)
	    found = found_lower;
	else
	    found = min(found_upper, found_lower);

	// Try to match each character of 'needle' with the characters
	// that start at 'found':

	const gchar *f = found;

	const gchar *n;
	for (n = needle; *n != 0; n = g_utf8_find_next_char(n, NULL))
	{
	    gunichar needle_char = g_utf8_get_char(n);
	    gunichar needle_char_lower = g_unichar_tolower(needle_char);

	    gunichar found_char = g_utf8_get_char(f);
	    if (found_char == 0)  // if no more text in which to search:
		break;  // failure

	    gunichar found_char_lower = g_unichar_tolower(found_char);

	    if (needle_char_lower != found_char_lower)
		break;  // failure

	    f = g_utf8_find_next_char(f, NULL);
	}

	if (*n == 0)  // if preceding loop succeeded
	    return found;

	haystack = g_utf8_find_next_char(found, NULL);
    }
}


void
SagasuApp::result_search_button_clicked_cb(GtkWidget *, gpointer)
{
    const gchar *target = gtk_entry_get_text(GTK_ENTRY(result_search_entry));
    g_return_if_fail(target != NULL);
    if (target[0] == '\0')
	return;


    display_wait_cursor();


    // This call must precede the call to select_region().
    gtk_editable_select_region(GTK_EDITABLE(result_search_entry), 0, -1);


    ResultPage *page = get_current_result_page();
    string contents = page->get_all_text();
    gint search_pos = page->get_next_search_pos();
    gint content_length = page->get_char_count();
    if (search_pos >= content_length)
	search_pos = content_length;

    const gchar *contents_ptr = contents.c_str();
    const gchar *srch = g_utf8_offset_to_pointer(contents_ptr, search_pos);
    const gchar *found = utf8_stristr(srch, target);

    if (found == NULL)
    {
	gnome_appbar_set_status(GNOME_APPBAR(status), _("Not found"));

	page->set_next_search_pos(0);
    }
    else
    {
	glong found_length = g_utf8_strlen(found, -1);
	glong found_pos = content_length - found_length;

	unsigned pct = (unsigned)
			floor(100.0 * found_pos / content_length + 0.5);
	char p[128];
	snprintf(p, sizeof(p), "%u", pct);
	string t = _("Found at:") + string(" ") + p + "%";

	gnome_appbar_set_status(GNOME_APPBAR(status), t.c_str());

	// Select found string:
	gint found_end = gint(found_pos + g_utf8_strlen(target, -1));
	page->select_text_region(gint(found_pos), found_end);

	// Set the position for the next search:
	page->set_next_search_pos(found_end);
    }

    remove_wait_cursor();
}


void
SagasuApp::default_file_patterns_cb(GtkWidget *, gpointer)
{
    gtk_entry_set_text(GTK_ENTRY(file_patterns_entry),
		latin1_string(default_file_patterns.c_str()).c_str());
}


void
SagasuApp::default_editor_cmd_cb(GtkWidget *, gpointer)
{
    gtk_entry_set_text(GTK_ENTRY(editor_cmd_entry),
		latin1_string(default_editor_command).c_str());
}


void
search_dir_file_sel_ok(GtkWidget *, gpointer)
{
    SagasuApp::get_instance().accept_search_dir();
}


void
search_dir_file_sel_cancel(GtkWidget *, gpointer)
{
    SagasuApp::get_instance().close_search_dir_file_sel();
}


gboolean
search_dir_file_sel_destroy(GtkWidget *, GdkEvent *, gpointer)
{
    SagasuApp::get_instance().close_search_dir_file_sel();
    return true;
}


void
SagasuApp::browse_search_dir_cb(GtkWidget *, gpointer)
{
    if (search_dir_file_sel_dlg != NULL)
	return;

    assert(search_dir_file_sel_label == NULL);

    search_dir_file_sel_dlg = gtk_file_selection_new(
				    _("Select search directory"));
    gtk_window_set_transient_for(
		    GTK_WINDOW(search_dir_file_sel_dlg),
		    GTK_WINDOW(appwin));

    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(
			    search_dir_file_sel_dlg)->ok_button),
		    "clicked",
		    G_CALLBACK(search_dir_file_sel_ok),
		    NULL);

    g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(
			    search_dir_file_sel_dlg)->cancel_button),
		    "clicked",
		    G_CALLBACK(search_dir_file_sel_cancel),
		    NULL);

    g_signal_connect(G_OBJECT(search_dir_file_sel_dlg),
		    "destroy",
		    G_CALLBACK(search_dir_file_sel_destroy),
		    NULL);

    search_dir_file_sel_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(
				    search_dir_file_sel_dlg)->action_area),
			    search_dir_file_sel_label,
			    FALSE, FALSE, 0);

    const char *utf8_dir = gtk_entry_get_text(GTK_ENTRY(search_dir_entry));
    string latin1_dir = latin1_string(utf8_dir);
    struct stat statbuf;
    if (stat(latin1_dir.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
    {
	if (latin1_dir[latin1_dir.length() - 1] != '/')
	    latin1_dir += '/';
	gtk_file_selection_set_filename(
			    GTK_FILE_SELECTION(search_dir_file_sel_dlg),
			    latin1_dir.c_str());
    }

    gtk_widget_show(search_dir_file_sel_dlg);
    gtk_widget_show(search_dir_file_sel_label);

    gtk_widget_set_sensitive(browse_search_dir_button, false);
}


void
SagasuApp::close_search_dir_file_sel()
{
    assert(search_dir_file_sel_dlg != NULL);
    assert(search_dir_file_sel_label != NULL);

    gtk_widget_hide(search_dir_file_sel_dlg);
    gtk_widget_destroy(search_dir_file_sel_dlg);

    search_dir_file_sel_dlg = NULL;
    search_dir_file_sel_label = NULL;

    gtk_widget_set_sensitive(browse_search_dir_button, true);
}


void
SagasuApp::accept_search_dir()
{
    const gchar *filename = gtk_file_selection_get_filename(
				GTK_FILE_SELECTION(search_dir_file_sel_dlg));
    struct stat statbuf;
    if (stat(filename, &statbuf) != 0)
    {
	gtk_label_set_text(GTK_LABEL(search_dir_file_sel_label),
				_("Directory not found."));
	gdk_beep();
	return;
    }
    if (!S_ISDIR(statbuf.st_mode))
    {
	gtk_label_set_text(GTK_LABEL(search_dir_file_sel_label),
				_("Not a directory."));
	gdk_beep();
	return;
    }
    gtk_entry_set_text(GTK_ENTRY(search_dir_entry),
			u8_string(filename).c_str());
    close_search_dir_file_sel();
}


gboolean
SagasuApp::key_press_in_search_field(GdkEventKey *event)
{
    switch (event->keyval)
    {
	case GDK_Return:
	case GDK_KP_Enter:
	    search_cb();
	    return true;

	default:
	    return false;
    }
}


gboolean
SagasuApp::key_press_in_result_search_entry(GdkEventKey *event)
{
    switch (event->keyval)
    {
	case GDK_Return:
	case GDK_KP_Enter:
	    result_search_button_clicked_cb();
	    return true;

	default:
	    return false;
    }
}


void
SagasuApp::home_page_cb(GtkWidget *, gpointer)
{
    show_url(PACKAGE_HOME_PAGE, appwin,
		_("Error when opening Sagasu Home Page"));
}


///////////////////////////////////////////////////////////////////////////////
//
// CALLBACKS METHODS
//

void
SagasuApp::create_new_result_page_cb(GtkWidget *, gpointer)
{
    ResultPage *page = new ResultPage();
    GtkWidget *tab = make_next_page_tab(page);
    gtk_notebook_append_page(GTK_NOTEBOOK(result_notebook),
				page->get_top_widget(), tab);
    gint page_num = gtk_notebook_page_num(GTK_NOTEBOOK(result_notebook),
				page->get_top_widget());
    gtk_notebook_set_current_page(GTK_NOTEBOOK(result_notebook), page_num);

    g_signal_connect(G_OBJECT(page->get_text_view()), "button-press-event",
			G_CALLBACK(result_page_button_press_cb), page);
}


void
SagasuApp::exit_cb(GtkWidget *, gpointer)
{
    close_input();  // terminate any searching subprocess if applicable
    save_configuration();
    gtk_main_quit();
}


void
SagasuApp::first_tab_cb(GtkWidget *, gpointer)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(result_notebook), 0);
}


void
SagasuApp::last_tab_cb(GtkWidget *, gpointer)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(result_notebook), -1);
}


void
SagasuApp::previous_tab_cb(GtkWidget *, gpointer)
{
    gtk_notebook_prev_page(GTK_NOTEBOOK(result_notebook));
}


void
SagasuApp::next_tab_cb(GtkWidget *, gpointer)
{
    gtk_notebook_next_page(GTK_NOTEBOOK(result_notebook));
}


void
SagasuApp::erase_search_string_cb(GtkWidget *, gpointer)
{
    entry_set_text(search_string_entry, "");
}


void
SagasuApp::tab_close_clicked_cb(GtkWidget *, gpointer data)
{
    ResultPage *page = (ResultPage *) data;
    assert(page != NULL);

    // If trying to close tab of active search, refuse to close it.
    if (page == result_page_of_current_search)
	return;

    GtkWidget *tw = page->get_top_widget();

    gint page_num = gtk_notebook_page_num(GTK_NOTEBOOK(result_notebook), tw);
    assert(page_num >= 0);

    // If only one tab open, refuse to close it.
    if (gtk_notebook_get_nth_page(GTK_NOTEBOOK(result_notebook), 1) == NULL)
	return;

    gtk_notebook_remove_page(GTK_NOTEBOOK(result_notebook), page_num);
    delete page;

    // If only one tab left, make it number 1:
    if (gtk_notebook_get_nth_page(GTK_NOTEBOOK(result_notebook), 1) == NULL)
    {
	// Get the page widget:
	GtkWidget *child = gtk_notebook_get_nth_page(
				    GTK_NOTEBOOK(result_notebook), 0);

	// Get the tab widget of that page:
	GtkWidget *hbox = gtk_notebook_get_tab_label(
				    GTK_NOTEBOOK(result_notebook), child);
	assert(hbox != NULL);

	// Get the label inside that tab widget (it's the 1st child widget):
	GList *hbox_children =
			gtk_container_get_children(GTK_CONTAINER(hbox));
	gpointer first_child = g_list_nth_data(hbox_children, 0);
	assert(first_child != NULL);
	assert(GTK_IS_LABEL(first_child));

	// Reset the global page number counter:
	next_notebook_page_num = 1;

	// Set the text of the label widget to "1":
	string new_label = make_next_page_tab_label();
	gtk_label_set_text_with_mnemonic(
			    GTK_LABEL(first_child), new_label.c_str());

	// Tell the user about this trick:
	gnome_appbar_set_status(GNOME_APPBAR(status),
				    _("Tab numbers restarted at 1"));
    }
}


void
SagasuApp::close_current_tab_cb(GtkWidget *, gpointer)
{
    tab_close_clicked_cb(NULL, get_current_result_page());
}


void
SagasuApp::say_sagasu_cb(GtkWidget *, gpointer)
{
    string fn = get_dir(PKGSOUNDDIR, "PKGSOUNDDIR") + PACKAGE + ".wav";
    gnome_sound_play(fn.c_str());
}
