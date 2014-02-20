///////////////////////////////////////////////////
//
// Attract-Mode Frontend - default screensaver script
//
///////////////////////////////////////////////////
class UserConfig {
	</ label="Collage Mode", help="Enable 4x4 collage mode", options="Yes,No" />
	a_collage="Yes";

	</ label="Screen Shot Mode", help="Enable screen shot mode", options="Yes,No" />
	b_screen="Yes";

	</ label="Movie Mode", help="Enable movie mode", options="Yes,No" />
	c_movie="Yes";

	</ label="Mode Interval", help="The amount of time to spend in each mode (in seconds)" />
	d_interval="21";
}

enum Mode {
	Screen,
	Movie,
	Collage
}

local config = fe.get_config();
local cycle = [];

if ( config["b_screen"] == "Yes" )
	cycle.append( Mode.Screen );

if ( config["c_movie"] == "Yes" )
	cycle.append( Mode.Movie );

if ( config["a_collage"] == "Yes" )
	cycle.append( Mode.Collage );

if ( cycle.len() < 1 )
	cycle.append( Mode.Screen );

local mode_interval = 21000;
try {
	local temp = config[ "d_interval" ].tointeger();
	mode_interval = temp * 1000;
}
catch ( e ) {
}

local screen_interval = 3000;
local collage_interval = 100;

local current_mode= cycle.len() - 1;
local mode_count=0;
local screen_count=1;

// Initialize artwork resources
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

fe.add_ticks_callback( "saver_tick" );

// saver_tick gets called repeatedly during screensaver.
// stime = number of milliseconds since screensaver began.
//
function saver_tick( stime )
{
	if ( stime > mode_count * mode_interval )
	{
		// change mode
		//
		mode_count++;
		current_mode++;
		if ( current_mode >= cycle.len() )
			current_mode = 0;

		if ( cycle[current_mode] == Mode.Collage )
		{
			foreach ( g in group )
			{
				g.visible = true;
				g.index_offset = rand();
			}

			art.visible = false;
			screen_count = 1;
		}
		else
		{
			foreach ( g in group )
			{
				g.visible = false;
			}

			art.visible = true;
			art.index_offset = rand();

			if ( cycle[current_mode] == Mode.Movie )
			{
				art.movie_enabled = true;
			}
			else
			{
				art.movie_enabled = false;
				screen_count = 1;
			}
		}
	}
	else
	{
		local m_time = stime - ( ( mode_count - 1 ) * mode_interval );
		switch ( cycle[current_mode] )
		{
		case Mode.Screen:
			if ( m_time > ( screen_count * screen_interval ))
			{
				// change image
				art.index_offset = rand();
				screen_count++;
			}
			break;

		case Mode.Movie:
			// Do nothing
			break;

		case Mode.Collage:
			if ( m_time > ( screen_count * collage_interval ))
			{
				// change image
				group[ ( rand() % group.len() ) ].index_offset = rand();
				screen_count++;
			}
			break;
		}
	}
}
