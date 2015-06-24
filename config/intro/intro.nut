//////////////////////////////////////////////////
//
// Attract-Mode Frontend - Basic intro layout
//
///////////////////////////////////////////////////
class UserConfig
{
	</ label="Video", help="Video to play at startup (leave blank for none)." />
	video="intro.mp4";
}

// any signal will cause intro mode to exit
function end_mode() { fe.signal( "select" ); }

local my_config = fe.get_config();
if ( my_config["video"].len() == 0 )
	return end_mode();

local vid = fe.add_image( my_config["video"], 0, 0, ScreenWidth, ScreenHeight );
if ( vid.file_name.len() == 0 )
	return end_mode();

fe.add_ticks_callback( "intro_tick" );
function intro_tick( ttime )
{
	// check if the video has stopped yet
	//
	if ( vid.video_playing == false )
		end_mode();

	return false;
}
