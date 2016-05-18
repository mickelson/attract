///////////////////////////////////////////////////
//
// "SubMenu" class
//
// Extend this class to create a submenu in the frontend
//
// A "trigger" input for the submenu can be set and once triggered
// the submenu will be displayed until the user either presses the
// trigger again, presses the "exit" input or presses the "select"
// input.
//
// The "up" and "down" inputs are captured to support scrolling
// while the submenu is displayed.
//
// The constructor of your class should pass one parameter to the
// base constructor if you want it to be triggered by a specific
// input.  This parameter should represent what the "trigger" input
// is for the menu
//
// Implement the following functions in your class to perform the
// appropriate actions:
//
//	function on_show()
//	function on_hide()
//	function on_scroll_up()
//	function on_scroll_down()
//
// on_tick() and on_transition() functions can be implemented as well
// in your class so long as you also call the corresponding base class
// function.
//
///////////////////////////////////////////////////
class SubMenu
{
	m_up = false;
	m_block = false;
	m_curr_scroll_button = "";
	m_last_scroll_tick = 0;
	m_trigger="";

	constructor( trigger="" )
	{
		m_trigger = trigger;
		fe.add_ticks_callback( this, "on_tick" );
	};

	function on_tick( ttime )
	{
		// Check whether the trigger been pressed.  
		//
		local button_down = fe.get_input_state( m_trigger );

		if ( !m_block && button_down && !fe.overlay.is_up )
		{ 
			show( !m_up );
			m_block = true;
		}
		else if ( !button_down && m_block )
			m_block = false;

		//
		// Now deal with scrolling a displayed menu
		//
		if ( m_curr_scroll_button.len() > 0 )
		{
			local nav_down = fe.get_input_state( m_curr_scroll_button );
			if ( !nav_down )
				m_curr_scroll_button = "";
			else if ( ttime > m_last_scroll_tick + 10 )
			{
				on_signal( m_curr_scroll_button );
				m_last_scroll_tick = ttime;
			}
		}
	}

	function on_signal( signal )
	{
		switch ( signal )
		{
		case "up":
			on_scroll_up();
			m_curr_scroll_button = signal;
			return true;

		case "down":
			on_scroll_down();
			m_curr_scroll_button = signal;
			return true;

		case "select":
			on_select();
			return true;

		case "back":
			show( false );
			return true;
		}

		return false;
	}

	function show( flag )
	{
		if ( flag )
		{
			m_up = true;
			fe.add_signal_handler( this, "on_signal" );
			on_show();
		}
		else
		{
			m_up = false;
			fe.remove_signal_handler( this, "on_signal" );
			on_hide();
		}
	}

	//
	// These are the functions that need to be implemented in your
	// derived class in order for anything interesting to happen
	//
	function on_show() { print( "SubMenu::on_show()\n" ); }
	function on_hide() { print( "SubMenu::on_hide()\n" ); }
	function on_scroll_up() {}
	function on_scroll_down() {}
	function on_select() { show( false ); }
};
