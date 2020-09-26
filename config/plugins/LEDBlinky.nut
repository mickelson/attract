///////////////////////////////////////////////////
//
// Attract-Mode Frontend - LEDBlinky plugin v3.0
//
///////////////////////////////////////////////////
//
// Usage:
//
// 1. Download LEDBlinky from https://ledblinky.net/Download.htm
//
// 2. Install LEDBlinky to the Attract-Mode \plugins folder.
//    Make sure to select the Attract-Mode plugin from the optional components list.
//
// 3. Configure LEDBlinky per directions in "Install and Config" pdf, readme, or online.
//
// 4. Run Attract-Mode and enable and configure the LEDBlinky plugin from the Plugins menu.
//
// NOTE: If installing this plugin manually, it must reside in the \LEDBlinky folder under the Attract-Mode \plugins folder; \plugins\LEDBlinky\plugin.nut
// NOTE: If you were using an older version of the LEDBlinky plugin (LEDBlinky.nut), it must be removed from the plugins folder.
//
///////////////////////////////////////////////////
class UserConfig </ help="Attract-Mode plug-in (v3.0) for use with LEDBlinky: https://www.LEDBlinky.net" /> {
	</ label="Command", help="Path to the LEDBlinky executable", order=1 />
	command = "plugins\\LEDBlinky\\LEDBlinky.exe";
};

class LEDBlinky
{
	config = null;
	last_transition = "none";
	debug_mode = false;
	printprefix = ">>>>>> ";

	constructor()
	{
		config = fe.get_config();
		fe.add_transition_callback( this, "on_transition" );	
		
		//We'll use a persistent value in the fe.nv table to determine if this is the first time the plugin is loaded.
		//Values in fe.nv are persisted in the script.nv file.
		//Note: Attract-Mode seems to reload each plugin multiple times, 
		//twice at startup and each time the Displays list is active. Not sure why?
		if (fe.nv.rawin("LEDBlinkyLastTransision")) { //If global exists then this is not the first time we've loaded the plugin.
			last_transition = fe.nv["LEDBlinkyLastTransision"]; //Get last transition from global.
		}
		else { //First Run
			fe.nv["LEDBlinkyLastTransision"] <- last_transition; //Add persistent global to store last transition. Global will be deleted when FE quits.
		
			//Load LEDBlinky
			if ( debug_mode ) print( printprefix + "EVENT_FE_START" + "\n" );
			fe.plugin_command_bg( config["command"], "1" ); //EVENT_FE_START
		}
	}

	function on_transition( ttype, var, ttime )
	{
		if ( debug_mode ) print( printprefix + "ttype: [" + ttype + "] var: [" + var + "] ttime: [" + ttime + "] rom: [" + fe.game_info( Info.Name ) + 
			"] emu: [" + fe.game_info( Info.Emulator ) + "] list: [" + fe.list.name + "] list index: [" + fe.list.display_index +
			"] last ttype: [" + last_transition + "]\n" );
			
		if ( !ScreenSaverActive ) {
		
			switch( ttype )
			{
			case Transition.ToGame:
				if ( debug_mode ) print( printprefix + "EVENT_GAME_START " + fe.game_info( Info.Name ) + " " + fe.game_info( Info.Emulator ) + "\n" );
				fe.plugin_command_bg( config["command"], "\"" + fe.game_info( Info.Name ) + "\" \"" + fe.game_info( Info.Emulator ) + "\"" );
				break;

			case Transition.FromGame:
				if ( debug_mode ) print( printprefix + "EVENT_GAME_QUIT" + "\n" );
				fe.plugin_command_bg( config["command"], "4" ); //EVENT_GAME_QUIT
				break;

			case Transition.StartLayout:
				if ( var == FromTo.ScreenSaver ) { //Leaving screensaver
					if ( debug_mode ) print( printprefix + "EVENT_SCREENSAVER_STOP" + "\n" );
					fe.plugin_command_bg( config["command"], "6" ); //EVENT_SCREENSAVER_STOP
				}
				break;

			case Transition.EndLayout:
				switch( var )
				{
				case FromTo.ScreenSaver: //Starting screensaver
					if ( debug_mode ) print( printprefix + "EVENT_SCREENSAVER_START" + "\n" );
					fe.plugin_command_bg( config["command"], "5" ); //EVENT_SCREENSAVER_START
					break;

				case FromTo.Frontend: //Ending FE
					if ( debug_mode ) print( printprefix + "EVENT_FE_QUIT" + "\n" );
					if (fe.nv.rawin("LEDBlinkyLastTransision")) delete fe.nv[ "LEDBlinkyLastTransision" ]; //Don't want to persist LEDBlinky global after quiting.
					fe.plugin_command_bg( config["command"], "2" ); //EVENT_FE_QUIT
					break;
				}
				break;

			case Transition.ToNewList:
				if ( last_transition != Transition.StartLayout + "," + FromTo.ScreenSaver ) {
					local temp = "\"" + fe.game_info( Info.Emulator ) + "\""; //For game list pass LEDBlinky the emulator.
					if ( fe.list.display_index == -1 ) temp = "\"" + fe.list.name + "\" display_menu"; //For display list pass LEDBlinky the display menu name plus flag.
					
					if ( debug_mode ) print( printprefix + "EVENT_LIST_SELECTED " + temp + "\n" );
					fe.plugin_command_bg( config["command"], "8 " + temp ); //EVENT_LIST_SELECTED
				}
				break;
				
			case Transition.FromOldSelection:
				local temp = " \"" + fe.game_info( Info.Emulator ) + "\""; //For game list pass LEDBlinky the emulator as second param.
				if ( fe.list.display_index == -1 ) temp = " \"" + fe.list.name + "\""; //For display list pass LEDBlinky the display menu name as second param.
					
				if ( debug_mode ) print( printprefix + "EVENT_GAME_SELECTED " + fe.game_info( Info.Name ) + temp + "\n" );
				fe.plugin_command_bg( config["command"], "9 \"" + fe.game_info( Info.Name ) + "\"" + temp ); //EVENT_GAME_SELECTED
				break;
			}
		}
		
		//Retain last transition
		if ( last_transition != ttype + "," + var ) {
			last_transition = ttype + "," + var;
			if (fe.nv.rawin("LEDBlinkyLastTransision")) fe.nv[ "LEDBlinkyLastTransision" ] = last_transition; //Save transition in global.
		}
		
		return false;
	}
}

fe.plugin[ "LEDBlinky" ] <- LEDBlinky();
