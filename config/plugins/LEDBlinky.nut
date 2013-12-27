///////////////////////////////////////////////////
//
// Attract-Mode Frontend - LEDBlinky plugin
//
///////////////////////////////////////////////////
//
// Usage:
//
// 1.  Install and configure LedBlinky on your system:
//     http://www.ledblinky.net 
//
// 2.  Copy this file to the "plugins" directory of your Attract-Mode
//     configuration.
//
// 3.  Run Attract-Mode and configure the LEDBlinky plugin from config mode.
//
///////////////////////////////////////////////////
class UserConfig </ help="Integration plug-in for use with LEDBlinky: http://www.ledblinky.net" /> { };

local ledblinky=fe.init_name;
fe.add_transition_callback( "ledblinky_plugin_transition" );

function ledblinky_plugin_transition( ttype, var, ttime ) {

	if ( ScreenSaverActive )
		return false;

	switch ( ttype )
	{
	case Transition.ToGame:
		fe.plugin_command( ledblinky, 
			"\"" + fe.game_info( Info.Name ) + "\" \"" 
			+ fe.game_info( Info.Emulator ) + "\"" );
		break;

	case Transition.FromGame:
		fe.plugin_command( ledblinky, "4" );
		break;

	case Transition.StartLayout:
		switch ( var )
		{
		case FromTo.ScreenSaver: // leaving screensaver
			fe.plugin_command( ledblinky, "6" );
			break;

		case FromTo.Frontend: // starting frontend
			fe.plugin_command( ledblinky, "1" );
			break;
		}
		break;

	case Transition.EndLayout:
		switch ( var )
		{
		case FromTo.ScreenSaver: // starting screensaver
			fe.plugin_command( ledblinky, "5" );
			break;

		case FromTo.Frontend: // ending frontend
			fe.plugin_command( ledblinky, "2" );
			break;
		}
		break;

	case Transition.ToNewList:
		// ignore this event if leaving screensaver or starting FE
		if ( var == FromTo.Null ) 
			fe.plugin_command( ledblinky, "8" );
		break;
	}

	return false;
}
