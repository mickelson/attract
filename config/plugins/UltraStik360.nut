///////////////////////////////////////////////////
//
// Attract-Mode Frontend - UltraStik360 plugin
//
// For use with the UltraMap (windows) or ultrastikcmd (Linux, OS X) 
// mapping utilities provided by Ultimarc.
//
///////////////////////////////////////////////////

//
// The UserConfig class identifies plugin settings that can be configured
// from Attract-Mode's configuration menu
//
class UserConfig </ help="Integration plug-in for use with the UltraStik360 mapping software provided by Ultimarc: http://www.ultimarc.com" /> {

	</ label="Command", help="Path to the mapping executable", order=1 />
	command="ultrastikcmd";

	</ label="Config Extension", help="The extension of your mapping configuration files", options=".ugc,.um", order=2 />
	maps_ext=".ugc";

	</ label="Config Directory", help="The directory that contains your mapping configuration files", order=3 />
	maps_dir="$HOME/UltraMap/Maps/";

	</ label="Default Config Name", help="The name of your default mapping (minus extension)", order=4 />
	default_map="8 Way";

	</ label="Game Info", help="The game info field that corresponds to the name of your mapping configuration files", options="Name,Control", order=5 />
	maps_info="Name";
}

local config=fe.get_config(); // get user config settings corresponding to the UserConfig class above

//
// Copy the configured values from uconfig so we can use them
// whenever the transition callback function gets called
//
local maps_dir = fe.path_expand( config["maps_dir"] );

// make sure the directory ends in a slash
if (( maps_dir.len() > 0 ) 
	&& (maps_dir[ maps_dir.len() - 1 ].tointeger() != 47 )  // frontslash
	&& (maps_dir[ maps_dir.len() - 1 ].tointeger() != 92 )) // backslash
{
	maps_dir += "/";
}

local maps_info=Info.Name;
if ( config["maps_info"] == "Control" )
	maps_info=Info.Control;

fe.add_transition_callback( "ultra_plugin_transition" );

function ultra_plugin_transition( ttype, var, ttime ) {

	if ( ScreenSaverActive )
		return false;

	switch ( ttype )
	{
	case Transition.ToGame:
		fe.plugin_command( config["command"], 
			"\"" + maps_dir 
			+ fe.game_info( maps_info ) 
			+ config["maps_ext"] + "\"" );
		break;

	case Transition.FromGame:
		fe.plugin_command( config["command"], 
			"\"" + maps_dir 
			+ config["default_map"]
			+ config["maps_ext"] + "\"" );
		break;
	}

	return false; // must return false
}
