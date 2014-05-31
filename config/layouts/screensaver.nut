///////////////////////////////////////////////////
//
// Attract-Mode Frontend - default screensaver script
//
///////////////////////////////////////////////////
class UserConfig {
	</ label="Movie Collage", help="Enable movie collage mode", options="Yes,No" />
	movie_collage="Yes";

	</ label="Image Collage", help="Enable 4x4 image collage mode", options="Yes,No" />
	image_collage="Yes";
}

fe.layout.width=100;
fe.layout.height=100;

function get_new_offset( obj )
{
	// try a few times to get a file
	for ( local i=0; i<6; i++ )
	{
		obj.index_offset = rand();

		if ( obj.file_name.len() > 0 )
			return true;
	}
	return false;
}

//
// Default mode - just play a video through once
//
class MovieMode
{
	MIN_TIME = 4000; // the minimum amount of time this mode should run for (in milliseconds)

	obj=0;
	start_time=0;

	constructor()
	{
		obj = fe.add_artwork( "", 0, 0, 100, 100 );
		obj.video_flags = Vid.NoAutoStart | Vid.NoLoop;
	}

	function init( ttime )
	{
		start_time=ttime;
		obj.visible = true;
		get_new_offset( obj );
		obj.video_playing = true;
	}

	function reset()
	{
		obj.visible = false;
	}

	// return true if mode should continue, false otherwise
	function check( ttime )
	{
		local elapsed = ttime - start_time;
		return (( obj.video_playing == true ) || ( elapsed <= MIN_TIME ));
	}

	function on_tick( ttime )
	{
		// nothing to do
	}

};

//
// 2x2 video display. Runs until first video ends
//
class MovieCollageMode
{
	objs = [];
	ignore = [];
	ignore_checked = false;
	chance = 25; // precentage chance that this mode is triggered

	constructor()
	{
		objs.append( _add_obj( 0, 0 ) );
		objs.append( _add_obj( 1, 0 ) );
		objs.append( _add_obj( 0, 1 ) );
		objs.append( _add_obj( 1, 1, Vid.NoAutoStart | Vid.NoLoop ) );

		for ( local i=0; i<objs.len(); i++ )
			ignore.append( false );
	}

	function _add_obj( x, y, vf=Vid.NoAudio | Vid.NoAutoStart | Vid.NoLoop )
	{
		local temp = fe.add_artwork( "", x*50, y*50, 50, 50 );
		temp.visible = false;
		temp.video_flags = vf;
		return temp;
	}

	function init( ttime )
	{
		foreach ( o in objs )
		{
			o.visible = true;
			get_new_offset( o );
			o.video_playing = true;
		}

		for ( local i=0; i<objs.len(); i++ )
			ignore[i] = false;

		ignore_checked = false;
	}

	function reset()
	{
		foreach ( o in objs )
			o.visible = false;
	}

	// return true if mode should continue, false otherwise
	function check( ttime )
	{
		if ( ttime < 4000 )
			return true;
		else if ( ignore_checked == false )
		{
			//
			// We ignore videos that stopped playing within the first
			// 4 seconds (images are captured by this too)
			//
			local all_are_ignored=true;
			for ( local i=0; i<objs.len(); i++ )
			{
				if ( objs[i].video_playing == false )
					ignore[i] = true;
				else
					all_are_ignored = false;
			}

			ignore_checked = true;
			return (!all_are_ignored);
		}

		for ( local i=0; i<objs.len(); i++ )
		{
			if (( objs[i].video_playing == false )
					&& ( ignore[i] == false ))
				return false;
		}
		return true;
	}

	function on_tick( ttime )
	{
		// nothing to do
	}
};

//
// 4x4 display of screen snapshots.  One shot is randomly chnaged every 100ms
//
class ImageCollageMode
{
	LENGTH = 12000; // length of time this mode should run (in milliseconds)

	objs = [];
	start_time=0;
	last_switch=0;
	chance = 15; // precentage chance that this mode is triggered

	constructor()
	{
		for ( local i=0; i<4; i++ )
		{
			for ( local j=0; j<4; j++ )
			{
				objs.append( fe.add_artwork( "", i * 25, j * 25, 25, 25 ) );
				objs.top().visible = false;
				objs.top().video_flags = Vid.ImagesOnly;
			}
		}
	}

	function init( ttime )
	{
		last_switch = start_time = ttime;
		foreach ( o in objs )
		{
			o.visible = true;
			get_new_offset( o );
		}
	}

	function reset()
	{
		foreach ( o in objs )
			o.visible = false;
	}

	// return true if mode should continue, false otherwise
	function check( ttime )
	{
		return (( ttime - start_time ) < LENGTH );
	}

	function on_tick( ttime )
	{
		if (( ttime - last_switch ) > 100 )
		{
			last_switch = ttime;
			objs[ rand() % objs.len() ].index_offset = rand();
		}
	}
}

//
// Movie mode is always on, turn on the others as configured by the user
//
local modes = [];
local default_mode = MovieMode();

local config = fe.get_config();

if ( config["movie_collage"] == "Yes" )
	modes.append( MovieCollageMode() );

if ( config["image_collage"] == "Yes" )
	modes.append( ImageCollageMode() );

local current_mode = default_mode;
local first_time = true;

fe.add_ticks_callback( "saver_tick" );

//
// saver_tick gets called repeatedly during screensaver.
// stime = number of milliseconds since screensaver began.
//
function saver_tick( ttime )
{
	if ( first_time ) // special case for initializing the very first mode
	{
		current_mode.init( ttime );
		first_time = false;
	}

	if ( current_mode.check( ttime ) == false )
	{
		//
		// If check returns false, we change the mode
		//
		current_mode.reset();

		current_mode = default_mode;
		foreach ( m in modes )
		{
			if ( ( rand() % 100 ) < m.chance )
			{
				current_mode = m;
				break;
			}
		}

		current_mode.init( ttime );
	}
	else
	{
		current_mode.on_tick( ttime );
	}
}
