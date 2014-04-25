///////////////////////////////////////////////////
//
// Attract-Mode Frontend - Orbit layout
//
///////////////////////////////////////////////////
class UserConfig {
	</ label="Orbit Artwork", help="The artwork to spin into orbit", options="marquee,flyer,wheel" />
	orbit_art="marquee";

	</ label="Bloom Effect", help="Enable Bloom Effect (requires shader support)", options="Yes,No" />
	enable_bloom="Yes";
}

local my_config = fe.get_config();

fe.layout.width = 800
fe.layout.height = 600

const MWIDTH = 280;
const MHEIGHT = 170;
const SPIN_MS = 200;

function get_y( x ) {
	return ( 200 + sqrt( pow( 270, 2 ) - pow( x - 400, 2 ) ) );
}

function set_bright( x, o ) {
	o.set_rgb( x, x, x );
}

local no_shader = fe.add_shader( Shader.Empty );
local yes_shader;
if ( my_config["enable_bloom"] == "Yes" )
{
	yes_shader = fe.add_shader( Shader.Fragment, "bloom_shader.frag" );
	yes_shader.set_texture_param("bgl_RenderedTexture");
}
else
{
	yes_shader = no_shader;
}

class Marquee {
	ob=null; 
	orig_ob=null;
	base_io=0;
	xl=0; xm=0; xr=0; 
	sl=0.0; sm=0.0; sr=0.0;

	constructor( pio, pxl, pxm, pxr, psl, psm, psr ) {
		xl=pxl; xm=pxm; xr=pxr; sl=psl; sm=psm; sr=psr;
		orig_ob = ob = fe.add_artwork( my_config["orbit_art"] );
		ob.preserve_aspect_ratio=true;
		ob.video_flags = Vid.ImagesOnly;
		ob.index_offset = base_io = pio;
		reset();
	}

	function move_left( p ) {
		local scale = ( sm - ( sm - sl ) * p );
		local nx = xm - ( xm - xl ) * p;

		ob.width = MWIDTH * scale;
		ob.height = MHEIGHT * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function move_right( p ) {
		local scale = ( sm - ( sm - sr ) * p );
		local nx = xm + ( xr - xm ) * p;

		ob.width = MWIDTH * scale;
		ob.height = MHEIGHT * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function reset() {
		ob.width = MWIDTH * sm;
		ob.height = MHEIGHT * sm;
		ob.x = xm - ob.width / 2;
		ob.y = get_y( xm ) - ob.height / 2;
		set_bright( sm * 255, ob );
	}
	function swap_art( o ) {
		local temp = o.ob;
		o.ob = ob;
		ob = temp;
	}
}

fe.add_artwork( "snap", 224, 59, 352, 264 );
local frame = fe.add_image( "frame.png", 216, 51, 368, 278 );
frame.shader = yes_shader;

local l = fe.add_text( "[ListFilterName] [[ListEntry]/[ListSize]]", 400, 580, 400, 20 );
l.set_rgb( 180, 180, 70 );
l.align = Align.Right;

local l = fe.add_text( "[Category]", 0, 580, 400, 20 );
l.set_rgb( 180, 180, 70 );
l.align = Align.Left;

local marquees = [
	Marquee( -2, 200, 150, 145, 0.7, 0.4, 0.1 ), 
	Marquee( -1, 400, 200, 150, 1.0, 0.7, 0.4 ), 
	Marquee(  2, 655, 650, 600, 0.1, 0.4, 0.7 )
];

// Delayed creation of these two so that they are drawn over top of the others
// (they will be later in the draw order)
//
marquees.insert( 2, Marquee(  1, 650, 600, 400, 0.4, 0.7, 1.0 ) );

// This is the marquee for the current selection
marquees.insert( 2, Marquee(  0, 600, 400, 200, 0.7, 1.0, 0.7 ) );
marquees[2].ob.shader = yes_shader;

l = fe.add_text( "[ListTitle]", 0, 0, 800, 55 );
l.set_rgb( 180, 180, 70 ); 
l.style = Style.Bold;
l.shader = yes_shader;

l = fe.add_text( "[Title], [Manufacturer] [Year]", 0, 550, 800, 30 );
l.set_rgb( 255, 255, 255 );

fe.add_transition_callback( "orbit_transition" );

local last_move=0;

function orbit_transition( ttype, var, ttime ) {
	switch ( ttype )
	{
	case Transition.ToNewSelection:
		if ( ttime < SPIN_MS )
		{
			marquees[2].ob.shader = no_shader;
			local moves = abs( var );
			local jump_adjust = 0;
			if ( moves > marquees.len() )
			{
				jump_adjust = moves - marquees.len();
				moves = marquees.len();
			}

			local move_duration = SPIN_MS / moves;
			local curr_move = ttime / move_duration;

			local change_index=false;
			if ( curr_move > last_move )
			{
				last_move=curr_move;
				change_index=true;
			}

			local progress = ( ttime % move_duration ).tofloat() / move_duration;

			if ( var < 0 )
			{

				if ( change_index )
				{
					// marquees[marquees.len()-1].ob will get swapped through to the leftmost position
					marquees[marquees.len()-1].ob.index_offset = marquees[0].base_io - curr_move - jump_adjust;
					for ( local i=marquees.len()-1; i>0; i-=1 )
					{
						marquees[i].swap_art( marquees[i-1] );
						marquees[i].reset();
					}
				}

				foreach ( m in marquees )
					m.move_left( progress );
			}
			else
			{
				if ( change_index )
				{
					// marquees[0].ob will get swapped through to the rightmost position
					marquees[0].ob.index_offset = marquees[marquees.len()-1].base_io + curr_move + jump_adjust;
					for ( local i=0; i<marquees.len()-1; i+=1 )
					{
						marquees[i].swap_art( marquees[i+1] );
						marquees[i].reset();
					}
				}
				foreach ( m in marquees )
					m.move_right( progress );
			}
			return true;
		}

		foreach ( m in marquees )
		{
			m.ob = m.orig_ob;
			m.reset();
			m.ob.index_offset = m.base_io;
		}
		marquees[2].ob.shader = yes_shader;
		last_move=0;
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
		break;

	case Transition.EndLayout:
	case Transition.ToGame:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = 255 - ttime;

			return true;
		}
		break;
	}

	return false;
}

fe.add_ticks_callback( "orbit_tick" );

function orbit_tick( ttime ) {
	local block = ttime / 30000;

	if ( block % 2 )
		set_bright( ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
	else
		set_bright( 255 - ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
}
