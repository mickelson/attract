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
// 3.  Run Attract-Mode and enter configuration mode.  Create a plugin 
//     labelled "espeak", and enter the path to the espeak executable 
//     file as the plugin command.
//
///////////////////////////////////////////////////
//
// User configurable settings:
//
const ESpeakOptions = ""; // requires trailing space if options added
const ESpeakWelcomeMsg = "Welcome";
const ESpeakGoodbyeMsg = "Goodbye";

///////////////////////////////////////////////////

local espeak=fe.init_name;
fe.add_transition_callback( "espeak_plugin_transition" );

function espeak_plugin_transition( ttype, var, ttime ) {

	if ( ScreenSaverActive )
		return false;

	switch ( ttype )
	{
	case Transition.StartLayout:
		if (( var == FromTo.Frontend ) && ( ESpeakWelcomeMsg.len() > 0 ))
			fe.plugin_command_bg( espeak, 
				ESpeakOptions + "\"" + ESpeakWelcomeMsg + "\"" );
		break;

	case Transition.EndLayout:
		if (( var == FromTo.Frontend ) && ( ESpeakGoodbyeMsg.len() > 0 ))
			fe.plugin_command_bg( espeak, 
				ESpeakOptions + "\"" + ESpeakGoodbyeMsg + "\"" );
		break;

	case Transition.ToGame:
		//
		// Only announce names up to the first open bracket
		//
		local speech = split( fe.game_info( Info.Title ), "([{<" );

		if ( speech.len() > 0 )
			fe.plugin_command_bg( espeak, 
				ESpeakOptions + "\"" + speech[0] + "\"" );
		break;
	}

	return false; // must return false
}
