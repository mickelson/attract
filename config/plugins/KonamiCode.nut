///////////////////////////////////////////////////
//
// Attract-Mode Frontend - KonamiCode plugin
//
///////////////////////////////////////////////////
//
// Define the user-configurable options:
//
class UserConfig </ help="Plugin for super secret joystick/button sequences that do something" /> {

	</ label="Code Sequence", help="The secret input sequence.  Can be any combination of 'up', 'down', 'left', 'right', 'A' or 'B', separated by commas", order=1 />
	code="up,up,down,down,left,right,left,right,B,A";

	</ label="Action", help="What to do if the input sequence is entered", options="configure,exit,exit_no_menu,lists_menu,filters_menu,replay_last_game,custom1,custom2", order=2 />
	action="configure";

	</ label="Up control", help="The input to press for 'up'", is_input=true, order=3 />
	up="";

	</ label="Down control", help="The input to press for 'down'", is_input=true, order=4 />
	down="";

	</ label="Left control", help="The input to press for 'left'", is_input=true, order=5 />
	left="";

	</ label="Right control", help="The input to press for 'right'", is_input=true, order=6 />
	right="";

	</ label="A button", help="The input to press for 'A'", is_input=true, order=7 />
	A="";

	</ label="B button", help="The input to press for 'B'", is_input=true, order=8 />
	B="";
}

// if there is 1.5 seconds of no further code entry we reset to the beginning
//
const RESET_TIME = 1500;

class KonamiCode
{
	m_config = null;

	m_current = 0;
	m_last_time = 0;
	m_pressed = false;
	m_commands = null;
	m_valid = [ "up", "down", "right", "left", "A", "B" ];

	constructor()
	{
		m_config = fe.get_config();
		local temp = split( m_config[ "code" ], "," );

		// Make sure m_commands only gets the values we expect
		//
		m_commands = [];
		foreach ( c in temp )
		{
			local cc = strip( c );
			if ( m_valid.find( cc ) != null )
				m_commands.append( cc )
		}

		if ( m_commands.len() > 0 )
		{
			fe.add_ticks_callback( this, "on_tick" );
			fe.add_transition_callback( this, "on_transition" );
		}
	}

	function on_tick( ttime )
	{
		if ( fe.overlay.is_up )
			return;

		if ( m_current < m_commands.len() )
		{
			local down = fe.get_input_state( m_config[ m_commands[ m_current ] ] );
			if ( down )
				m_pressed=true;
			else if ( !down && m_pressed )
			{
				// register the keypress only after release
				m_current++;
				m_last_time = ttime;
				m_pressed = false;
			}
			else if ( m_current > 0 )
			{
				// Check for whether we need to cancel the sequence
				//
				if ( ttime > m_last_time + RESET_TIME )
				{
					// timeout
					m_current = 0;
				}
				else
				{
					foreach ( v in m_valid )
					{
						if ( fe.get_input_state( m_config[ v ] ) )
						{
							// wrong input in sequence
							m_current=0;
							break;
						}
					}
				}
			}
		}
		else
		{
			m_current=0;
			fe.signal( m_config[ "action" ] );
		}
	}

	function on_transition( ttype, var, ttime )
	{
		if ( ttype == Transition.StartLayout )
		{
			if ( fe.nv.rawin( "KonamiCode" ) )
			{
				m_current = fe.nv[ "KonamiCode" ];
				m_last_time = RESET_TIME / 2;
				delete fe.nv[ "KonamiCode" ];
			}
		}
		else if (( ttype == Transition.EndLayout )
			&& ( m_current > 0 ))
		{
			fe.nv[ "KonamiCode" ] <- m_current;
		}
	}
}

// Create an entry in the fe.plugin table in case anyone else wants to
// find this plugin.
//
if ( !ScreenSaverActive )
	fe.plugin[ "KonamiCode" ] <- KonamiCode();
