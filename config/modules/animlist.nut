///////////////////////////////////////////////////
//
// Attract-Mode Frontend - "animlist" module
//
// Provides an Animated list box that is a drop-in
// replacement for Attract-Mode's built-in listbox
//
///////////////////////////////////////////////////
fe.load_module( "conveyor" );

///////////////////////////////////////////////////
class AnimListEntry extends ConveyorSlot
{
	constructor( p )
	{
		_p=p;
		_t = fe.add_text( "[Title]",
			_p._x,
			0,
			_p._w,
			_p._h/(_p._num-1) );
	}

	function on_progress(prog,var)
	{
		_t.y=_p._y+prog*_p._h;
	}

	function _fc()
	{
		if ( get_base_index_offset() ==  0 )
			foreach ( i,o in _p._sc )
				_t[i] = o;
		else
			foreach ( i,o in _p._c )
				_t[i] = o;
	}

	function swap( o ) {}
	function set_index_offset( io ) { _t.index_offset=io; }
	function reset_index_offset() { set_index_offset( m_base_io ); }
	function _set( idx, val ) { _t[idx]=val; return val; }
	_p=null; _t=null;
}

///////////////////////////////////////////////////
//
// AnimList - Animated Listbox
//
///////////////////////////////////////////////////
//
// A drop-in replacement for Attract-Mode's built-in
// listbox, with animation of the list
//
// Usage:
//
//    local my_listbox = AnimatedList( x, y, width, height );
//    my_listbox.rows = 13; 
//
// Alternatively, the number of rows can be specified
// as the fifth parameter in the class constructor:
//
//    local my_listbox = AnimatedList( x, y, width, height, 13 );
//
///////////////////////////////////////////////////
class AnimList extends Conveyor
{
	constructor( x, y, w, h, num=13 )
	{
		base.constructor();

		_x=x; _y=y; _w=w; _h=h;
		_c={}; _sc={};

		// set up default selection colours
		// (yellow text, blue bg)
		//
		_sc["red"]<-255
		_sc["green"]<-255;
		_sc["blue"]<-0;
		_sc["bg_red"]<-0;
		_sc["bg_green"]<-0;
		_sc["bg_blue"]<-255;
		_sc["bg_alpha"]<-255;

		_create_entries( num );
		transition_swap_point = 1.0;
		transition_ms = 80;
	}

	function on_transition( ttype, var, ttime )
	{
		if (( ttype == Transition.FromOldSelection )
			|| ( ttype == Transition.ToNewList ))
		{
			foreach ( o in m_objs )
			{
				local rom_abs = fe.list.index
					+ o.get_base_index_offset();
				if (( rom_abs < 0 )
						|| ( rom_abs >= fe.list.size ))
					o._t.visible = false;
				else
					o._t.visible = true;
			}
		}

		return base.on_transition( ttype, var, ttime );
	}

	function _create_entries( num )
	{
		_num=num;

		local sl=array( num );

		for ( local i=0; i<num; i++ )
		{
			if ( m_objs.len() > i )
				sl[i] = m_objs[i]; // we recycle
			else
				sl[i] = AnimListEntry( this );
		}

		local orphan = m_objs.len() - num;
		if ( orphan > 0 )
			print( "AnimatedList: Warning, orphaned "
				+ orphan + " texts\n" );

		set_slots( sl );
		_fc();
	}

	function _fc()
	{
		foreach ( o in m_objs )
			o._fc();
	}

	function set_rgb( r, g, b )
	{ _c["red"]<-r; _c["green"]<-g; _c["blue"]<-b; _fc(); }

	function set_bg_rgb( r, g, b )
	{ _c["bg_red"]<-r; _c["bg_green"]<-g; _c["bg_blue"]<-b; _fc(); }

	function set_sel_rgb( r, g, b )
	{ _sc["red"]<-r; _sc["green"]<-g; _sc["blue"]<-b; _fc(); }

	function set_selbg_rgb( r, g, b )
	{ _sc["bg_red"]<-r; _sc["bg_green"]<-g; _sc["bg_blue"]<-b; _fc(); }

	_c = null; // regular colours
	_sc = null; // sel colours

	// map sel colours -> colours
	static _SCM = {
		sel_red="red",
		sel_green="green",
		sel_blue="blue",
		sel_alpha="alpha",
		sel_style="style",
		selbg_red="bg_red",
		selbg_green="bg_green",
		selbg_blue="bg_blue",
		selbg_alpha="bg_alpha"
	};

	function _set( idx, val )
	{
		if ( idx == "format_string" )
			idx = "msg";
		else if ( idx in [ "red", "green", "blue", "alpha",
			"style", "bg_red", "bg_green", "bg_blue",
			"bg_alpha" ] )
		{
			_c[idx]<-val;
			_fc();
			return val;
		}
		if ( idx in _SCM )
		{
			_sc[_SCM[idx]]<-val;
			_fc();
			return val;
		}
		else if ( idx == "rows" )
		{
			_create_entries( val )
			return val;
		}

		foreach ( o in m_objs )
			o[idx] = val;

		return val;
	}

	_x=0; _y=0; _w=0; _h=0; _num=0;
}
