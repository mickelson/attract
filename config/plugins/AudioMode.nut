///////////////////////////////////////////////////
//
// Attract-Mode Frontend - AudioMode plugin
//
///////////////////////////////////////////////////
//
// Define the user-configurable options:
//
class UserConfig </ help="A plugin to play background audio" /> {

	</ label="Next Track", help="Button to press to jump to the next audio track", is_input=true, order=1 />
	skip_button="";

	</ label="Source Directory", help="The full path to the directory containing the audio files to play", order=2 />
	dir="";
}

class AudioMode
{
	m_list = [];
	m_index = 0;
	m_input_block = false;
	m_work = "";
	m_config = {};

	constructor()
	{
		fe.add_ticks_callback( this, "on_tick" );
		fe.add_transition_callback( this, "on_transition" );

		fe.ambient_sound.loop = false;

		m_config=fe.get_config();

		load_playlist( fe.path_expand( m_config[ "dir" ] ) );
		index_to_current();
	}

	function _callback( msg )
	{
		m_work += msg;
	}

	function load_playlist( dir )
	{
		// Read the play directory
		//
		local command = "/bin/sh";
		local param = "/c ls ";
		if ( OS == "Windows" )
		{
			command = "cmd";
			param = " /c dir /b ";
		}
		param += dir;

		m_work = "";

		// This will load the directory listing into m_work
		//
		fe.plugin_command( command, param, this, "_callback" );

		// Create a playlist from the directory contents
		//
		local temp_list = split( m_work, "\n" );
		m_work = "";

		if (( dir[ dir.len() - 1 ] != '/' )
				&& ( dir[ dir.len() -1 ] != '\\' ))
			dir += "/";

		// Shuffle the playlist
		m_list = [];
		while ( temp_list.len() > 0 )
		{
			local idx = rand() % temp_list.len();
			m_list.append( dir + strip( temp_list[ idx ] ) );
			temp_list.remove( idx );
			
		}
	}

	function index_to_current()
	{
		// check if current track is in our playlist, and if so use that as start
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

		fe.ambient_sound.playing = true;
	}

	function on_tick( ttime )
	{
		if ( fe.ambient_sound.playing == false )
			change_track( 1 );

		local next_sel = fe.get_input_state( m_config[ "skip_button" ] );
		if ( next_sel && !m_input_block )
		{
			change_track( 1 );
			m_input_block = true;
		}
		else if ( !next_sel && m_input_block )
		{
			m_input_block = false;
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
