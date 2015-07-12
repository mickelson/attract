//
// Map of supported theme file types -> their loader script
//
local loadmap = [
	[ ".mll", "mala.nut" ]
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
