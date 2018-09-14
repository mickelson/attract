////////////////////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - Emulator detection script (MAME)
//
// Copyright (c) 2017 Andrew Mickelson
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
////////////////////////////////////////////////////////////////////////
fe.load_module( "file" );

// Our base default values for mame
//
mame_emu <-
{
	"name"   : "mame",
	"exe"    : "mame",
	"args"   : "[name]",
	"rompath": "$HOME/mame/roms/",
	"exts"   : ".zip;.7z",
	"system" : "Arcade",
	"source" : "listxml",
};

// We only keep this at the nes settings if no non-arcade systems are
// detected and we are writing a "mess-style" template
//
console_emu <-
{
	"name"   : "mame-nes",
	"exe"    : "mame",
	"args"   : "[system] -cart \"[romfilename]\"",
	"rompath": "$HOME/mame/roms/nes/",
	"exts"   : ".zip;.7z",
	"system" : "nes;Nintendo Entertainment System (NES)",
	"source" : "listsoftware"
};

////////////////////////////////////////////////////////////////////////
//
// Paths to check for the MAME executable
//
////////////////////////////////////////////////////////////////////////
search_paths <- {

	"Linux" : [
		"/usr/games",
		"/usr/bin",
		"/usr/local/games",
		"/usr/local/bin",
		"$HOME/mame",
		"$HOME/sdlmame",
		"./mame",
		"../mame",
		],

	"Windows" : [
		"$HOME/mame/",
		"./mame/",
		"../mame/",
		],

	"OSX" : [
		"$HOME/mame",
		"$HOME/sdlmame",
		"./mame",
		"../mame",
		],
};

////////////////////////////////////////////////////////////////////////
//
// Potential names for the mame executable
//
////////////////////////////////////////////////////////////////////////
search_names <- {
	"Linux" : [
		"mame",
		"mame64",
		"mame32",
		"sdlmame",
		"groovymame",
		],

	"Windows" : [
		"mame.exe",
		"mame64.exe",
		"mame32.exe",
		"groovymame.exe",
		],

	"OSX" : [
		"mame",
		"mame64",
		"mame32",
		"sdlmame",
		"groovymame",
		]
};

local my_OS = OS;
if ( OS == "FreeBSD" )
	my_OS = "Linux";

////////////////////////////////////////////////////////////////////////
//
// Classes and functions used in script
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
class RompathParser
{
	exepath = "";
	homepath = "";
	rompaths = [];

	constructor( p, e )
	{
		exepath = p;
		homepath = "";
		rompaths = [];

		fe.plugin_command( p + e, "-showconfig", this, "parse_cb" );
	}

	function _tailing1( t )
	{
		local retval = "";
		for ( local i=1; i<t.len(); i++ )
		{
			retval += "/";
			retval += t[i];
		}
		return retval;
	}

	function parse_cb( op )
	{
		local temp = split( op, " \t\n" );

		if ( temp.len() < 2 )
			return;

		if ( temp[0] == "homepath" )
		{
			local t = split( temp[1], "/\\" );
			if (( t[0] == "$HOME" ) || ( t[0] == "~" ))
				homepath = fe.path_expand( t[0] ) + _tailing1( t );
			else if ( t[0] == "." )
				homepath = exepath + _tailing1( t );
			else
				homepath = fe.path_expand( temp[1] );
		}
		else if ( temp[0] == "rompath" )
		{
			local t = split( temp[1], ";" );
			foreach ( p in t )
			{
				local p2 = strip( p );

				// make sure there is a trailing slash
				if (( p2.len() > 0 ) && ( p2[ p2.len()-1 ] != 47 ))
					p2 += "/";

				if ( fe.path_test( p2, PathTest.IsRelativePath ) )
				{
					if (( homepath.len() > 0 ) && ( fe.path_test( homepath + p2, PathTest.IsDirectory ) ))
						rompaths.push( homepath + p2 );
					else if ( fe.path_test( exepath + p2, PathTest.IsDirectory ) )
						rompaths.push( exepath + p2 );
				}
				else
				{
					p2 = fe.path_expand( p2 );
					rompaths.push( p2 );
				}
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////
class VersionParser
{
	version=0;

	constructor( p, e )
	{
		version = 0;

		// This is the first time we are running an executable we suspect is mame
		// we use the timeout command where available in case the executable isn't
		// what we expect (in which case it will be killed soon after running
		//
		if ( my_OS == "Windows" )
			fe.plugin_command( p + e, "-help", this, "parse_cb" );
		else
			fe.plugin_command( "timeout", "8s " + p + e + " -help", this, "parse_cb" );
	}

	function parse_cb( op )
	{
		if ( version == 0 )
		{
			local temp = split( op, " \t\n" );
			if ( temp.len() < 2 )
			{
				version = -1;
				return;
			}

			local temp2 = split( temp[1], "." );
			if ( temp2.len() < 2 )
			{
				version = -1;
				return;
			}

			version = temp2[1].tointeger();
		}
	}
}

////////////////////////////////////////////////////////////////////////
class FullNameParser
{
	name="";
	fullname = "";

	constructor( p, e, n )
	{
		name = n;
		fe.plugin_command( p + e, "-listfull " + name, this, "parse_cb" );
	}

	function parse_cb( op )
	{
		local temp = split( op, " \t\n" );

		if ( temp[0] == name )
		{
			local temp2 = split( op, "\"(/\\" );

			if ( temp2.len() > 1 )
				fullname = temp2[1];
			else
				fullname = temp[1];
		}
	}
};

////////////////////////////////////////////////////////////////////////
class SystemParser
{
	name="";
	triggerpath=""; // kept for reporting
	is_system=false;
	extensions=[];
	fullname="";
	media="";

	constructor( p, e, n, tp )
	{
		name = n;
		triggerpath=tp;
		is_system = false;
		extensions = [];

		fe.plugin_command( p + e, "-listmedia " + name, this, "parse_cb" );

		local fnp = FullNameParser( p, e, n );
		fullname = fnp.fullname;
	}

	function parse_cb( op )
	{
		local temp = split( op, " \t\n" );

		if ( is_system && ( temp.len() > 2 ))
		{
			// if we've already got the system line then any following lines are setting
			// out other media types.  Gather the file extensions for these other media types
			for ( local i=2; i < temp.len(); i++ )
			{
				if ( temp[i].slice( 0, 1 ) == "." )
					extensions.push( temp[i] );
			}

			return;
		}

		if ( temp.len() < 4 )
			return;

		if ( temp[0] == name )
		{
			if ( temp[1].slice( 0, 1 ) != "(" )
				media = temp[1];

			for ( local i=3; i<temp.len(); i++ )
				extensions.push( temp[i] );

			is_system = true;
		}
	}
};

////////////////////////////////////////////////////////////////////////
//
// Ok, start doing stuff now...
//
////////////////////////////////////////////////////////////////////////

// search for file
//
local res = search_for_file( search_paths[ my_OS ], search_names[ my_OS ] );

// run 'mame -help' to get the version of the MAME we found
//
local ver=0;
if ( res )
{
	local vp = VersionParser( res[0], res[1] );
	ver = vp.version;
}

// If we didn't get a version, then we don't really know what we are
// dealing with...
//
if ( ver <= 0 )
{
	// No MAME found, write templates and get out of here...
	//
	local wm = write_config( mame_emu, FeConfigDirectory + "emulators/templates/" + mame_emu["name"] + ".cfg" );
	local wc = write_config( console_emu, FeConfigDirectory + "emulators/templates/" + console_emu["name"] + ".cfg", true );

	console_report( mame_emu["name"], "", false );
	console_report( console_emu["name"], "", false );
	return;
}

local path = res[0];
local executable = res[1];

// Search for console systems (we assume they are organized as subdirectories
// of the rom paths, with the sub name being the name of the system)
//
// i.e. <rompath>/nes/ for nintendo entertainment system, etc.
//
local systems = [];
function is_duplicate_system( name )
{
	foreach ( s in systems )
	{
		if ( s.name == name )
			return true;
	}

	return false;
}

local rp = RompathParser( path, executable );


if ( ver >= 162 ) // mame and mess merged as of v 162
{
	foreach ( p in rp.rompaths )
	{
		local dl = DirectoryListing( p, false );

		foreach ( sd in dl.results )
		{
			if ( fe.path_test( p + sd, PathTest.IsDirectory ) )
			{
				if ( is_duplicate_system( sd ) )
					continue;

				local st = SystemParser( path, executable, sd.tolower(), p + sd );

				if ( st.is_system )
					systems.push( st );
			}
		}
	}
}

// Check for extras files (catver.ini and nplayers.ini)
//
ext_files <-
[
	"catver.ini",
	"nplayers.ini"
];

ext_paths <- [ path ];

if ( rp.homepath.len() > 0 )
	ext_paths.push( rp.homepath );

if ( my_OS == "Linux" )
	ext_paths.push( fe.path_expand( "$HOME/.mame/" ) );

foreach ( ef in ext_files )
{
	foreach ( ep in ext_paths )
	{
		if ( fe.path_test( ep + ef, PathTest.IsFile ) )
		{
			mame_emu["import_extras"] += ep + ef + ";";
			break;
		}
	}
}

local emu_dir = FeConfigDirectory + "emulators/";

// Write emulator config for arcade
//
mame_emu["exe"] = path + executable;

if ( rp.homepath.len() > 0 )
	mame_emu["workdir"] <- rp.homepath;

mame_emu["rompath"] <- "";
foreach ( r in rp.rompaths )
	mame_emu["rompath"] += r + ";";

local wc = write_config( mame_emu, emu_dir + mame_emu["name"] + ".cfg" );
write_config( mame_emu, emu_dir + "templates/" + mame_emu["name"] + ".cfg", true );
console_report( mame_emu["name"], mame_emu["exe"], wc );

if ( !path_is_empty( split(mame_emu["rompath"],";"), split(mame_emu["exts"],";") ) )
	emulators_to_generate.push( mame_emu["name"] );

// Write emulator configs for each console system
//
console_emu["exe"] <- path + executable;

if ( rp.homepath.len() > 0 )
	console_emu["workdir"] <- rp.homepath;

// Write configs for every console system found
foreach ( s in systems )
{
	if ( s.media.len() > 0 )
		console_emu["args"] <- "[system] -" + s.media + " \"[romfilename]\"";

	console_emu["name"] <- "mame-" + s.name;
	console_emu["system"] = s.name;

	if ( s.fullname.len() > 0 )
		console_emu["system"] += ";" + s.fullname;

	console_emu["rompath"] <- "";
	foreach ( r in rp.rompaths )
		console_emu["rompath"] += r + s.name + ";";

	console_emu["exts"] <- ".zip;.7z;";
	foreach ( e in s.extensions )
		console_emu["exts"] += e + ";";

	local cfg_fn = emu_dir + console_emu["name"] + ".cfg";
	local tmp_fn = emu_dir + "templates/" + console_emu["name"] + ".cfg";

	local wc = write_config( console_emu, cfg_fn );
	write_config( console_emu, tmp_fn, true );

	print( "\t" );
	console_report( console_emu["name"], s.triggerpath, wc );

	if ( !path_is_empty( split(console_emu["rompath"],";"), split(console_emu["exts"],";") ) )
		emulators_to_generate.push( console_emu["name"] );
}

if ( systems.len() == 0 )
	write_config( console_emu, emu_dir + "templates/" + console_emu["name"] + ".cfg" );
