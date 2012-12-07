/*  $Id: main.cpp,v 1.6 2012/11/25 00:58:22 sarrazip Exp $
    main.cpp - main() function to initialize the GNOME 2 application

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

#include "SagasuApp.h"

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#include <locale.h>  /* required by g++ 2.95.3 to define LC_ALL */

#include <assert.h>
#include <libintl.h>
#include <libgnomeui/libgnomeui.h>
#include <iostream>  /* defines LC_ALL */

using namespace std;


static
struct poptOption options[] =
{
    {
	NULL,
	'\0',
	0,
	NULL,
	0,
	NULL,
	NULL
    }
};


static
poptContext
get_poptContext(GnomeProgram *program)
{
    GValue value;
    ::memset(&value, '\0', sizeof(value));
    g_value_init(&value, G_TYPE_POINTER);
    g_object_get_property(G_OBJECT(program),
			    GNOME_PARAM_POPT_CONTEXT, &value);
    return (poptContext) g_value_get_pointer(&value);
}


int
main(int argc, char *argv[])
{
    // GNU Gettext initialization:

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");  // _() will yield UTF-8 strings
    textdomain(PACKAGE);

    // Program initialization:

    GnomeProgram *program = gnome_program_init(
    		PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv,
		GNOME_PARAM_POPT_TABLE, options,
		GNOME_PARAM_APP_DATADIR, DATADIR,
		NULL);
    gnome_window_icon_set_default_from_file(GNOMEICONDIR "/" PACKAGE ".png");


    // Argument parsing:

    poptContext pctx = get_poptContext(program);

    const char **args = poptGetArgs(pctx);

    string init_search_expr, init_search_dir;
    if (args != NULL && args[0] != NULL)
    {
	init_search_expr = args[0];
	if (args[1] != NULL)
	{
	    init_search_dir = args[1];
	    if (args[2] != NULL)
		cerr << PACKAGE << ": "
		    << latin1(_("superfluous arguments ignored")) << endl;

	    // TODO: gettext returns UTF-8 strings in this program.
	    // Do not convert to Latin-1 if terminal is in UTF-8.
	}
    }

    poptFreeContext(pctx);


    SagasuApp::create_instance(init_search_expr, init_search_dir);


    // Main loop:

    gtk_main();

    return EXIT_SUCCESS;
}
