///////////////////////////////////////////////////
//
// Attract-Mode Frontend - AudioMode plugin
//
///////////////////////////////////////////////////
//
// Define the user-configurable options:
//
class UserConfig </ help="A plugin to play background audio" /> {

	</ label="Next Track",
		help="Button to press to jump to the next audio track",
		is_input=true,
		order=1 />
	skip_button="";

	</ label="Track Info",
		help="Button to press to display current audio track info",
		is_input=true,
		order=2 />
	info_button="";

	</ label="Source Directory",
		help="The full path to the directory containing the audio files to play",
		order=3 />
	dir="";
}

fe.load_module( "file" );

class AudioMode
{
	m_list = [];
	m_index = 0;
	m_input_block = false;
	m_work = "";
	m_config = {};
	m_display_text = null;
	m_display_time = 0;

	constructor()
	{
		m_display_text = fe.add_text( "",
			fe.layout.width/8,
			fe.layout.height/2,
			3*fe.layout.width/4,
			fe.layout.height/4 );

		m_display_text.word_wrap = true;
		m_display_text.charsize = fe.layout.height/24;
		m_display_text.set_rgb( 255, 255, 255 );
		m_display_text.set_bg_rgb( 20, 20, 20 );
		m_display_text.bg_alpha = 100;
		m_display_text.visible = false;

		fe.add_ticks_callback( this, "on_tick" );
		fe.add_transition_callback( this, "on_transition" );

		fe.ambient_sound.loop = false;

		m_config=fe.get_config();

		local dir = fe.path_expand( m_config[ "dir" ] );
		if ( dir.len() > 0 )
		{
			load_playlist( fe.path_expand( m_config[ "dir" ] ) );
			index_to_current();
		}
		else
		{
			print( "AudioMode plugin error: no source directory configured\n" );
		}
	}

	function _callback( msg )
	{
		m_work += msg;
	}

	function load_playlist( path )
	{
		local dir = DirectoryListing( path );

		// Shuffle the playlist
		m_list = [];
		while ( dir.results.len() > 0 )
		{
			local idx = rand() % dir.results.len();
			m_list.append( strip( dir.results[ idx ] ) );
			dir.results.remove( idx );
		}

		print( " - AudioMode plugin: found "
			+ m_list.len() + " file(s) in: " + path + "\n" );
	}

	function index_to_current()
	{
		// Check if current track is in our playlist,
		// and if so use that as start
		//
		m_index = 0;

		if ( !fe.ambient_sound.playing )
			return;

		for ( local idx=0; idx < m_list.len(); idx++ )
		{
			if ( fe.ambient_sound.file_name == m_list[idx] )
			{
				m_index = idx;
				return;
			}
		}
	}

	function change_track( offset )
	{
		if ( m_list.len() <= 0 )
			return;

		m_index += ( offset % m_list.len() );

		if ( m_index < 0 )
			m_index += m_list.len();
		if ( m_index >= m_list.len() )
			m_index -= m_list.len();

		// play the next track
		fe.ambient_sound.file_name
			= fe.path_expand( m_list[ m_index ] );

		if ( fe.ambient_sound.file_name.len() < 1 )
		{
			m_list.remove( m_index );
			return change_track( offset );
		}

		fe.ambient_sound.playing = true;
		m_display_text.msg = get_track_msg();
	}

	function get_track_msg()
	{
		if ( fe.ambient_sound.file_name.len() == 0 )
			return "";

		local title = fe.ambient_sound.get_metadata( "title" );
		if ( title.len() < 1 )
		{
			local n = split( fe.ambient_sound.file_name, "\\/" );
			if ( n.len() > 0 )
				return "Now Playing: " + n[ n.len()-1 ];
			else
				return "";
		}

		return "Now Playing: " + title + " - "
			+ fe.ambient_sound.get_metadata( "artist" ) + "\n\n"
			+ fe.ambient_sound.get_metadata( "album" );
	}

	function show_track_msg( flag, ttime )
	{
		if ( flag )
		{
			m_display_text.msg = get_track_msg();

 			if ( m_display_text.msg.len() > 0 )
			{
				m_display_time = ttime;
				m_display_text.visible = true;
				return;
			}
		}

		m_display_text.visible = false;
		m_display_time = 0;
	}

	function on_tick( ttime )
	{
		if ( fe.ambient_sound.playing == false )
			change_track( 1 );

		local next_sel = fe.get_input_state( m_config["skip_button"] );
		local info_sel = fe.get_input_state( m_config["info_button"] );
		if ( next_sel && !m_input_block )
		{
			change_track( 1 );
			m_input_block = true;
		}
		else if ( info_sel && !m_input_block )
		{
			show_track_msg( !m_display_text.visible, ttime );
			m_input_block = true;
		}
		else if ( m_input_block && !next_sel && !info_sel )
		{
			m_input_block = false;
		}

		if ( m_display_time > 0 )
		{
			local dtime = ttime - m_display_time;
			if ( dtime > 8000 )
				show_track_msg( false, ttime );
		}
	}

	function on_transition( ttype, var, ttime )
	{
		if ( ttype == Transition.FromGame )
			change_track( 1 );

		return false;
	}
}

// create an entry in the fe.plugin table in case anyone else wants to
// find this plugin
//
fe.plugin[ "AudioMode" ] <- AudioMode();
