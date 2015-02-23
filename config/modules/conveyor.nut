///////////////////////////////////////////////////
//
// Attract-Mode Frontend - "conveyor" module
//
// The 'orbit' and 'grid' layouts are good examples of the usage of this
// module
//
///////////////////////////////////////////////////
const CONVEYOR_VERSION=2;

///////////////////////////////////////////////////
//
// ConveyorSlot class
//
// A class that extends this class and reimplements its on_progress() function
// needs to be created if you want to use the Conveyor class to manage
// a transition effect.
//
// The on_progress() function should place/scale/etc. whatever object(s)
// are being displayed appropriately.  The placement should be based on the
// value of the "progress" variable, which ranges from 0.0 to 1.0.  This range
// represents the full range of motion over which the conveyor will move the
// slot objects.
//
///////////////////////////////////////////////////
class ConveyorSlot
{
	constructor( obj=null ) { m_obj = obj; }

	function get_base_progress() { return m_base_progress; }
	function get_base_index_offset() { return m_base_io; }
	
	//
	// These functions can be overridden for anything
	// more complicated (than a single Image object)
	//
	function swap( other )
	{
		try
		{
			m_obj.swap( other.m_obj );
		}
		catch ( e )
		{
			// fallback if swap doesn't exist
			local tmp = other.m_obj;
			other.m_obj = m_obj;
			m_obj = tmp;
		}
	}

	// Set the index offset and trigger a redraw:
	function set_index_offset( io ) { m_obj.index_offset = io; }

	// Reset the index offset, preferably without triggering a redraw:
	function reset_index_offset()
	{
		try
		{
			m_obj.rawset_index_offset( m_base_io );
		}
		catch ( e )
		{
			// fallback if rawset doesn't exist
			set_index_offset( m_base_io );
		}
	}

	//
	// This function must be overridden in a class that extends this class
	// for anything interesting to happen
	//
	function on_progress( progress, var ) {}

	// For convenience, we forward the setting of things that we
	// don't know about to the underlying object
	//
	function _set( idx, val ) { m_obj[idx]=val; return val; }

	//
	// Implementation details that may change in future versions:
	//
	m_base_io=0;
	m_base_progress=0.0;
	m_obj = null;
}

///////////////////////////////////////////////////
//
// Conveyor class
//
// Create an instance of the Conveyor class to manage the navigation
// transitions for a series of ConveyorSlot-derived objects
//
// Use the set_slots() function to set the array of ConveyorSlot objects
// that the conveyor is to manage.
//
// These ConveyorSlot objects will be spread out evenly over a notional
// "progress" range from 0.0 to 1.0.  In all cases where the slot objects need
// to be placed, that will be done by calling the slot object's on_progress()
// function with the corresponding "progress" value for the object.
//
// The on_progress() function is called to place the ConveyorSlot objects both
// in their resting position as well as during each frame of a transition.
//
///////////////////////////////////////////////////
class Conveyor
{
	//
	// Set this to the length of time it should take to complete a
	// transition (in milliseconds)
	//
	transition_ms			= 220;

	//
	// Set this to a value other than 1 if you want the conveyor to jump entries
	// at a set rate when advancing over multiple entries at once.  This value
	// represents how many entries should be advanced with each 'stride'.  See
	// the grid layout for an example usage.
	//
	stride					= 1;		// must be 1 or greater

	//
	// The transition_progress value that the transition swap occurs at
	//
	transition_swap_point = 0.5;

	//
	// This gets set during transitions to a value between 0.0 (beginning)
	// and 1.0 (end) depending on where we are at in the transition
	//
	transition_progress	= 0.0;

	//
	// This gets set to the slot index that contains the current selection
	//
	selection_index		= 0;

	//
	// Set this to false to disable the conveyor
	//
	enabled			= true;

	constructor()
	{
		fe.add_transition_callback( this, "on_transition" );
	}

	//
	// Set the array of ConveyorSlot objects to be used.
	// sel_index is the array index of the object that should
	// contain the current selection
	//
	function set_slots( objs, sel_index=-1 )
	{
		if ( sel_index < 0 )
			selection_index = objs.len() / 2;
		else
			selection_index = sel_index;

		m_objs = [];

		for ( local i=0; i<objs.len(); i++ )
		{
			m_objs.push( objs[i] );
			m_objs[i].m_base_progress = i.tofloat() / objs.len();

			m_objs[i].m_base_io = i - selection_index;

			m_objs[i].reset_index_offset();
			m_objs[i].on_progress( m_objs[i].m_base_progress, 0 );
		}
	}

	//
	// Set the current selection on the conveyor
	// sel_index is the array index of the object that should
	// contain the current selection
	//
	function set_selection( sel_index=-1 )
	{
		if ( sel_index < 0 )
			selection_index = m_objs.len() / 2;
		else
			selection_index = sel_index;

		for ( local i=0; i<m_objs.len(); i++ )
		{
			m_objs[i].m_base_io = i - selection_index;
			m_objs[i].set_index_offset( m_objs[i].m_base_io );
		}
	}

	// Internal swap function used in on_transition()
	function _swap( var, adjust )
	{
		local a = ( stride < abs( var ) ) ? stride : abs( var );

		if ( var < 0 )
		{
			for ( local i=0; i < a; i++ )
				m_objs[ m_objs.len() - a + i].set_index_offset(
					m_objs[i].m_base_io - adjust );

			for ( local i=m_objs.len()-1; i>=a; i-- )
				m_objs[i].swap( m_objs[i-a] );
		}
		else
		{
			for ( local i=0; i < a; i++ )
				m_objs[i].set_index_offset(
					m_objs[m_objs.len() - a + i].m_base_io + adjust );

			for ( local i=0; i < m_objs.len()-a; i+=1 )
				m_objs[i].swap( m_objs[i+a] );
		}
	}

	function on_transition( ttype, var, ttime )
	{
		if ((!enabled) || ( ttype != Transition.ToNewSelection ) || ( m_objs.len() < 1 ))
			return false;

		if ( ttime < transition_ms )
		{
			transition_progress = ttime.tofloat() / transition_ms;

			local moves = ( stride == 1 ) ? abs( var ) : 1;

			local jump_adjust = 0;

			// Adjust for situation where we are skipping
			// ahead
			if ( moves > m_objs.len() )
			{
				jump_adjust = moves - m_objs.len();
				moves = m_objs.len();
			}

			local move_duration = transition_ms / moves;

			if ( moves > 1 )
			{
				local curr_move = ttime / move_duration;

				if ( curr_move >= m_last_move + stride )
				{
					m_last_move = curr_move;
					_swap( var, curr_move + jump_adjust );
				}
			}

			//
			// Optimization: Attract-Mode will reload artworks according to their
			// index_offset once this transition is complete.  However it is smart
			// enough to not reload where the container already holds an image/video
			// for the game that is going to be displayed.  The following swap results
			// in the artwork images being in the right place to make use of this feature
			//
			if ( !m_did_move_swap && ( transition_progress > transition_swap_point ))
			{
				_swap( var, abs(var) );
				m_did_move_swap=true;
			}

			local move_progress = ( ttime % move_duration ).tofloat()
				/ move_duration
				/ m_objs.len()
				* (( var > 0 ) ? -1 : 1);

			//
			// Correct the move progress if we've done the 'move swap'
			//
			if ( m_did_move_swap )
			{
				if ( var < 0 )
					move_progress -= 1.0/m_objs.len();
				else
					move_progress += 1.0/m_objs.len();
			}

			foreach ( o in m_objs )
				o.on_progress( o.m_base_progress + move_progress, var );

			return true;
		}

		// it is possible that we haven't done the move swap at this point
		// so make sure it is done now...
		if ( !m_did_move_swap )
			_swap( var, abs(var) );

		foreach ( o in m_objs )
		{
			o.reset_index_offset();
			o.on_progress( o.m_base_progress, 0 );
		}

		m_last_move=0;
		m_did_move_swap = false;
		return false;
	}

	function reset_progress()
	{
		foreach ( o in m_objs )
			o.on_progress( o.m_base_progress, 0 );
	}

	//
	function _set( idx, val )
	{
		foreach ( o in m_objs )
			o[idx]=val;

		return val;
	}

	m_last_move = 0;
	m_did_move_swap = false;
	m_objs = [];
};

///////////////////////////////////////////////////
// SimpleArtStripSlot class - used by SimpleArtStrip
///////////////////////////////////////////////////
class SimpleArtStripSlot extends ConveyorSlot
{
	m_p=null;
	constructor( parent, artname )
	{
		m_p=parent;
		base.constructor( ::fe.add_artwork( artname ) );
	}

	function on_progress( progress, var )
	{
		m_obj.width = m_p.m_width;
		m_obj.height = m_p.m_height;
		m_obj.x = m_p.m_x + progress * m_p.m_x_span;
		m_obj.y = m_p.m_y + progress * m_p.m_y_span;
	}
};

///////////////////////////////////////////////////
//
// SimpleArtStrip class
//
// Create a simple artwork strip that smoothly animates a transition when
// the selection changes
//
// SimpleArtStrip( artwork_label,num_objs,x,y,width,height,pad=0 )
//
// Parameters:
//
//   artwork_label - the label of the artwork to show in the strip (i.e. "snap")
//   num_objs - the number of artwork objects to show in the strip
//   x - the x position of the top left corner of the strip
//   y - the y position of the top left corner of the strip
//   width - the width of the entire strip
//   height - the height of the entire strip
//   pad - the amount of padding to insert between each object in the strip
//        (default value of 0)
//
// If height>width, you will get a vertical strip.  Otherwise the strip will
//  be horizontal.
//
// Usage example.  The following code creates a strip of 5 snap artworks across
// the top of the screen:
//
//    local my_strip = SimpleArtStrip( "snap", 5, 0, 0,
//             fe.layout.width, fe.layout.height/3, 2 );
//
///////////////////////////////////////////////////
class SimpleArtStrip extends Conveyor
{
	m_x=0; m_y=0; m_width=0; m_height=0; m_x_span=0; m_y_span=0;

	constructor( artwork_label, num_objs, x, y, width, height, pad=0 )
	{
		base.constructor();
		local my_list = [];
		for ( local i=0; i<num_objs; i++ )
			my_list.push( SimpleArtStripSlot(this,artwork_label) );
		set_slots( my_list );

		m_x=x+pad/2; m_y=y+pad/2;
		if ( width < height )
		{
			m_x_span=0;
			m_y_span=height;
			m_width=width-pad;
			m_height=height/m_objs.len()-pad;
		}
		else
		{
			m_x_span=width;
			m_y_span=0;
			m_width=width/m_objs.len()-pad;
			m_height=height-pad;
		}

		reset_progress();
	}
};

