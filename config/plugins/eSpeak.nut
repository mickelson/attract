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
	</ label="Voice", help="Select Voice", options="Male,Female" />
	voice="Male";

	</ label="Welcome Message", help="Message to play on startup" />
	welcome="Welcome";

	</ label="Goodbye Message", help="Message to play on exit" />
	goodbye="Goodbye";
}

local espeak=fe.init_name;

//
// Copy the configured values from uconfig so we can use them
// whenever the transition callback function gets called
//
local welcome_msg = fe.uconfig["welcome"];
local goodbye_msg = fe.uconfig["goodbye"];
local options = "";

if ( fe.uconfig["voice"] == "Female" )
{
	options = "-ven+f2 ";
}

fe.add_transition_callback( "espeak_plugin_transition" );

function espeak_plugin_transition( ttype, var, ttime ) {

	if ( ScreenSaverActive )
		return false;

	switch ( ttype )
	{
	case Transition.StartLayout:
		if (( var == FromTo.Frontend ) && ( welcome_msg.len() > 0 ))
			fe.plugin_command_bg( espeak, options + "\"" + welcome_msg + "\"" );
		break;

	case Transition.EndLayout:
		if (( var == FromTo.Frontend ) && ( goodbye_msg.len() > 0 ))
			fe.plugin_command_bg( espeak, options + "\"" + goodbye_msg + "\"" );
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
