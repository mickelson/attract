//
// "Any Command" utility for the Attract-Mode frontend
//
// Launchable from the Utilty Menu plugin
//
const MAX_OUTPUT_LINES = 40;

//
// Set up an overlay text for our output if we haven't set one up previously
//
if ( ::rawin( "any_command_output" ) == false )
{
	::any_command_output <- fe.add_text( "", 0, 0, fe.layout.width, fe.layout.height );
	::any_command_output.charsize=fe.layout.height / MAX_OUTPUT_LINES;
	::any_command_output.align=Align.Left;
	::any_command_output.word_wrap=true;
	::any_command_output.bg_alpha=180;
}

//
// Prompt user for the command to run
//
local command = fe.edit_dialog( "Enter Command", "" );

//
// Hide the overlay if no command is entered
//
if ( command.len() < 1 )
{
	::any_command_output.visible = false;
	return;
}

//
// Run the command, dumping the output into "output"
//
local output="";
local line_count=0;

function any_command_callback( tt )
{
	if ( line_count < MAX_OUTPUT_LINES )
		output += rstrip( tt ) + "\n";

	line_count++;
};

if ( OS == "Windows" )
{
	fe.plugin_command( "cmd", "/c " + command, "any_command_callback" );
}
else
{
	fe.plugin_command( "/bin/sh", "-c " + command, "any_command_callback" );
}

//
// If there is no output, hide the overlay
//
if ( output.len() < 1 )
{
	::any_command_output.visible = false;
	return;
}

//
// Otherwise, put the output in the overlay and make it visible
//
::any_command_output.msg = output;
::any_command_output.visible = true;
