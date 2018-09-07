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

class UserConfig </ help="Playable layout based on Pac-Man arcade game by Toru Iwatani (Namco, 1980)" /> {
	</ label="Player Up", help="Set control to move Player up.", is_input="yes", order=1 />
	p1_up="R";

	</ label="Player Down", help="Set control to move Player down.", is_input="yes", order=2 />
	p1_down="F";

	</ label="Player Left", help="Set control to move Player left.", is_input="yes", order=3 />
	p1_left="D";

	</ label="Player Right", help="Set control to move Player right.", is_input="yes" order=4 />
	p1_right="G";

	</ label="Increase Speed", help="Set control to incrase game speed.", is_input="yes", order=5 />
	speed_up="";

	</ label="Decrease Speed", help="Set control to decrease game speed.", is_input="yes", order=6 />
	speed_down="";

	</ label="Ghost Up", help="Set control to move Blinky up.", is_input="yes", order=7 />
	p2_up="";

	</ label="Ghost Down", help="Set control to move Blinky down.", is_input="yes", order=8 />
	p2_down="";

	</ label="Ghost Left", help="Set control to move Blinky left.", is_input="yes", order=9 />
	p2_left="";

	</ label="Ghost Right", help="Set control to move Blinky right.", is_input="yes" order=10 />
	p2_right="";

	</ label="Intro Sound", help="Sound file to play on game intro", order=11 />
	intro_sound="";

	</ label="Death Sound", help="Sound file to play on player death", order=12 />
	death_sound="";

	</ label="Chase Sound", help="Sound file to play (repeatedly) as player is being chased", order=13 />
	chase_sound="";

	</ label="Fright Sound", help="Sound file to play (repeatedly) when monsters are frightened", order=14 />
	fright_sound="";

	</ label="Artwork Mode", help="How to fit artwork images into their spot...", options="Stretch,Zoom,Preserve Aspect Ratio", order=15 />
	art_mode="Stretch";
}

::AM_CONFIG <- fe.get_config();

local zoom = (::AM_CONFIG["art_mode"]=="Zoom");
local pres_ar = (::AM_CONFIG["art_mode"]=="Preserve Aspect Ratio");

//
// Initialize the layout's frontend-related graphic elements
//
fe.layout.width=456;
fe.layout.height=336;
fe.layout.preserve_aspect_ratio=true;

fe.load_module( "fade" );

local snap;
if ( zoom )
{
	snap = FadeArt( "snap", 215, 71, 242, 242 );
	fe.add_transition_callback( "fix_zoom" );
}
else
	snap = FadeArt( "snap", 240, 96, 192, 192 );

snap.trigger = Transition.EndNavigation;

if ( pres_ar || zoom )
	snap.preserve_aspect_ratio = true;

function fix_zoom( ttype, var, ttime )
{
	if ( ttype == Transition.EndNavigation )
	{
		if ( snap._front.texture_height > snap._front.texture_width )
		{
			snap.x = 215;
			snap.y = 51;
			snap.width  = 242;
			snap.height = 252;
		}
		else
		{
			snap.x = 210;
			snap.y = 71;
			snap.width  = 252;
			snap.height = 242;
		}
	}
	return false;
}

// Create a FadeArt that doesn't do anything on transitions (just timed fading of artwork)
//
class MyFade extends FadeArt { function on_transition( ttype, var, ttime ) {return false;}; };
local lb_bg = MyFade( "fanart", 24, 96, 192, 192 );

local sort_lb = fe.add_listbox( 176, 96, 45, 202 );
sort_lb.rows = 13;
sort_lb.charsize = 10;
sort_lb.set_rgb( 255, 255, 0 );
sort_lb.format_string = "[SortValue]";
sort_lb.bg_alpha = 195;
sort_lb.selbg_alpha = 225;
sort_lb.set_selbg_rgb( 0, 0, 120 );
sort_lb.visible = false;

local lb = fe.add_listbox( 24, 96, 192, 202 );
lb.rows = 13;
lb.charsize = 10;
lb.bg_alpha=195;
lb.selbg_alpha=225;
lb.set_selbg_rgb( 0, 0, 120 );

local tmp = FadeArt( "marquee", 144, 24, 168, 48 );
tmp.trigger = Transition.EndNavigation;
if ( pres_ar )
	tmp.preserve_aspect_ratio = true;

fe.add_image( "field.png", 0, 0, 456, 336 );
local main_caption = fe.add_text( "[DisplayName]", 118, 316, 220, 14 );

fe.overlay.set_custom_controls( main_caption, lb );

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
