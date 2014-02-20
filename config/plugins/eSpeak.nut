///////////////////////////////////////////////////
//
// Attract-Mode Frontend - eSpeak plugin
//
///////////////////////////////////////////////////
//
// Usage: 
//
// 1.  Install eSpeak on your system: http://espeak.sourceforge.net/
//
// 2.  Copy this file to the "plugins" directory of your Attract-Mode 
//     configuration.
//
// 3.  Run Attract-Mode and enter configuration mode.  Configure the
//     eSpeak plugin with the path to the espeak executable.
//
///////////////////////////////////////////////////

//
// The UserConfig class identifies plugin settings that can be configured
// from Attract-Mode's configuration menu
//
class UserConfig </ help="Integration plug-in for use with eSpeak Speech Synthesizer: http://espeak.sourceforge.net" /> {
	</ label="Voice", help="Select Voice", options="Default, Male1, Male2, Male3,Female1, Female2, Female3, MBrola_en1" />
	a_voice="Default";

	</ label="Welcome Message", help="Message to play on startup" />
	b_welcome="Welcome";

	</ label="Goodbye Message", help="Message to play on exit" />
	c_goodbye="Goodbye";
}

local espeak=fe.init_name;
local config=fe.get_config(); // get the plugin settings configured by the user

local v_map = {
	Default=""
	Male1="-v male1 "
	Male2="-v male2 "
	Male3="-v male3 "
	Female1="-v female1 "
	Female2="-v female2 "
	Female3="-v female3 "
	MBrola_en1="-v mb-en1 "
};

local options = v_map[ config["a_voice"] ];

fe.add_transition_callback( "espeak_plugin_transition" );

function espeak_plugin_transition( ttype, var, ttime ) {

	if ( ScreenSaverActive )
		return false;

	switch ( ttype )
	{
	case Transition.StartLayout:
		if (( var == FromTo.Frontend ) && ( config["b_welcome"].len() > 0 ))
			fe.plugin_command_bg( espeak, options + "\"" + config["b_welcome"] + "\"" );
		break;

	case Transition.EndLayout:
		if (( var == FromTo.Frontend ) && ( config["c_goodbye"].len() > 0 ))
			fe.plugin_command_bg( espeak, options + "\"" + config["c_goodbye"] + "\"" );
		break;

	case Transition.ToGame:
		//
		// Only announce names up to the first open bracket
		//
		local speech = split( fe.game_info( Info.Title ), "([{<" );

		if ( speech.len() > 0 )
			fe.plugin_command_bg( espeak, options + "\"" + speech[0] + "\"" );
		break;
	}

	return false; // must return false
}
