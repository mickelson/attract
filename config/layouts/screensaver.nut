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

local PH = ScreenHeight / 4;
local PW = ScreenWidth / 4;
local group = [];

for ( local i=0; i<4; i++ )
{
	for ( local j=0; j<4; j++ )
	{
		group.append( fe.add_artwork( "", i * PW, j * PH, PW, PH ) );
		group.top().visible = false;
		group.top().movie_enabled = false;
	}
}

local show_group=false;
local group_switch=0;

fe.add_ticks_callback( "saver_tick" );

// saver_tick gets called repeatedly during screensaver.
// stime = number of milliseconds since screensaver began.
//
function saver_tick( stime )
{
	if ( stime - last_time > interval )
	{
		// show 16 image group every 9 switches
		if ( switch_count % 9 == 0 )
		{
			interval=movie_interval;

			foreach ( g in group )
			{
				g.visible = true;
				g.index_offset = rand();
			}

			art.visible = false;
			show_group=true;
			group_switch = stime + 100;
		}
		else
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
			art.index_offset = rand();

			foreach ( g in group )
				g.visible = false;

			art.visible = true;
			show_group = false;
		}
		last_time = stime;
		switch_count++;
	}

	if (( show_group ) && ( stime > group_switch ))
	{
		group[ ( rand() % group.len() ) ].index_offset = rand();
		group_switch = stime + 100;
	}
}
