//
// Map of supported theme file types -> their loader script
//
local loadmap = [
	[ ".mll", "mala.nut" ],
	[ ".lay", "mamewah.nut" ],
];

fe.load_module( "file" );
local dir = DirectoryListing( fe.script_dir, false );

foreach ( f in dir.results )
{
	foreach ( m in loadmap )
	{
		if ( f.slice( f.len()-m[0].len() ) == m[0] )
		{
			::file_to_load<-f;
			return dofile(fe.loader_dir+m[1]);
		}
	}
}

//
// Try XML only if the previous stuff didn't work
//
foreach ( f in dir.results )
{
	if ( f.slice( f.len()-4 ) == ".xml" )
	{
		::file_to_load<-f;
		return dofile(fe.loader_dir+"attract_xml.nut");
	}
}

//
// Utility function to test for a hyperspin layout
//
function test_for_hyperspin( my_path )
{
	local score=0;

	local d = DirectoryListing( my_path, false );
	foreach ( f in d.results )
	{
		local subdir = DirectoryListing( my_path + f, false );

		foreach ( sf in subdir.results )
		{
			if (( sf == "Images" )
					|| ( sf == "Sound" )
					|| ( sf == "Themes" )
					|| ( sf == "Video" ))
				score++;

			if ( score > 2 )
				return true;
		}
	}
	return false;
}

//
// Final step is to test for hyperspin
//
local hs = false;
if ( test_for_hyperspin( fe.script_dir ) )
{
	hs=true;
	::file_to_load<-"";
}
else if ( test_for_hyperspin( fe.script_dir + "Media/" ) )
{
	hs=true;
	::file_to_load<-"Media/";
}

if ( hs )
	return dofile(fe.loader_dir+"hyperspin.nut");
