///////////////////////////////////////////////////
//
// Attract-Mode Frontend - History.dat plugin
//
// File utilities
//
///////////////////////////////////////////////////
const READ_BLOCK_SIZE=80960;
local next_ln_overflow=""; // used by the get_next_ln() function
local idx_path = FeConfigDirectory + "history.idx/";
local loaded_idx = {};

//
// Write a single line of output to f
//
function write_ln( f, line )
{
	local b = blob( line.len() );

	for (local i=0; i<line.len(); i++)
		b.writen( line[i], 'b' );

	f.writeblob( b );
}

//
// Get a single line of input from f
//
function get_next_ln( f )
{
	local ln = next_ln_overflow;
	next_ln_overflow="";
	while ( !f.eos() )
	{
		local char = f.readn( 'b' );
		if ( char == '\n' )
			return strip( ln );

		ln += char.tochar();
	}

	next_ln_overflow=ln;
	return "";
}

//
// Scan through f and return the next line starting with a "$"
//
function scan_for_ctrl_ln( f )
{
	local char;
	while ( !f.eos() )
	{
		char = f.readn( 'b' );
		if (( char == '\n' ) && ( !f.eos() ))
		{
			char = f.readn( 'b' );
			if ( char == '$' )
			{
				next_ln_overflow="$";
				return get_next_ln( f );
			}
		}
	}
	return "";
}

//
// Generate our history.dat index files
//
function generate_index( config )
{
	local histf_name = fe.path_expand( config[ "dat_path" ] );
	local histf;

	try
	{
		histf = file( histf_name, "rb" );
	}
	catch ( e )
	{
		return "Error opening file: " + histf_name;
	}

	local indices = {};

	local last_per=0;

	//
	// Get an index for all the entries in history.dat
	//
	while ( !histf.eos() )
	{
		local base_pos = histf.tell();
		local blb = histf.readblob( READ_BLOCK_SIZE );

		// Update the user with the percentage complete
		local percent = 100.0 * ( histf.tell().tofloat() / histf.len().tofloat() );
		if ( percent.tointeger() > last_per )
		{
			last_per = percent.tointeger();
			if ( fe.overlay.splash_message(
					"Generating index ("
					+ last_per + "%)" ) )
				break; // break loop if user cancels
		}
		
		while ( !blb.eos() )
		{
			local line = scan_for_ctrl_ln( blb );

			if ( line.len() < 5 ) // skips $bio, $end
				continue;

			local eq = line.find( "=", 1 );
			if ( eq != null )
			{
				local systems = split( line.slice(1,eq), "," );
				local roms = split( line.slice(eq+1), "," );

				foreach ( s in systems )
				{
					if ( !indices.rawin( s ) )
						indices[ s ] <- {};

					if ( config[ "index_clones" ] == "Yes" )
					{
						foreach ( r in roms )
							(indices[ s ])[ r ]
								<- ( base_pos + blb.tell() );
					}
					else if ( roms.len() > 0 )
					{
						(indices[ s ])[ roms[0] ]
							<- ( base_pos + blb.tell() );
					}
				}
			}
		}
	}

	//
	// Make sure the directory we are writing to exists...
	//
	system( "mkdir \"" + idx_path + "\"" );

	fe.overlay.splash_message( "Writing index file." );

	//
	// Now write an index file for each system encountered
	//
	foreach ( n,l in indices )
	{
		local idx = file( idx_path + n + ".idx", "w" );
		foreach ( rn,ri in l )
			write_ln( idx, rn + ";" + ri + "\n" );

		idx.close();
	}

	histf.close();

	return "Created index for " + indices.len()
		+ " systems in " + idx_path;
}

//
// Return the text for the history.dat entry after the given offset
//
function get_history_entry( offset, config )
{
	local histf = file( fe.path_expand( config[ "dat_path" ] ), "rb" );
	histf.seek( offset );

	local entry = "\n\n"; // a bit of space to start
	local open_entry = false;

	while ( !histf.eos() )
	{
		local blb = histf.readblob( READ_BLOCK_SIZE );
		while ( !blb.eos() )
		{
			local line = get_next_ln( blb );

			if ( !open_entry )
			{
				//
				// forward to the $bio tag
				//
				if (( line.len() < 1 )
						|| (  line != "$bio" ))
					continue;

				open_entry = true;
			}
			else
			{
				if ( line == "$end" )
				{
					entry += "\n\n";
					return entry;
				}
				else if (!(blb.eos() && ( line == "" )))
					entry += line + "\n";
			}
		}
	}

	return entry;
}

//
// Load the index for the given system if it is not already loaded
//
function load_index( sys )
{
	// check if system index already loaded
	//
	if ( loaded_idx.rawin( sys ) )
		return true;

	local idx;
	try
	{
		idx = file( idx_path + sys + ".idx", "r" );
	}
	catch( e )
	{
		loaded_idx[sys] <- null;
		return false;
	}

	loaded_idx[sys] <- {};

	while ( !idx.eos() )
	{
		local blb = idx.readblob( READ_BLOCK_SIZE );
		while ( !blb.eos() )
		{
			local line = get_next_ln( blb );
			local bits = split( line, ";" );
			if ( bits.len() > 0 )
				(loaded_idx[sys])[bits[0]] <- bits[1].tointeger();
		}
	}

	return true;
}

//
// Return the index the history.dat entry for the specified system and rom
//
function get_history_offset( sys, rom, alt, cloneof )
{
	foreach ( s in sys )
	{
		if (( load_index( s ) )
			&& ( loaded_idx[s] != null ))
		{
			if ( loaded_idx[s].rawin( rom ) )
				return (loaded_idx[s])[rom];
			else if ((alt.len() > 0 )
					&& ( loaded_idx[s].rawin( alt ) ))
				return (loaded_idx[s])[alt];
			else if ((cloneof.len() > 0 )
					&& ( loaded_idx[s].rawin( cloneof ) ))
				return (loaded_idx[s])[cloneof];
		}

	}

	if (( sys.len() != 1 ) || ( sys[0] != "info" ))
		return get_history_offset( ["info"], rom, alt, cloneof );

	return -1;
}
