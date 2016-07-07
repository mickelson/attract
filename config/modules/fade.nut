/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - Fade Module
//
/////////////////////////////////////////////////////////
//
// FadeArt - creates an artwork that does a smooth fade from existing to
// new when it changes images.  This artwork also cycles the image that is
// displayed when multiple artworks are available for a particular game.
//
class FadeArt
{
	index_offset=0;		// the index_offset of the game to display
	filter_offset=0;	// the filter_offset of the game to display

	//
	// The interval (in ms) to show an art for if there are multiple
	// artworks available.  Set to 0 to not switch until a new game is
	// selected
	//
	interval=15000;

	//
	// Time (in ms) for the "short" fade effect that occurs when new
	// games are selected
	//
	short_fade=500;		

	//
	// Time (in ms) for the "long" fade effect that occurs when switching
	// between available artworks for the current game
	//
	long_fade=2500;

	//
	// Constructor for a FadeArt object.
	//
	// label - label of the artwork to show
	// x - x position for top left corner
	// y - y position for top left corner
	// width - width
	// height - height
	// target - target surface for the object.  Defaults to the main
	//    layout surface (fe).  Could also be one of the secondary
	//    monitors on a multi-monitor setup or a surface object
	//
	constructor( label, x, y, width, height, target=::fe )
	{
		_back = target.add_image( "", x, y, width, height );
		_front = target.add_image( "", x, y, width, height );
		_label = label;

		fe.add_transition_callback( this, "on_transition" );
		fe.add_ticks_callback( this, "on_tick" );
	}

	function on_transition( ttype, var, ttime )
	{
		if (( ttype == Transition.EndNavigation )
				|| ( ttype == Transition.ToNewList ))
		{
			if ( _in_fade )
				flip();

			_trigger_load=true;
		}

		return false;
	}

	function on_tick( ttime )
	{
		if ( _trigger_load ||
			((interval>0)&&(ttime-_last_switch>interval)) )
		{
			local test="";

			local tmp = fe.get_art( _label,
					index_offset,
					filter_offset,
					Art.FullList );

			if ( tmp.len() > 0 )
			{
				local arr = split( tmp, ";" );
				test = arr[ rand()%arr.len() ];
			}

			// Start changed: ArcadeBliss to show default artwork from the layout dir
			if (test=="")
			{
				test = fe.script_dir + _label +".png";
			}
			// End changed: ArcadeBliss

			if ( test != _back.file_name )
			{
				_front.file_name=test;
				_in_fade=true;
			}
			_last_switch=ttime;
			_front.alpha=0;
			_back.alpha=255;

			_fade_time=_trigger_load ? short_fade : long_fade;
			_trigger_load=false;
		}

		if ( _in_fade )
		{
			local new_alpha = 255*(ttime-_last_switch)/_fade_time;
			if ( new_alpha > 255 )
				flip();
			else
			{
				_front.alpha = new_alpha;
				_back.alpha = 255-new_alpha;
			}
		}
	}

	function flip()
	{
		_back.swap( _front );
		_front.file_name = "";
		_front.alpha = 0;
		_back.alpha = 255;

		_in_fade=false;
	}

	//
	function _set( idx, val )
	{
		_front[idx]=val;
		_back[idx]=val;
		return val;
	}

	//
	function set_rgb(r,g,b) {_front.set_rgb(r,g,b);_back.set_rgb(r,g,b);}
	function set_pos(x,y) {_front.set_pos(x,y);_back.set_pos(x,y);}

	_front=null;
	_back=null;
	_label="";
	_last_switch=0;
	_in_fade=false;
	_fade_time=0;
	_trigger_load=true;
};
