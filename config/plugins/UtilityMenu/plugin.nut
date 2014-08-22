///////////////////////////////////////////////////
//
// Attract-Mode Frontend - Utility Menu plugin
//
///////////////////////////////////////////////////
//
// Define use configurable settings
//

const OPT_HELP="The text to show in the menu for this item";
const CMD_HELP="The command to run when this item is selected.  Use @<script_name.nut> to run a squirrel script that is located in the Utility Menu's plugin directory.";

class UserConfig </ help="Menu for running utility commands and scripts" /> {
	</ label="Control", help="The control to press to display the utility menu", is_input=true, order=1 />
	button="U";

	</ label="Item 1 Menu Text", help=OPT_HELP, order=2 /> opt1="Shell Command";
	</ label="Item 1 Command",   help=CMD_HELP, order=3 /> opt1_cmd="@any_command.nut";

	</ label="Item 2 Menu Text", help=OPT_HELP, order=4 /> opt2="";
	</ label="Item 2 Command",   help=CMD_HELP, order=5 /> opt2_cmd="";

	</ label="Item 3 Menu Text", help=OPT_HELP, order=6 /> opt3="";
	</ label="Item 3 Command",   help=CMD_HELP, order=7 /> opt3_cmd="";

	</ label="Item 4 Menu Text", help=OPT_HELP, order=8 /> opt4="";
	</ label="Item 4 Command",   help=CMD_HELP, order=9 /> opt4_cmd="";

	</ label="Item 5 Menu Text", help=OPT_HELP, order=10 /> opt5="";
	</ label="Item 5 Command",   help=CMD_HELP, order=11 /> opt5_cmd="";

	</ label="Item 6 Menu Text", help=OPT_HELP, order=12 /> opt6="";
	</ label="Item 6 Command",   help=CMD_HELP, order=13 /> opt6_cmd="";

	//
	// To add more items, just follow the pattern above...
	//</ label="Item 7 Menu Text", help=OPT_HELP, order=14 /> opt7="";
	//</ label="Item 7 Command",   help=CMD_HELP, order=15 /> opt7_cmd="";
}

local config=fe.get_config();
local my_dir = fe.script_dir;
local items = [];
local actions = [];

const MAX_OUTPUT_LINES = 40;

fe.load_module( "submenu" );

class AnyCommandOutput extends SubMenu
{
	m_t = "";

	constructor()
	{
		base.constructor();
		m_t = fe.add_text( "", 0, 0, fe.layout.width, fe.layout.height );
		m_t.charsize=fe.layout.height / MAX_OUTPUT_LINES;
		m_t.align=Align.Left;
		m_t.word_wrap=true;
		m_t.bg_alpha=180;
		m_t.visible = false;
	}

	function on_show() { m_t.visible = true; }
	function on_hide() { m_t.visible = false; }
	function on_scroll_up() { m_t.first_line_hint--; }
	function on_scroll_down() { m_t.first_line_hint++; }

	function show_output( msg )
	{
		m_t.msg = msg;
		m_t.first_line_hint=0;

		show( true );
	}
};
fe.plugin[ "Utility Menu" ] <- AnyCommandOutput();

//
// Load configured menu items into the menu
//
local i=1;
while (  config.rawin( "opt" + i ) )
{
	local temp_entry = config[ "opt" + i ];
	if ( temp_entry.len() > 0 )
	{
		items.append( temp_entry );
		actions.append( config[ "opt" + i + "_cmd" ] );
	}
	i++;
}

//
// Add a cancel/back option
//
items.append( "Back" );

//
// Create a tick function that tests if the configured button is pressed and runs
// the corresponding script or command if it is.
//
fe.add_ticks_callback( "utility_menu_plugin_tick" );

function utility_menu_plugin_tick( ttime )
{
	if ( fe.get_input_state( config["button"] ) )
	{
		local res = fe.overlay.list_dialog( items, "Utility Menu", items.len() / 2 );
		if (( res < 0 ) || ( res >= actions.len() ) || ( actions[res].len() < 1 ))
			return;

		if ( actions[res].slice( 0, 1 ) == "@" )
		{
			local script_file = my_dir + actions[res].slice( 1 );
			print( "[UtilityMenu] Running script: " + script_file + "\n" );
			dofile( script_file );
		}
		else
		{
			print( "[UtilityMenu] Running command: " + actions[res] + "\n" );
			system( actions[res] );
		}
	}
}
