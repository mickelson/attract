
///////////////////////////////////////////////////
function IS_ZIP(p)
{
	return ((p.len()>3)&&(p.slice(p.len()-4).tolower()==".zip"));
}

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
		local temp_list;

		if ( IS_ZIP( path ) )
		{
			temp_list = zip_get_dir( path );
		}
		else
		{
			local command = "/bin/sh";
			local param = "-c \"ls ";
			if ( OS == "Windows" )
			{
				command = "cmd";
				param = " /c dir /b \"";
			}
			param += path;
			param += "\"";

			// This will load the directory listing into m_work
			//
			fe.plugin_command( command, param, this, "_callback" );

			temp_list = split( _work, "\n" );
		}

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

	function _callback( m ) { _work += m; }
	_work="";
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
		if ( IS_ZIP( path ) )
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
