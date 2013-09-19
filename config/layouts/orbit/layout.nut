//
// Attract-Mode Front-End - "Orbit" sample layout
//
fe.layout.width = 800
fe.layout.height = 600

const mwidth = 300;
const mheight = 100;

function get_y( x ) {
	return ( 225 + sqrt( pow( 270, 2 ) - pow( x - 400, 2 ) ) );
}

function set_bright( x, o ) {
	o.set_rgb( x, x, x );
}

class Marquee {
	ob=null; 
	xl=0; xm=0; xr=0; 
	sl=0.0; sm=0.0; sr=0.0;

	constructor( pio, pxl, pxm, pxr, psl, psm, psr ) {
		xl=pxl; xm=pxm; xr=pxr; sl=psl; sm=psm; sr=psr;
		ob = fe.add_artwork( "marquee" );
		ob.index_offset = pio;
		reset();
	}

	function move_left( p ) {
		local scale = ( sm - ( sm - sl ) * p );
		local nx = xm - ( xm - xl ) * p;

		ob.width = mwidth * scale;
		ob.height = mheight * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function move_right( p ) {
		local scale = ( sm - ( sm - sr ) * p );
		local nx = xm + ( xr - xm ) * p;

		ob.width = mwidth * scale;
		ob.height = mheight * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function reset() {
		ob.width = mwidth * sm;
		ob.height = mheight * sm;
		ob.x = xm - ob.width / 2;
		ob.y = get_y( xm ) - ob.height / 2;
		set_bright( sm * 255, ob );
	}

}

fe.add_artwork( "screen", 224, 59, 352, 264 );
local frame = fe.add_image( "frame.png", 220, 55, 360, 270 );

local games = [
	Marquee( -2, 200, 150, 145, 0.7, 0.4, 0.1 ), 
	Marquee(  2, 655, 650, 600, 0.1, 0.4, 0.7 ), 
	Marquee( -1, 400, 200, 150, 1.0, 0.7, 0.4 ), 
	Marquee(  1, 650, 600, 400, 0.4, 0.7, 1.0 ), 
	Marquee(  0, 600, 400, 200, 0.7, 1.0, 0.7 )
];


local l = fe.add_text( "[ListTitle]", 0, 0, 800, 55 );
l.set_rgb( 180, 180, 70 ); 
l.style = Style.Bold;

l = fe.add_text( "[Title], [Manufacturer] [Year]", 0, 570, 800, 30 );
l.set_rgb( 255, 255, 255 );

fe.add_transition_callback( "transition" );

function transition( ttype, var, ttime ) {
	switch ( ttype )
	{
	case Transition.ToNewSelection:
		if ( ttime < 200 )
		{
			if ( var < 0 )
				foreach ( g in games )
					g.move_left( ttime / 200.0 );
			else
				foreach ( g in games )
					g.move_right( ttime / 200.0 );

			return true;
		}

		foreach ( g in games )
			g.reset();
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

fe.add_ticks_callback( "tick" );

function tick( ttime ) {
	local block = ttime / 30000;

	if ( block % 2 )
		set_bright( ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
	else
		set_bright( 255 - ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
}
