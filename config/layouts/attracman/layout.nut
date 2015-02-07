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
	</ label="Player Up", help="Set player controls", is_input="yes", order=1 />
	p1_up="R";

	</ label="Player Down", help="Set player controls", is_input="yes", order=2 />
	p1_down="F";

	</ label="Player Left", help="Set player controls", is_input="yes", order=3 />
	p1_left="D";

	</ label="Player Right", help="Set player controls", is_input="yes" order=4 />
	p1_right="G";

	</ label="Increase Speed", help="Set game speed controls", is_input="yes", order=5 />
	speed_up="";

	</ label="Decrease Speed", help="Set game speed controls", is_input="yes", order=6 />
	speed_down="";

	</ label="Ghost Up", help="Set controls to influence Blinky", is_input="yes", order=7 />
	p2_up="";

	</ label="Ghost Down", help="Set controls to influence Blinky", is_input="yes", order=8 />
	p2_down="";

	</ label="Ghost Left", help="Set controls to influence Blinky", is_input="yes", order=9 />
	p2_left="";

	</ label="Ghost Right", help="Set controls to influence Blinky", is_input="yes" order=10 />
	p2_right="";


}

::AM_CONFIG <- fe.get_config();

//
// Initialize the layout's frontend-related graphic elements
//
fe.layout.width=456;
fe.layout.height=336;

local sort_lb = fe.add_listbox( 176, 96, 45, 192 );
sort_lb.rows = 13;
sort_lb.charsize = 10;
sort_lb.set_rgb( 255, 255, 0 );
sort_lb.format_string = "[SortValue]";
sort_lb.visible = false;

local lb = fe.add_listbox( 24, 96, 192, 192 );
lb.rows = 13;
lb.charsize = 10;

local tmp = fe.add_artwork( "marquee", 144, 24, 168, 48 );
tmp.trigger = Transition.EndNavigation;

tmp = fe.add_artwork( "snap", 240, 96, 192, 192 );
tmp.trigger = Transition.EndNavigation;

fe.add_image( "field.png", 0, 0, 456, 336 );
fe.add_text( "[DisplayName]", 118, 316, 220, 14 );

local l = fe.add_text( "[ListEntry]/[ListSize]", 0, 322, 60, 10 );
l.align = Align.Left;
l.set_rgb( 80, 80, 80 );

l = fe.add_text( "[FilterName]", 396, 322, 60, 10 );
l.align = Align.Right;
l.set_rgb( 80, 80, 80 );

//
// Update the listbox display to show the sort critera if we are showing
// a filter that is sorted
//
fe.add_transition_callback( "attracman_transition" );
function attracman_transition( ttype, var, ttime )
{
	if ( ttype == Transition.ToNewList )
	{
		if (( fe.list.sort_by != Info.NoSort )
				&& ( fe.list.sort_by != Info.Title ))
		{
			lb.width = 157;
			sort_lb.visible = true;
		}
		else
		{
			lb.width = 192;
			sort_lb.visible = false;
		}
	}
	return false;
}

//
// Now run the game...
//
fe.do_nut( "engine.nut" );
