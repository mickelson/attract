// Attract-Mode Frontend - default screensaver script
//
const screen_interval=3000; // screenshot display time (in ms)
const movie_interval=20000; // movie display time

local last_time=0;
local switch_count=1;
local interval=screen_interval;

// Initialize artwork resource
//
local art = fe.add_artwork( "", 0, 0, ScreenWidth, ScreenHeight );
art.movie_enabled = false;
art.index_offset = rand();

fe.add_ticks_callback( "saver_tick" );

// saver_tick gets called repeatedly during screensaver.
// stime = number of milliseconds since screensaver began.
//
function saver_tick( stime )
{
	if ( stime - last_time > interval )
	{
		// show movie every 10 switches
		if ( switch_count % 10 == 0 )
		{
			interval=movie_interval;
			art.movie_enabled = true;
		}
		else
		{
			interval=screen_interval;
			art.movie_enabled = false;
		}

		last_time = stime;
		switch_count++;
		art.index_offset = rand();
	}
}
