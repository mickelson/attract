//
// "Any Command" utility for the Attract-Mode frontend
//
// Launchable from the Utilty Menu plugin
//
const MAX_OUTPUT_LINES = 40;

fe.load_module( "submenu" );

class AnyCommandOutput extends SubMenu
{
	m_t = "";

	constructor()
	{
		base.constructor();
		m_t = fe.add_text( "", 0, 0, fe.layout.width, fe.layout.height );
		m_t.charsize=fe.layout.height / MAX_OUTPUT_LINES;
		m_t.align=Align.Left;
		m_t.word_wrap=true;
		m_t.bg_alpha=180;
		m_t.visible = false;
	}

	function on_show() { m_t.visible = true; }
	function on_hide() { m_t.visible = false; }
	function on_scroll_up() { m_t.first_line_hint--; }
	function on_scroll_down() { m_t.first_line_hint++; }
};

//
// Set up an overlay text for our output if we haven't set one up previously
//
if ( ::rawin( "any_command_output" ) == false )
{
	::any_command_output <- AnyCommandOutput();
}

//
// Prompt user for the command to run
//
local command = fe.overlay.edit_dialog( "Enter Command", "" );

//
// Hide the overlay if no command is entered
//
if ( command.len() < 1 )
{
	::any_command_output.show( false );
	return;
}

//
// Run the command, dumping the output into "output"
//
local output="";
function any_command_callback( tt )
{
	output += rstrip( tt ) + "\n";
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
	::any_command_output.show( false );
	return;
}

//
// Otherwise, put the output in the overlay and make it visible
//
::any_command_output.m_t.msg = "\n\n" + output + "\n\n";
::any_command_output.show( true );
