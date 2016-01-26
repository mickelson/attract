
///////////////////////////////////////////////////
function IS_ARCHIVE(p)
{
	local ar_types = [
		".zip",
		".rar",
		".7z",
		".tar.gz",
		".tgz",
		".tar.bz2",
		".tbz2",
		".tar" ];

	foreach ( a in ar_types )
	{
		if (( p.len() >= a.len() )
				&& ( p.slice( p.len() - a.len() ).tolower() == a ))
			return true;
	}

	return false;
}

function IS_ZIP(p) { return IS_ARCHIVE(p); };

///////////////////////////////////////////////////
//
// "DirectoryListing" class
//
// This class get a list of the files in the specified path
// The list is put in the "results" member array
//
class DirectoryListing
{
	constructor( path, fullnames=true )
	{
		results = [];
		local temp_list;

		//
		// It's undocumented, but zip_get_dir() simply returns the
		// directory contents if path points to a directory
		//
		temp_list = zip_get_dir( path );

		if ( fullnames )
		{
			if (( path[ path.len()-1 ] != '/' )
					&& ( path[ path.len()-1 ] != '\\' ))
				path += "/";

			foreach ( t in temp_list )
				results.append( path + strip( t ) );
		}
		else
		{
			foreach ( t in temp_list )
				results.append( strip( t ) );
		}
	}

	results=[];
};

///////////////////////////////////////////////////
//
// "ReadTextFile" class
//
// This class reads lines from a text file
//
// If the first argument to the constructor is a zip file, it will
// read the file with the second argument name from inside the zip
//
class ReadTextFile
{
	constructor( path, filename="" )
	{
		//
		// Handle zip files
		//
		if ( IS_ARCHIVE( path ) )
		{
			try
			{
				_blb = zip_extract_file( path, filename );
			}
			catch ( e )
			{
				print( "Error opening zip file for reading: "
					+ path + ", " + filename + ": "
					+ e + "\n" );
			}
		}
		else
		{
			try
			{
				_f = file( path + filename, "r" );
				_blb = _f.readblob( _readsize );
			}
			catch ( e )
			{
				print( "Error opening file for reading: "
					+ path + filename + ": " + e + "\n" );
			}
		}
	}

	//
	// Read a line from the text file, stripping whitespace
	//
	function read_line()
	{
		local line="";
		local char;

		while ( !eos() )
		{
			if ( _blb.eos() && _f && !_f.eos() )
				_blb = _f.readblob( _readsize );

			while ( !_blb.eos() )
			{
				char = _blb.readn( 'b' );
				if ( char == '\n' )
					return strip( line );

				line += char.tochar();
			}
		}

		return line;
	}

	function eos()
	{
		if ( !_blb )
			return true;
		else if ( !_f )
			return ( _blb.eos() );

		return ( _blb.eos() && _f.eos() );
	}

	_f=null;
	_blb=null;
	_next_ln="";
	_readsize=1024;
};

///////////////////////////////////////////////////
//
// "WriteTextFile" class
//
// This class writes lines to a text file
//
class WriteTextFile
{
	constructor( filename )
	{
		try
		{
			_f = file( filename, "w" );
		}
		catch ( e )
		{
			print( "Error opening file for writing: "
				+ filename + "\n" );
		}
	}

	function write_line( line )
	{
		local b = blob( line.len() );

		for (local i=0; i<line.len(); i++)
			b.writen( line[i], 'b' );

		_f.writeblob( b );
	}

	_f=null;
};
