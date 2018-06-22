///////////////////////////////////////////////////
//
// Attract-Mode Frontend - Orbit layout
//
///////////////////////////////////////////////////
//
// NOTES:
//
// - The looping static video is from the rec room website at:
//   http://recroomhq.com/downloads/2010/04/14/tv-static-freebie.html
//   It is licensed under the Creative Commons Attribution 3.0 License
//
///////////////////////////////////////////////////
class UserConfig {
	// Note the per_display="true" below means that this parameter has to be configured by the user for each
	// display where this layout gets used.  If this isn't set then the layout parameter is the same across
	// all displays that use this layout
	//
	</ label="Orbit Artwork", help="The artwork to spin into orbit", options="marquee,flyer,wheel", order=1, per_display="true" />
	orbit_art="marquee";

	</ label="Bloom Effect", help="Enable Bloom Effect (requires shader support)", options="Yes,No", order=2 />
	enable_bloom="Yes";

	</ label="Background", help="The filename of the image or video to display in the background", order=3 />
	bg_image="";

	</ label="Satellite Count", help="The number of orbiting artworks", options="5,7,9,11,13,15", order=4 />
	count="5";

	</ label="Spin Time", help="The amount of time it takes to spin to the next selection (in milliseconds)", order=5 />
	spin_ms="120";

	</ label="Static Effect", help="Enable static effect", options="Yes,No", order=6 />
	static_effect="Yes";
}

fe.load_module( "conveyor" );
local my_config = fe.get_config();

fe.layout.width = 800
fe.layout.height = 600

const MWIDTH = 280;
const MHEIGHT = 170;
const SNAPBG_ALPHA = 200;

local num_sats = fe.layout.page_size = my_config["count"].tointeger();
local progress_correction = 1.0 / ( num_sats * 2 );

local spin_ms = 120;
try {
	spin_ms = my_config["spin_ms"].tointeger();
} catch ( e ) {}

function get_y( x )
{
	// 270^2 = 72900
	return ( 200 + sqrt( 72900 - pow( x - 400, 2 ) ));
}

function set_bright( x, o )
{
	o.set_rgb( x, x, x );
}

local last_op = 0;
local last_x = 0;

function set_bright_lb( x, o )
{
	last_x = x;
	o.set_rgb( x, x, x );
	o.set_selbg_rgb( x, x, x );

	last_op = ( x + 120 ) % 255;
	if (( x > 135 ) && ( x < 145 ))
		last_op = 25 * ( 145 - x );

	o.set_bg_rgb( last_op, last_op, last_op );
	o.set_sel_rgb( last_op, last_op, last_op );
}

//
// Create a class to contain an orbit artwork
//
class Satallite extends ConveyorSlot
{
	static x_lookup = [ 145, 147, 150, 165, 200, 250,
			400, 550, 600, 635, 650, 653, 655 ];
	static s_lookup = [ 0.1, 0.2, 0.5, 0.6, 0.86, 0.95,
			1.25, 0.95, 0.86, 0.6, 0.5, 0.2, 0.1 ];

	constructor()
	{
		local o = fe.add_artwork( my_config["orbit_art"] );
		o.preserve_aspect_ratio=true;
		o.video_flags = Vid.ImagesOnly;

		base.constructor( o );
	}

	//
	// Place, scale and set the colour of the artwork based
	// on the value of "progress" which ranges from 0.0-1.0
	//
	function on_progress( progress, var )
	{
		local scale;
		local new_x;
		progress += progress_correction;

		if ( progress >= 1.0 )
		{
			scale = s_lookup[ 12 ];
			new_x = x_lookup[ 12 ];
		}
		else if ( progress < 0 )
		{
			scale = s_lookup[ 0 ];
			new_x = x_lookup[ 0 ];
		}
		else
		{
			local slice = ( progress * 12.0 ).tointeger();
			local factor = ( progress - ( slice / 12.0 ) ) * 12.0;

			scale = s_lookup[ slice ]
				+ (s_lookup[slice+1] - s_lookup[slice]) * factor;

			new_x = x_lookup[ slice ]
				+ (x_lookup[slice+1] - x_lookup[slice]) * factor;
		}

		m_obj.width = MWIDTH * scale;
		m_obj.height = MHEIGHT * scale;
		m_obj.x = new_x - m_obj.width / 2;
		m_obj.y = get_y( new_x ) - m_obj.height / 2;

		set_bright( ( scale > 1.0 ) ? 255 : scale * 255, m_obj );
	}
}

//
// Initialize background image if configured
//
if ( my_config[ "bg_image" ].len() > 0 )
	fe.add_image( my_config[ "bg_image" ],
			0, 0, fe.layout.width, fe.layout.height );

//
// Initialize the video frame
//
local snapbg=null;
if ( my_config[ "static_effect" ] == "Yes" )
{
	snapbg = fe.add_image(
		"static.mp4",
		224, 59, 352, 264 );

	snapbg.set_rgb( 150, 150, 150 );
	snapbg.alpha = SNAPBG_ALPHA;
}
else
{
	local temp = fe.add_text(
		"",
		224, 59, 352, 264 );
	temp.bg_alpha = SNAPBG_ALPHA;
}

local snap = fe.add_artwork( "snap", 224, 59, 352, 264 );
snap.trigger = Transition.EndNavigation;

local overlay_lb = fe.add_listbox( 224, 59, 352, 264 );
overlay_lb.rows = 10;
const OVERLAY_ALPHA = 190;
overlay_lb.bg_alpha = overlay_lb.selbg_alpha = OVERLAY_ALPHA;
overlay_lb.set_sel_rgb( 0, 0, 0 );
overlay_lb.set_selbg_rgb( 255, 255, 255 );
overlay_lb.visible=false;

local frame = fe.add_image( "frame.png", 216, 51, 368, 278 );

//
// Initialize misc text
//
local l = fe.add_text( "[FilterName] [[ListEntry]/[ListSize]]", 400, 580, 400, 20 );
l.set_rgb( 180, 180, 70 );
l.align = Align.Right;

local l = fe.add_text( "[Category]", 0, 580, 400, 20 );
l.set_rgb( 180, 180, 70 );
l.align = Align.Left;

//
// Initialize the orbit artworks with selection at the top
// of the draw order
//
local sats = [];
for ( local i=0; i < num_sats  / 2; i++ )
	sats.append( Satallite() );

for ( local i=0; i < ( num_sats + 1 ) / 2; i++ )
	sats.insert( num_sats / 2, Satallite() );

//
// Initialize a conveyor to control the artworks
//
local orbital = Conveyor();
orbital.transition_ms = spin_ms;
orbital.transition_swap_point = 1.0;
orbital.set_slots( sats );

//
// Title text
//
l = fe.add_text( "[DisplayName]", 0, 0, 800, 55 );
l.set_rgb( 180, 180, 70 ); 
l.style = Style.Bold;

fe.overlay.set_custom_controls( l, overlay_lb );

//
// Set the shader effect if configured
//
if ( my_config["enable_bloom"] == "Yes" )
{
	local sh = fe.add_shader( Shader.Fragment, "bloom_shader.frag" );
	sh.set_texture_param("bgl_RenderedTexture"); 

	frame.shader = sh;
	sats[ sats.len() / 2 ].m_obj.shader = sh;
	l.shader = sh;
}

//
// Name text w/ black outline
//
fe.add_text( "[Title], [Manufacturer] [Year]", 0, 550, 800, 30 )
	.set_rgb( 0, 0, 0 );

fe.add_text( "[Title], [Manufacturer] [Year]", 2, 550, 800, 30 )
	.set_rgb( 0, 0, 0 );

fe.add_text( "[Title], [Manufacturer] [Year]", 1, 550, 800, 30 )
	.set_rgb( 0, 0, 0 );

fe.add_text( "[Title], [Manufacturer] [Year]", 1, 552, 800, 30 )
	.set_rgb( 0, 0, 0 );

fe.add_text( "[Title], [Manufacturer] [Year]", 1, 551, 800, 30 )
	.set_rgb( 255, 255, 255 );

//
// Have the frame around the snap/video slowly cycles from white to
// black and back
//
fe.add_ticks_callback( "orbit_tick" );

function orbit_tick( ttime )
{
	local block = ttime / 30000;
	local bright = ( ( ttime % 30000 ) / 30000.0 ) * 255;

	if ( block % 2 )
	{
		set_bright( bright, frame );
		set_bright_lb( bright, overlay_lb );
	}
	else
	{
		set_bright( 255 - bright, frame );
		set_bright_lb( 255 - bright, overlay_lb );
	}
}

//
// Add fade effect when moving to/from the layout or a game
//
fe.add_transition_callback( "orbit_transition" );
function orbit_transition( ttype, var, ttime )
{
	switch ( ttype )
	{
	case Transition.ToNewSelection:
		if ( snapbg )
		{
			if ( snap.file_name.len() > 0 )
			{
				if ( ttime < spin_ms )
				{
					snap.alpha = 255 - 255.0 * ttime / spin_ms;
					return true;
				}
			}
			snap.file_name="";
			snap.alpha=0;
		}
		break;

	case Transition.EndNavigation:
		if ( snapbg )
		{
			if ( ttime < spin_ms )
			{
				snap.alpha = 255.0 * ttime / spin_ms;
				return true;
			}
			snap.alpha = 255;
		}
		break;

	case Transition.StartLayout:
	case Transition.FromGame:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = ttime;

			return true;
		}
		else
		{
			foreach (o in fe.obj)
				o.alpha = 255;
		}
		if ( snapbg )
			snapbg.alpha=SNAPBG_ALPHA;
		break;

	case Transition.EndLayout:
	case Transition.ToGame:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = 255 - ttime;

			return true;
		}
		else
		{
			local old_alpha;
			foreach (o in fe.obj)
			{
				old_alpha = o.alpha;
				o.alpha = 0;
			}

			if ( old_alpha != 0 )
				return true;
		}

		break;

	case Transition.ShowOverlay:
		overlay_lb.rows = ( var == Overlay.Exit ) ? 6 : 10;
		overlay_lb.visible = true;

		// Do a fade in effect for the overlay text, lasting for
		// spin_ms.  selection text goes from sel bg colour to
		// selection text colour, and unselected text goes from bg
		// colour to unselected text colour
		//
		if ( ttime < spin_ms )
		{
			local percent = ttime / spin_ms.tofloat();
			local span = ( last_op - last_x ) * percent;
			local x = last_op - span;
			local op = last_x + span;

			local alpha = OVERLAY_ALPHA
				+ ( 255 - OVERLAY_ALPHA ) * percent;

			overlay_lb.sel_alpha = alpha;
			overlay_lb.alpha = alpha;
			overlay_lb.set_rgb( x, x, x );
			overlay_lb.set_sel_rgb( op, op, op );
			return true;
		}
		overlay_lb.sel_alpha = 255;
		overlay_lb.alpha = 255;
		break;

	case Transition.HideOverlay:
		overlay_lb.visible = false;
		break;
	}

	return false;
}
