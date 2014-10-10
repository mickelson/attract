/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - "Attrac-Man" Layout
//
/////////////////////////////////////////////////////////
// Define the default game speed, should be between 0.5 and 2.0
//
const DEFAULT_SPEED = 0.8;

// Define size of speed change when SPEED_UP/SPEED_DOWN pressed
//
const SPEED_INCREMENT = 0.1;

class UserConfig {
	</ label="Player Up", help="Set player controls", is_input="yes" />
	a_up="R";

	</ label="Player Down", help="Set player controls", is_input="yes" />
	b_down="F";

	</ label="Player Left", help="Set player controls", is_input="yes" />
	c_left="D";

	</ label="Player Right", help="Set player controls", is_input="yes" />
	d_right="G";

	</ label="Increase Speed", help="Set game speed controls", is_input="yes" />
	e_speed_up="";

	</ label="Decrease Speed", help="Set game speed controls", is_input="yes" />
	f_speed_down="";

}

::AM_CONFIG <- fe.get_config();

//
// Initialize the layout's frontend-related graphic elements
//
fe.layout.width=456;
fe.layout.height=336;

//
// Show the sort value if we have sorted this list by something other than the title
//
local lb_width = 192;
if (( fe.list.sort_by != Info.NoSort ) && ( fe.list.sort_by != Info.Title ))
{
	lb_width = 157;
	local sort_lb = fe.add_listbox( 176, 96, 45, 192 );
	sort_lb.rows = 13;
	sort_lb.charsize = 10;
	sort_lb.set_rgb( 255, 255, 0 );
	sort_lb.format_string = "[SortValue]";
}

local lb = fe.add_listbox( 24, 96, lb_width, 192 );
lb.rows = 13;
lb.charsize = 10;

fe.add_artwork( "marquee", 144, 24, 168, 48 );
fe.add_artwork( "snap", 240, 96, 192, 192 );
fe.add_image( "field.png", 0, 0, 456, 336 );
fe.add_text( "[ListTitle]", 118, 316, 220, 14 );

local l = fe.add_text( "[ListEntry]/[ListSize]", 0, 322, 60, 10 );
l.align = Align.Left;
l.set_rgb( 80, 80, 80 );

l = fe.add_text( "[ListFilterName]", 396, 322, 60, 10 );
l.align = Align.Right;
l.set_rgb( 80, 80, 80 );

//
// Now run the game...
//
fe.do_nut( "engine.nut" );
