///////////////////////////////////////////////////
//
// Attract-Mode Frontend - default screensaver script
//
///////////////////////////////////////////////////
class UserConfig {
	</ label="Basic Movie", help="Enable basic movie mode", options="Yes,No", order=1 />
	basic_movie="Yes";

	</ label="Movie Collage", help="Enable 2x2 movie collage mode", options="Yes,No", order=2 />
	movie_collage="Yes";

	</ label="Image Collage", help="Enable 4x4 image collage mode", options="Yes,No", order=3 />
	image_collage="Yes";

	</ label="Overlay Artwork", help="Artwork to overlay on videos", options="wheel,flyer,marquee,None", order=4 />
	overlay_art="wheel";

	</ label="Play Sound", help="Play video sounds during screensaver", options="Yes,No", order=5 />
	sound="Yes";

	</ label="Preserve Aspect Ratio", help="Preserve the aspect ratio of screensaver snaps/videos", options="Yes,No", order=6 />
	preserve_ar="No";
}

local actual_rotation = (fe.layout.base_rotation + fe.layout.toggle_rotation)%4;
if (( actual_rotation == RotateScreen.Left )
	|| ( actual_rotation == RotateScreen.Right ))
{
	// Vertical orientation
	//
	fe.layout.height = 1024;
	fe.layout.width = 1024 * ScreenHeight / ScreenWidth;
}
else
{
	// Horizontal orientation
	//
	fe.layout.width = 1024;
	fe.layout.height = 1024 * ScreenHeight / ScreenWidth;
}

local config = fe.get_config();

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
// Container for a wheel image w/ shadow effect
//
class ArtOverlay
{
	logo=0;
	logo_shadow=0;
	in_time=0;
	out_time=0;

	constructor( x, y, width, height, shadow_offset )
	{
		if ( config["overlay_art"] != "None" )
		{
			logo_shadow = fe.add_artwork(
				config["overlay_art"],
				x + shadow_offset,
				y + shadow_offset,
				width,
				height );

			logo_shadow.preserve_aspect_ratio = true;

			logo = fe.add_clone( logo_shadow );
			logo.set_pos( x, y );

			logo_shadow.set_rgb( 0, 0, 0 );
			logo_shadow.visible = logo.visible = false;
		}
	}

	function init( index_offset, ttime, duration )
	{
		if ( config["overlay_art"] != "None" )
		{
			logo.index_offset = index_offset;
			logo.visible = logo_shadow.visible = true;
			logo.alpha = logo_shadow.alpha = 0;
			in_time = ttime + 1000; // start fade in one second in

			if ( config["overlay_art"] == "wheel" )
			{
				// start fade out 2 seconds before video ends
				// for wheels
				out_time = ttime + duration - 2000;
			}
			else
			{
				// otherwise just flash for 4 seconds
				out_time = ttime + 4000;
			}
		}
	}

	function reset()
	{
		if ( config["overlay_art"] != "None" )
		{
			logo.visible = logo_shadow.visible = false;
		}
	}

	function on_tick( ttime )
	{
		if (( config["overlay_art"] != "None" )
			&& ( logo.visible ))
		{
			if ( ttime > out_time + 1000 )
			{
				logo.visible = logo_shadow.visible = false;
			}
			else if ( ttime > out_time )
			{
				logo.alpha = logo_shadow.alpha = 255 - ( 255 * ( ttime - out_time ) / 1000.0 );
			}
			else if ( ( ttime < in_time + 1000 ) && ( ttime > in_time ) )
			{
				logo.alpha = logo_shadow.alpha = ( 255 * ( ttime - in_time ) / 1000.0 );
			}
		}
	}
};

//
// Default mode - just play a video through once
//
class MovieMode
{
	MIN_TIME = 4000; // the minimum amount of time this mode should run for (in milliseconds)
	obj=0;
	logo=0;
	start_time=0;
	is_exclusive=false;

	constructor()
	{
		obj = fe.add_artwork( "", 0, 0, fe.layout.width, fe.layout.height );
		if ( config["sound"] == "No" )
			obj.video_flags = Vid.NoAudio | Vid.NoAutoStart | Vid.NoLoop;
		else
			obj.video_flags = Vid.NoAutoStart | Vid.NoLoop;

		if ( config["preserve_ar"] == "Yes" )
			obj.preserve_aspect_ratio = true;

		logo = ArtOverlay( 10, fe.layout.height - 250, 520, 240, 2 );
	}

	function init( ttime )
	{
		start_time=ttime;
		obj.visible = true;
		get_new_offset( obj );
		obj.video_playing = true;

		logo.init( obj.index_offset, ttime, obj.video_duration );
	}

	function reset()
	{
		obj.visible = false;
		logo.reset();
	}

	// return true if mode should continue, false otherwise
	function check( ttime )
	{
		local elapsed = ttime - start_time;
		return (( obj.video_playing == true ) || ( elapsed <= MIN_TIME ));
	}

	function on_tick( ttime )
	{
		logo.on_tick( ttime );
	}

	function on_select()
	{
		// select the presently displayed game
		fe.list.index += obj.index_offset;
	}
};

//
// 2x2 video display. Runs until first video ends
//
class MovieCollageMode
{
	objs = [];
	logos = [];
	ignore = [];
	ignore_checked = false;
	chance = 25; // precentage chance that this mode is triggered
	is_exclusive=false;

	constructor()
	{
		objs.append( _add_obj( 0, 0 ) );
		objs.append( _add_obj( 1, 0 ) );
		objs.append( _add_obj( 0, 1 ) );
		if ( config["sound"] == "No" )
			objs.append( _add_obj( 1, 1 ) );
		else
			objs.append( _add_obj( 1, 1, Vid.NoAutoStart | Vid.NoLoop ) );

		for ( local i=0; i<objs.len(); i++ )
		{
			ignore.append( false );
			logos.append( ArtOverlay( objs[i].x + 5, objs[i].y + objs[i].height - 125, 260, 120, 1 ) );
		}
	}

	function _add_obj( x, y, vf=Vid.NoAudio | Vid.NoAutoStart | Vid.NoLoop )
	{
		local temp = fe.add_artwork( "", x*fe.layout.width/2, y*fe.layout.height/2, fe.layout.width/2, fe.layout.height/2 );
		temp.visible = false;
		temp.video_flags = vf;

		if ( config["preserve_ar"] == "Yes" )
			temp.preserve_aspect_ratio = true;

		return temp;
	}

	function obj_init( idx, ttime )
	{
		objs[idx].visible = true;
		get_new_offset( objs[idx] );

		// try not to duplicate videos
		if ( fe.list.size > 7 )
		{
			for ( local j=0; j<4; j++ )
			{
				if (( j != idx ) && ( objs[idx].file_name == objs[j].file_name ))
				{
					get_new_offset( objs[idx] );
					break;
				}
			}
		}

		objs[idx].video_playing = true;

		logos[idx].init( objs[idx].index_offset, ttime, objs[idx].video_duration );
	}

	function init( ttime )
	{
		for ( local i=0; i<objs.len(); i++ )
		{
			obj_init( i, ttime );
			ignore[i] = false;
		}

		ignore_checked = false;
	}

	function reset()
	{
		foreach ( o in objs )
		{
			o.visible = false;
			o.video_playing = false;
		}

		foreach ( l in logos )
			l.reset();
	}

	// return true if mode should continue, false otherwise
	function check( ttime )
	{
		if (( ttime < 4000 ) || ( is_exclusive ))
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
				return ( ( rand() % 2 ) == 0 ); // 50/50 chance of leaving mode
		}
		return true;
	}

	function on_tick( ttime )
	{
		foreach ( l in logos )
			l.on_tick( ttime );

		for ( local i=0; i<objs.len(); i++ )
		{
			if ( objs[i].video_playing == false )
				obj_init( i, ttime );	
		}
	}

	function on_select()
	{
		// randomly select one of the presently displayed games
		fe.list.index += objs[ rand() % 4 ].index_offset;
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
	is_exclusive=false;

	constructor()
	{
		local width = fe.layout.width / 4;
		local height = fe.layout.height / 4;
		for ( local i=0; i<4; i++ )
		{
			for ( local j=0; j<4; j++ )
			{
				objs.append( fe.add_artwork( "", i * width, j * height, width, height ) );
				objs.top().visible = false;
				objs.top().video_flags = Vid.ImagesOnly;

				if ( config["preserve_ar"] == "Yes" )
					objs.top().preserve_aspect_ratio = true;
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
		if ( is_exclusive )
			return true;
		else
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

	function on_select()
	{
		// randomly select one of the presently displayed games
		fe.list.index += objs[ rand() % 16 ].index_offset;
	}
}

//
// Movie mode is always on, turn on the others as configured by the user
//
local modes = [];
local default_mode = MovieMode();

if ( config["movie_collage"] == "Yes" )
	modes.append( MovieCollageMode() );

if ( config["image_collage"] == "Yes" )
	modes.append( ImageCollageMode() );

if (( config["basic_movie"] == "No" ) && ( modes.len() > 0 ))
{
	default_mode = modes[0];
	modes.remove( 0 );
}

if ( modes.len() == 0 )
	default_mode.is_exclusive = true;

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

fe.add_signal_handler( "saver_signal_handler" );

function saver_signal_handler( sig )
{
	if ( sig == "select" )
		current_mode.on_select();

	return false;
}
