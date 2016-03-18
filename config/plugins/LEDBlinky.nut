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
class UserConfig </ help="Integration plug-in for use with LEDBlinky: http://www.ledblinky.net" /> {
	</ label="Command", help="Path to the LEDBlinky executable", order=1 />
	command = "LEDBlinky.exe";
};

class LedBlinky
{
	config = null;

	constructor()
	{
		config=fe.get_config();
		fe.add_transition_callback( this, "on_transition" );
	}

	function on_transition( ttype, var, ttime )
	{

		if ( ScreenSaverActive )
			return false;

		switch ( ttype )
		{
		case Transition.ToGame:
			fe.plugin_command_bg( config["command"],
				"\"" + fe.game_info( Info.Name ) + "\" \""
				+ fe.game_info( Info.Emulator ) + "\"" );
			break;

		case Transition.FromGame:
			fe.plugin_command_bg( config["command"], "4" );
			break;

		case Transition.StartLayout:
			switch ( var )
			{
			case FromTo.ScreenSaver: // leaving screensaver
				fe.plugin_command_bg( config["command"], "6" );
				break;

			case FromTo.Frontend: // starting frontend
				fe.plugin_command_bg( config["command"], "1" );
				break;
			}
			break;

		case Transition.EndLayout:
			switch ( var )
			{
			case FromTo.ScreenSaver: // starting screensaver
				fe.plugin_command_bg( config["command"], "5" );
				break;

			case FromTo.Frontend: // ending frontend
				fe.plugin_command_bg( config["command"], "2" );
				break;
			}
			break;

		case Transition.ToNewList:
			// TODO: don't do this one when screensaver is stopping
			// or frontend is first beginning
			fe.plugin_command_bg( config["command"], "8" );
			break;
		}

		return false;
	}
}

fe.plugin[ "LedBlinky" ] <- LedBlinky();
