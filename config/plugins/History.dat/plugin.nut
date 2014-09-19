///////////////////////////////////////////////////
//
// Attract-Mode Frontend - History.dat plugin
//
///////////////////////////////////////////////////
//
// Define use configurable settings
//
class UserConfig </ help="History.dat viewer for the Attract-Mode frontend" /> {
	</ label="Control", help="The button to press to view history", is_input=true, order=1 />
	button="H";

	</ label="File Path", help="The full path to the history.dat file", order=2 />
	dat_path="$HOME/history.dat";

	</ label="Rows", help="Set the number of rows of text to display in the viewer", order=3 />
	rows="30";

	</ label="Index Clones", help="Set whether entries for clones should be included in the index.  Enabling this will make the index significantly larger", order=4, options="Yes,No" />
	index_clones="No";

	</ label="Generate Index", help="Generate the history.dat index now (this can take some time)", is_function=true, order=5 />
	// Map the config option to the "generate_index" function in "file_util.nut"
	generate="generate_index";
}

//
// Load our history.dat file utilities
//
local my_dir = fe.script_dir;
dofile( my_dir + "file_util.nut" );

fe.load_module( "submenu" );
local config=fe.get_config();

//
// Define our history viewer by extending the SubMenu class from the
// "submenu" module
//
class HistoryViewer extends SubMenu
{
	m_text = "";
	m_curr_rom = "";

	constructor()
	{
		base.constructor( config["button"] );

		m_text = fe.add_text( "", 0, 0, fe.layout.width, fe.layout.height );
		m_text.first_line_hint = 0; // enables word wrapping
		m_text.charsize = fe.layout.height / config[ "rows" ].tointeger();
		m_text.bg_alpha=220;
		m_text.visible=false;
	}

	function on_show()
	{
		local sys = split( fe.game_info( Info.System ), ";" );
		local rom = fe.game_info( Info.Name );

		//
		// we only go to the trouble of loading the entry if
		// it is not already currently loaded
		//
		if ( m_curr_rom != rom )
		{
			m_curr_rom = rom;
			local alt = fe.game_info( Info.AltRomname );
			local cloneof = fe.game_info( Info.CloneOf );

			local lookup = get_history_offset( sys, rom, alt, cloneof );

			if ( lookup >= 0 )
			{
				m_text.first_line_hint = 0;
				m_text.msg = get_history_entry( lookup, config );
			}
			else
			{
				if ( lookup == -2 )
					m_text.msg = "Index file not found.  Try generating an index from the history.dat plug-in configuration menu.";
				else	
					m_text.msg = "Unable to locate: "
						+ rom;
			}
		}

		m_text.visible=true;
	}

	function on_hide()
	{
		m_text.visible=false;
	}

	function on_scroll_up()
	{
		m_text.first_line_hint--;
	}

	function on_scroll_down()
	{
		m_text.first_line_hint++;
	}
}

fe.plugin[ "History.dat" ] <- HistoryViewer();
