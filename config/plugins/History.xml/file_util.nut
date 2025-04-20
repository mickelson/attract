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
local went_eos_mid_tag = false;
fe.load_module( "file-format.nut" );

function unescape( str )
{
	local escapes = [
		[ "&quot;", "\"" ],
		[ "&apos;", "'" ],
		[ "&amp;", "&" ],
		[ "&gt;", ">" ],
		[ "&lt;", "<" ]
	];

	local temp = "";
	local pos=0;
	local newpos=str.find( "&" );

	while ( newpos != null )
	{
		temp += str.slice( pos, newpos );

		local found=false;
		foreach ( e in escapes )
		{
			local lngth = e[0].len();
			if ( str.slice( newpos, newpos + e[0].len() ) == e[0] )
			{
				temp += e[1];
				pos = newpos + e[0].len();
				found = true;
				break;
			}
		}

		if ( !found )
		{
			temp += "&";
			pos = newpos+1;
		}

		newpos = str.find( "&", pos );
	}

	temp += str.slice( pos );
	return temp;
}

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

// scan past the next xml tag matching ctrl.  If ctrl is null, move beyond the next tag encountered
// returns name of last tag scanned past
function scan_past_tag( f, ctrl=null )
{
	next_ln_overflow="";
	while ( !f.eos() )
	{
		local ch = f.readn( 'b' );
		if ( ch == '<' )
		{
			local tag = ch.tochar();
			while ( !f.eos() )
			{
				ch = f.readn( 'b' );
				if ( ch == '>' )
					break;

				tag += ch.tochar();
			}

			if ( f.eos() )
			{
				went_eos_mid_tag = true;
				return "";
			}

			tag += ch.tochar();

			// Skip past trailing whitespace
			while ( !f.eos() )
			{
				local current=f.tell();
				ch = f.readn( 'b' );
				if (( ch != '\n' ) && ( ch != '>' ) && ( ch != ' ' ))
				{
					f.seek( current );
					break;
				}
			}

			if (( ctrl == null ) || ( ctrl == tag ))
				return tag;
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
	local last_entry=0;

	//
	// Get an index for all the entries in history.dat
	//
	while ( !histf.eos() )
	{
		if ( went_eos_mid_tag )
		{
			// reposition so we start right at the <entry> tag we last encountered
			went_eos_mid_tag = false;
			histf.seek( last_entry );

		}

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
			// ***HANDLE:***
			//	<entry>
			//  		<systems>
			//			<system name="SYSTEM" game="yes" />
			//		</systems>

			// ***OR***

			//	<entry>
			//  		<software>
			//			<item list="SYSTEM" name="ROM" game="yes" />
			//			<item list="SYSTEM" name="ROM" game="yes" />
			//  		</software>

			// ***FOLLOWED BY***

			//  		<text>TEXT TO DISPLAY...
			//  ... AND IT CONTINUES...
			//  ... LAST LINE
			//  		</text>
			//	</entry>
			local ctr = scan_past_tag( blb, "<entry>" ); // jump to next <entry>
			if ( !went_eos_mid_tag )
				last_entry = base_pos + blb.tell() - 7; // location in file of this <entry>
			else
				break;

			local entries = [];
			local loc=0;

			while ( !blb.eos() && ( ctr != "</entry>") )
			{
				ctr = scan_past_tag( blb );
				if (( ctr == "<systems>" ) || ( ctr == "<software>" ))
				{
					ctr = scan_past_tag( blb );
					while ( !blb.eos() && ( ctr != "</systems>" ) && (  ctr != "</software>" ) )
					{
						local xmlstr = xml.load( ctr );

						local skip=false;
						local system="info";
						local rom="";
						foreach ( name,val in xmlstr.attr )
						{
							switch (name)
							{
							case "game":
								if ( val == "no" ) skip=true;
								break;
							case "list":
								system = val;
								break;
							case "name":
								rom = val;
								break;
							};
						}

						if ( !skip && ( rom.len() > 0 ) )
							entries.push( [ system, rom ] );

						ctr = scan_past_tag( blb );
					}
				}
				else if ( ctr == "<text>" )
					loc = base_pos + blb.tell();
			}

			foreach ( e in entries )
			{
				if (!indices.rawin( e[0] ))
					indices[ e[0] ] <- {};

				(indices[ e[0] ])[ e[1] ] <- loc;
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

	while ( !histf.eos() )
	{
		local blb = histf.readblob( READ_BLOCK_SIZE );
		while ( !blb.eos() )
		{
			local line = get_next_ln( blb );
			if ( line == "</text>" )
			{
				entry += "\n\n";
				return entry;
			}
			else if (!(blb.eos() && ( line == "" )))
				entry += unescape( line ) + "\n";
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

	if (( sys.len() < 1 ) || ( sys[0] != "info" ))
		return get_history_offset( ["info"], rom, alt, cloneof );

	return -1;
}
