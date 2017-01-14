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

	</ label="RGB Movie", help="Enable RGB movie mode", options="Yes,No", order=4 />
	rgb_movie="Yes";

	</ label="Overlay Artwork", help="Artwork to overlay on videos", options="wheel,flyer,marquee,None", order=5 />
	overlay_art="wheel";

	</ label="Play Sound", help="Play video sounds during screensaver", options="Yes,No", order=6 />
	sound="Yes";

	</ label="Preserve Aspect Ratio", help="Preserve the aspect ratio of screensaver snaps/videos", options="Yes,No", order=7 />
	preserve_ar="No";

	</ label="Blank Screen Time", help="Minutes before switching to blank screen (low power) mode.   Set this to 0 to disable.", order=8 />
	blank_time="120";

	</ label="Blank Screen Start Command", help="The command line to run when blank screen (low power) mode starts (to turn the monitor off, for example).", order=9 />
	blank_start_cmd="";

	</ label="Blank Screen Stop Command", help="The command line to run when blank screen (low power) mode stops (to turn the monitor back on, for example).", order=10 />
	blank_stop_cmd="";
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
		obj.video_playing = false;
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
		temp.video_playing = false;
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
//
//
class RGBMovieMode
{
        MIN_TIME = 4000; // the minimum amount of time this mode should run for (in milliseconds)
        chance = 25; // precentage chance that this mode is triggered

        _objL=0;
        _objL1=0;
        _obj=0;
        _objR=0;
        _objR1=0;

        logo=0;
        start_time=0;
        is_exclusive=false;

        cstep ={ red=0, green=0, blue=0 };

        last_colour=0;
        last_lcycle=1;
        last_rcycle=1;
        lcycle_ms=1000;
        rcycle_ms=1000;

        constructor()
        {
                _objL = fe.add_artwork( "", 0, 0, fe.layout.width/4, fe.layout.height );
                _objL1 = fe.add_clone( _objL );

                _objR = fe.add_clone( _objL );
                _objR1 = fe.add_clone( _objL );

                _obj = fe.add_clone( _objL );
                _obj.width = fe.layout.width/2;

                if ( config["sound"] == "No" )
                        _all_set( "video_flags", Vid.NoAudio | Vid.NoAutoStart | Vid.NoLoop );
                else
                        _all_set( "video_flags", Vid.NoAutoStart | Vid.NoLoop );

                if ( config["preserve_ar"] == "Yes" )
                        _all_set( "preserve_aspect_ratio", true );

                logo = ArtOverlay( 10, fe.layout.height - 250, 520, 240, 2 );

                _obj.set_pos( fe.layout.width/4, 0 );

		_all_set( "visible", false );
		_all_set( "video_playing", false );
        }

        function _all_set( tag, value )
        {
                _objL[tag] = value;
                _objL1[tag] = value;
                _obj[tag] = value;
                _objR[tag] = value;
                _objR1[tag] = value;
        }

        function _one_colour( label )
        {
                local temp = _obj[label] + cstep[label];
                if ( temp < 0 )
                {
                        cstep[label] *= -1;
                        temp = temp * -1;
                }
                else if ( temp > 255 )
                {
                        cstep[label] *= -1;
                        temp = 512 - temp;
                }

                _obj[label] = temp;
        }

        function _set_colour( ttime )
        {
                local elapsed = ( ttime - start_time ) / 50.0;
                if ( elapsed.tointeger() <= last_colour )
                        return;

                last_colour = elapsed.tointeger();

                _one_colour( "red" );
                _one_colour( "green" );
                _one_colour( "blue" );
        }

        function _reset_lpos()
        {
                _objL.set_pos( 0, 0 );
                _objL1.set_pos( 0, -fe.layout.height );
        }

        function _set_lpos( ttime )
        {
                local elapsed = ( ttime - start_time ) / lcycle_ms.tofloat();
                if (( elapsed.tointeger() < last_lcycle ))
                        return;
                else if ( elapsed.tointeger() > last_lcycle )
                {
                        // randomly stick in position reset
                        last_lcycle = elapsed.tointeger() + (rand()%3);
                        _reset_lpos();
                        return;
                }

                local step= fe.layout.height * ( elapsed - elapsed.tointeger() );

                _objL.set_pos( _objL.x, step );
                _objL1.set_pos( _objL1.x, -fe.layout.height + step );
        }

        function _reset_rpos()
        {
                _objR.set_pos( fe.layout.width - fe.layout.width/4, 0 );
                _objR1.set_pos( fe.layout.width - fe.layout.width/4, fe.layout.height );
        }

        function _set_rpos( ttime )
        {
                local elapsed = ( ttime - start_time ) / rcycle_ms.tofloat();
                if ( elapsed.tointeger() < last_rcycle )
                        return;
                else if ( elapsed.tointeger() > last_rcycle )
                {
                        // randomly stick in position reset
                        last_rcycle = elapsed.tointeger() + (rand()%3);
                        _reset_rpos();
                        return;
                }

                local step= fe.layout.height * ( elapsed - elapsed.tointeger() );

                _objR.set_pos( _objR.x, -step );
                _objR1.set_pos( _objR1.x, fe.layout.height-step );
        }

        function init( ttime )
        {
                start_time=ttime;
                _all_set( "visible", true );

                get_new_offset( _obj );
                _all_set( "index_offset", _obj.index_offset );

                _all_set( "subimg_width", _obj.texture_width / 4 );
                _obj.subimg_width = _obj.texture_width / 2;

                _obj.subimg_x = _obj.texture_width/4;
                _objR1.subimg_x = _objR.subimg_x = _objL.subimg_width + _obj.subimg_width;

                _all_set( "video_playing", true );

                cstep["red"] = rand()%20-10;
                cstep["green"] = rand()%20-10;
                cstep["blue"] = rand()%20-10;

                lcycle_ms = _get_cycle_ms();
                rcycle_ms = _get_cycle_ms();

                _reset_rpos();
                _reset_lpos();
                logo.init( _obj.index_offset, ttime, _obj.video_duration );
        }

        function _get_cycle_ms()
        {
                return 250 + rand()%1500;
        }

        function reset()
        {
                last_colour=0;
                last_lcycle=1;
                last_rcycle=1;

                _all_set( "visible", false );
                _all_set( "video_playing", false );

                logo.reset();
        }

        // return true if mode should continue, false otherwise
        function check( ttime )
        {
                local elapsed = ttime - start_time;
                return (( _obj.video_playing == true ) || ( elapsed <= MIN_TIME ));
        }

        function on_tick( ttime )
        {
                _set_colour( ttime );
                _set_lpos( ttime );
                _set_rpos( ttime );
                logo.on_tick( ttime );
        }

        function on_select()
        {
                // select the presently displayed game
                fe.list.index += _obj.index_offset;
        }
};

//
// Movie mode is always on, turn on the others as configured by the user
//
local modes = [];
local default_mode = MovieMode();

if ( config["movie_collage"] == "Yes" )
	modes.append( MovieCollageMode() );

if ( config["image_collage"] == "Yes" )
	modes.append( ImageCollageMode() );

if ( config["rgb_movie"] == "Yes" )
	modes.append( RGBMovieMode() );

if (( config["basic_movie"] == "No" ) && ( modes.len() > 0 ))
{
	default_mode = modes[0];
	modes.remove( 0 );
}

if ( modes.len() == 0 )
	default_mode.is_exclusive = true;

local current_mode = default_mode;
local first_time = true;

local blank_time = 0;
try { blank_time = config[ "blank_time" ].tointeger() * 60000; } catch (e) {};

local do_blank=false;

fe.add_ticks_callback( "saver_tick" );

//
// saver_tick gets called repeatedly during screensaver.
// ttime = number of milliseconds since screensaver began.
//
function saver_tick( ttime )
{
	if ( do_blank )
		return;

	if ( blank_time && ( ttime > blank_time ))
	{
		current_mode.reset();
		do_blank = true;

		if ( config[ "blank_start_cmd" ].len() > 0 )
			system( config[ "blank_start_cmd" ] );

		return;
	}

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

fe.add_transition_callback( "saver_transition" );
function saver_transition( ttype, var, ttime )
{
	if (( ttype == Transition.EndLayout ) && ( do_blank )
			&& ( config[ "blank_stop_cmd" ].len() > 0 ))
		system( config[ "blank_stop_cmd" ] );

	return false;
}

fe.add_signal_handler( "saver_signal_handler" );

function saver_signal_handler( sig )
{
	if ( sig == "select" )
		current_mode.on_select();

	return false;
}
