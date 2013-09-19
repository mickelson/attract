//
// Attract-Mode Front-End - "Machines" sample layout
//
// adapted from the "Machines" 3D Arcade skin by PucPuc
// http://www.mameworld.info/3darcade/skins/machines_pucpuc.zip
//
fe.layout.width = 1024;
fe.layout.height = 768;
fe.layout.font = "slicker";

// Set out the various artwork, image and text elements
//
fe.add_artwork( "marquee", 312, 30, 400, 115 );
local lb = fe.add_listbox( 43, 261, 490, 417 );
lb.charsize = 22;
fe.add_artwork( "screen", 577, 263, 412, 412 );
fe.add_image( "machinesselbg.png", 0, 0, 1024, 768 );

local l = fe.add_text( "[ListTitle]", 45, 195, 940, 25 );
l.set_rgb( 180, 180, 70 );
l.style = Style.Bold;

fe.add_text( "[Manufacturer]", 70, 714, 280, 22 );
fe.add_text( "[Year]", 362, 714, 160, 22 );
fe.add_text( "[Category]", 588, 714, 370, 22 );

// Class to encapsulate each machine animation:
//
class Machine {
	img=null;
	fcount=0;
	constructor( name, x, y, w, h ) {
		img = fe.add_image( name, x, y, w, h );
		img.subimg_width=img.texture_width;
		img.subimg_height=img.texture_width;
		fcount=img.texture_height / img.subimg_height;
	}
	function animate( f ) {
		img.subimg_y = ( f % fcount ) * img.subimg_height;
	}
}

// Create a class instance for each animation
//
local machines = [ 
	Machine( "joust.png", 109, 75, 21, 21 ),
	Machine( "mpatrol.png", 191, 68, 29, 22 ),
	Machine( "defender.png", 239, 69, 27, 13 ),
	Machine( "polepos.png", 752, 74, 24, 17 ),
	Machine( "invaders.png", 810, 65, 19, 23 ),
	Machine( "phoenix.png", 939, 60, 17, 25 )
];

local next_time=500;
local frame=0;

fe.add_ticks_callback( "tick" );

// Animate the machines every half second
//
function tick( t )
{
	if ( t > next_time )
	{
		frame++;
		next_time += 500;

		foreach (m in machines)
			m.animate( frame );
	}
}
