/*  $Id: SagasuApp.h,v 1.13 2012/11/25 00:58:21 sarrazip Exp $
    SagasuApp.h - Class representing the main window

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

#ifndef _H_SagasuApp
#define _H_SagasuApp

#include "ResultPage.h"

#include <sys/types.h>
#include <unistd.h>


class SagasuApp
{
public:

    static void create_instance(
			const std::string &init_search_expr,
			const std::string &init_search_dir);
    /*
	Creates the unique instance of this class.
	Initializes it with the given parameters.
    */

    static SagasuApp &get_instance();
    /*
	Retrieves the unique instance of this class.
	create_instance() must have been called previously.
    */

    ~SagasuApp();
    /*
	Nothing interesting.
    */

    void start_search();

    std::string get_file_patterns() const;

    bool match_whole_words() const;
    bool match_case() const;
    bool use_regex() const;
    gint dir_recursion_depth() const;
    bool exclude_symlinked_dirs() const;
    /*
	These methods return the state of the search parameter checkboxes.
    */

    void show_num_matching_lines();

    gboolean input_channel_ready(GIOCondition condition);
    /*
	Called when characters are ready to be read from the pipe
	connected to the Perl script that does the search.
    */

    void process_pending_input();
    /*
	Processes any complete lines of text stored in the pending input
	buffer of this object.  The input in question is the text that
	comes from the Perl script that does the search.
    */

    std::string get_result_text_line_for_pos(gint pos_in_chars) const;
    /*
	Retrieves the line of text at the given position from the
	current result page's text buffer.
	The retrieved line contains the given position; this position
	does not have to be the start of the line.
	The position must be expressed in UTF-8 encoding.
	The returned string is in Latin-1 however.
    */

    void launch_editor_for_position(gint pos_in_chars);
    /*
	Launches an editor with parameters that are taken from the text line
	that contains the given position in the current result page's
	text buffer.
	Actually, the 'editor command' specified by the user is executed,
	and this command does not have to be an editor.  The %n and %f
	placeholders in the editor command are replaced with the line
	number and filename found in the text line.
    */

    void save_configuration();
    /*
	Save the current settings of the main dialog in the GNOME
	configuration file for this application.
    */

    void set_result_search_button_sensitivity();
    /*
	Sets the sensitivity of the 'Find' button in the 'find in
	results' subdialog according to the emptiness of the
	associated entry.
    */

    #define WIDGET_AND_POINTER_CALLBACK(cb) \
    	void cb(GtkWidget * = NULL, gpointer = NULL);
    #include "callbacks.h"
    #undef WIDGET_AND_POINTER_CALLBACK

    void close_search_dir_file_sel();
    void accept_search_dir();

    gboolean key_press_in_search_field(GdkEventKey *event);
    gboolean key_press_in_result_search_entry(GdkEventKey *event);

private:

    GtkWidget *appwin;

    GtkWidget *search_string_entry;
    GtkWidget *file_patterns_entry;
    GtkWidget *search_dir_entry;
    GtkWidget *excl_dirs_entry;
    GtkWidget *editor_cmd_entry;

    GtkWidget *search_button;
    GtkWidget *default_file_patterns_button;
    GtkWidget *browse_search_dir_button;
    GtkWidget *default_excl_dirs_button;
    GtkWidget *default_editor_cmd_button;

    GtkWidget *match_whole_words_button;
    GtkWidget *match_case_button;
    GtkWidget *use_perl_regex_button;
    GtkWidget *dir_recursion_depth_spin_button;
    GtkWidget *exclude_symlinked_dirs_button;

    GtkWidget *result_notebook;
    unsigned long next_notebook_page_num;
    std::string default_file_patterns;
    std::string default_excl_dirs;
    std::string default_editor_command;
    GIOChannel *input_channel;
    int input_fd;
    pid_t search_pid;
    ResultPage *result_page_of_current_search;
    std::string pending_input;
    size_t num_matching_lines;
    size_t num_matching_files;
    std::string last_matching_filename;
    GtkWidget *status;

    GtkWidget *result_search_entry;
    GtkWidget *result_search_button;

    GtkWidget *search_dir_file_sel_dlg;
    GtkWidget *search_dir_file_sel_label;


    static SagasuApp *instance;


    SagasuApp(const std::string &init_search_expr,
		const std::string &init_search_dir);
    void build_user_interface(const std::string &init_search_expr,
				const std::string &init_search_dir);
    std::string make_next_page_tab_label();
    GtkWidget *make_next_page_tab(ResultPage *page);
    void load_configuration();
    ResultPage *get_current_result_page() const;
    void create_search_process(const std::string &target_expr,
				const std::string &raw_target_expr,
				const std::string &search_dir,
				const std::string &file_patterns,
				bool match_case,
				gint recursion_depth,
				const std::string &excl_dirs,
				bool exclude_symlinked_dirs);
    void close_input();

    void show_working_state(bool busy);
    void display_wait_cursor();
    void remove_wait_cursor();

    // Forbidden operations:
    SagasuApp(const SagasuApp &);
    SagasuApp &operator = (const SagasuApp &);
};


#endif  /* _H_SagasuApp */
