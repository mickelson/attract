//
// Attract-Mode Front-End - "Reflect" sample layout
//
class UserConfig {
	// Note the per_display="true" below means that this parameter has to be configured by the user for each
	// display where this layout gets used.  If this isn't set then the layout parameter is the same across
	// all displays that use this layout
	//
	</ label="Colour Scheme", help="Select the menu colour scheme.", options="Blue,Green,Red,Yellow,Purple,Brown,Grey", per_display="yes" />
	colour="Blue";
};

local config = fe.get_config();

fe.load_module( "conveyor" );
fe.load_module( "fade" );

class Colour
{
	r = 0; g = 0; b = 0;
	constructor( rr, gg, bb ) { r = rr; g = gg; b = bb; }
};

local bg_colour = Colour( 0, 0, 255 );
local fg_colour = Colour( 255, 255, 255 );
switch ( config[ "colour" ] )
{
	case "Blue":
		bg_colour = Colour( 0, 0, 255 );
		fg_colour = Colour( 200, 200, 200 );
		break;
	case "Green":
		bg_colour = Colour( 0, 100, 0 );
		fg_colour = Colour( 80, 255, 80 );
		break;
	case "Red":
		bg_colour = Colour( 180, 0, 0 );
		fg_colour = Colour( 255, 190, 190 );
		break;
	case "Yellow":
		bg_colour = Colour( 255, 255, 0 );
		fg_colour = Colour( 100, 100, 0 );
		break;
	case "Purple":
		bg_colour = Colour( 100, 0, 100 );
		fg_colour = Colour( 250, 100, 250 );
		break;
	case "Brown":
		bg_colour = Colour( 80, 80, 0 );
		fg_colour = Colour( 200, 200, 100 );
		break;
	case "Grey":
		bg_colour = Colour( 60, 60, 60 );
		fg_colour = Colour( 180, 180, 180 );
		break;
}

fe.layout.width=640;
fe.layout.height=480;
fe.layout.page_size=15;

// fill an entire surface with our snap at a resolution of 480x360
//
local surface = fe.add_surface( 480, 360 );
local snap = FadeArt( "snap", 0, 0, 480, 360, surface );
snap.preserve_aspect_ratio = true;

// position and pinch the surface
//
surface.set_pos( 330, 80, 300, 250 );
surface.pinch_y = -80;

// now create a reflection of the surface
//
local reflect = fe.add_clone( surface );
reflect.subimg_y = reflect.texture_height;
reflect.subimg_height = -reflect.texture_height;
reflect.set_rgb( 40, 40, 40 );

// position the reflection
//
reflect.set_pos( 325, 340, 300, 60 );
reflect.skew_y = 170;
reflect.skew_x = -200;

local hyp = pow( 240, 2 );
function get_x( y )
{
	return 240 - sqrt( hyp - pow( ( 240 - y ), 2 ) );
}

// store the background image used for listbox entries here:
//
local lb_bg_image=0;

// Class representing a listbox bubble
//
class LBEntry extends ConveyorSlot
{
	bg = 0;
	text = 0;
	texts = 0;
	m_num = 0;

	constructor( num )
	{
		if ( lb_bg_image == 0 )
			bg = lb_bg_image = fe.add_image( "bubble.png" );
		else
			bg = fe.add_clone( lb_bg_image );

		bg.set_rgb( bg_colour.r, bg_colour.g, bg_colour.b );
		m_num = num;

		texts = fe.add_text( _get_name(), 0, 0, 320, 25 );
		texts.set_rgb( 0, 0, 0 );

		text = fe.add_text( _get_name(), 0, 0, 320, 25 );

		if ( num == 7 )	// selection bubble
		{
			if ( config["colour"] == "Yellow" )
				text.set_rgb( 0, 0, 0 );
			else
				text.set_rgb( 255, 255, 0 );
		}
		else	
		{
			text.set_rgb( fg_colour.r, fg_colour.g, fg_colour.b );
		}
	}

	function update()
	{
		text.msg = texts.msg = _get_name();
	}

	function _get_name() 
	{
		local s = split( fe.game_info( Info.Title, get_base_index_offset() ), "(" );

		if ( s.len() > 0 )
			return s[0];

		return "";
	}

	function on_progress( progress, var )
	{
		local prog_range = 1.0 / 15;

		local x=0;
		local y=0;
		local width=190;
		local height=20;

		if ( progress <= prog_range * 6 )
		{
			y = 10 + 30 * ( progress / prog_range );
			x = get_x( y );
		}
		else if ( progress >= prog_range * 8 )
		{
			// bottom entries
			y = 270 + 30 * ( progress / prog_range - 8.0 );
			x = get_x( 10 + 30 * ( 14.0 - progress / prog_range ) );
		}
		else
		{
			// selection
			if ( progress <= prog_range * 7 )
			{
				local factor = ( progress - prog_range * 6 ) / prog_range;
				y = 190 + 34 * factor;
				x = get_x( y );

				width = 190 + 130 * factor;
				height = 20 + 5 * factor;
			}
			else
			{
				local factor = ( progress - prog_range * 7 ) / prog_range;
				y = 224 + 34 * factor;
				x = get_x( 10 + 30 * ( 14.0 - progress / prog_range ) );

				width = 320 - 130 * factor;
				height = 25 - 5 * factor;
			}
		}

		bg.set_pos( x - 10, y - 3, width + 12, height + 7 );
		texts.set_pos( x + 1, y + 1, width, height );
		text.set_pos( x, y, width, height );
	}

	function swap( other )
	{
		local temp = other.text.msg;
		text.msg = texts.msg = other.text.msg;
		other.text.msg = other.texts.msg = temp;
	}

	function set_index_offset( io )
	{
		text.index_offset = texts.index_offset = io;
	}

	function reset_index_offset() { set_index_offset( m_base_io ); }
}

local lb = [];
for ( local j=0; j<15; j++ )
	lb.append( LBEntry( j ) );

local my_conveyor = Conveyor();
my_conveyor.transition_swap_point = 1.0;
my_conveyor.transition_ms = 140;
my_conveyor.set_slots( lb );

class RedBubble
{
	bg = 0;
	text = 0;
	texts = 0;

	constructor( t, x, y, w, h, pad=0 )
	{
		if ( lb_bg_image == 0 )
			bg = lb_bg_image = fe.add_image( "bubble.png" );
		else
			bg = fe.add_clone( lb_bg_image );

		bg.set_pos( x - 15, y + 1 - pad, w + 15, h + 2 * pad );

		if ( config[ "colour" ] == "Red" )
			bg.set_rgb( 0, 0, 255 );
		else
			bg.set_rgb( 220, 0, 0 );

		texts = fe.add_text( t, x + 1, y + 1, w, h );
		texts.set_rgb( 0, 0, 0 );
		text = fe.add_text( t, x, y, w, h );
	}

	function set_msg( m )
	{
		text.msg = texts.msg = m;
	}

	function set_visible( f )
	{
		bg.visible = text.visible = texts.visible = f;
	}

	function get_visible()
	{
		return bg.visible;
	}

	function set_alpha( a )
	{
		bg.alpha = text.alpha = texts.alpha = a;
	}

	function get_alpha()
	{
		return bg.alpha;
	}
}

local info_bub = RedBubble( "", 0, 249, 295, 15 );

local l = fe.add_text( "[Title]", 285, 407, 325, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

l = fe.add_text( "[Category]", 370, 424, 240, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

l = fe.add_text( "Played [PlayedCount] time(s)", 370, 441, 240, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

l = fe.add_text( "[ListEntry]/[ListSize]", 370, 458, 240, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

l = fe.add_text( "[DisplayName]", 4, 4, 140, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Left;

l = fe.add_text( "[FilterName]", 370, 4, 240, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

local loading_bub = RedBubble( "Loading...", 195, 220, 250, 40, 5 );
loading_bub.set_visible( false );

local do_fade_in = false;
local fade_in_time = 0;
fe.add_transition_callback( "update_lb" );
function update_lb( ttype, var, ttime )
{
	switch ( ttype )
	{
	case Transition.ToGame:
		if ( loading_bub.get_visible() == false )
		{
			loading_bub.set_visible( true );
			return true; // need a redraw
		}
		break;

	case Transition.FromGame:
		if ( loading_bub.get_visible() == true )
		{
			loading_bub.set_visible( false );
			return true; // need a redraw
		}
		break;

	case Transition.ToNewSelection:
		info_bub.set_alpha( 0 );
		fade_in_time = clock() * 1000 + 500;
		break;

	case Transition.FromOldSelection:
	case Transition.ToNewList:
		foreach( l in lb )
		{
			l.update();

			//
			// Hide entries appropriately at the edges of the list
			//
			local rom_abs = fe.list.index + l.text.index_offset; 
			if (( rom_abs < 0 ) || ( rom_abs >= fe.list.size ))
			{
				l.bg.visible = l.text.visible = l.texts.visible = false;
			}
			else
			{
				l.bg.visible = l.text.visible = l.texts.visible = true;
			}
		}


		local year = fe.game_info( Info.Year, 0 );
		if ( year.len() > 0 )
		{
			info_bub.set_msg( "Copyright " + year + " " +
			fe.game_info( Info.Manufacturer, 0 ) );
			do_fade_in = true;
		}
		else
		{
			info_bub.set_msg( "" );
			do_fade_in = false;
			info_bub.set_alpha( 0 );
		}
		break;
	}

	return false;
}

fe.add_ticks_callback( "info_fade_tick" );
function info_fade_tick( ttime )
{
	if ( do_fade_in )
	{
		local t = clock() * 1000;
		if ( t > fade_in_time )
		{
			local a = t - fade_in_time;
			if ( a > 255 ) a = 255;

			if ( info_bub.get_alpha() < 255 )
				info_bub.set_alpha( a );
			else
				do_fade_in = false;
		}
	}
}
