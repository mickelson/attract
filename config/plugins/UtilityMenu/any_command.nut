//
// "Any Command" utility for the Attract-Mode frontend
//
// Launchable from the Utility Menu plugin
//

//
// Prompt user for the command to run
//
local command = fe.overlay.edit_dialog( "Enter Command", "" );

if ( command.len() < 1 )
	return;

//
// Run the command, dumping the output into "output"
//
local output="";
function any_command_callback( tt )
{
	output += rstrip( tt ) + "\n";
};

print( "Running command: " + command + "\n" );
if ( OS == "Windows" )
{
	fe.plugin_command( "cmd", "/c " + command, "any_command_callback" );
}
else
{
	fe.plugin_command( "/bin/sh", "-c \"" + command + "\"", "any_command_callback" );
}

if ( output.len() < 1 )
	return;

//
// Put the output in the Utility Menu's output viewer
//
fe.plugin[ "Utility Menu" ].show_output( "\n\n" + output + "\n\n" );
