///////////////////////////////////////////////////
//
// Attract-Mode Frontend - ResFix plugin
//
///////////////////////////////////////////////////
class UserConfig </ help="This plugin will recreate the Attract-Mode window when returning from a game.  Use this to correct Attract-Mode's display if the game/emulator changes the screen resolution away from what it was previously" /> {

	</ label="Specific Emulators", help="If you list specific emulators here, then the window will only be reset for those specified emulators.  Multiple entries can be separated by a semicolon.  If this is left blank then the window will be reset for every emulator", order=1 />
	emulators="";
};

class ResFix
{
	emu_array = [];

	constructor()
	{
		local my_config = fe.get_config();

		if ( my_config[ "emulators" ].len() > 0 )
			emu_array = split( my_config[ "emulators" ], ";" );

		fe.add_transition_callback( this, "on_transition" );
	}

	function on_transition( ttype, var, ttime )
	{
		if ( ttype == Transition.FromGame )
		{
			if (( emu_array.len() == 0 )
				|| ( emu_array.find(
					fe.game_info( Info.Emulator )
					) != null ))
			{
				fe.signal( "reset_window" );
			}
		}
		return false;
	}
}

fe.plugin[ "ResFix" ] <- ResFix();

