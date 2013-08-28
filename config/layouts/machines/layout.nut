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

local l = fe.add_listbox( 43, 261, 490, 417 );
l.charsize = 19;
l.align = Align.Centre;

fe.add_artwork( "screen", 577, 263, 412, 412 );
fe.add_image( "machinesselbg.png", 0, 0, 1024, 768 );

l = fe.add_text( "[list_title]", 45, 195, 940, 31 );
l.red = 180;
l.green = 180;
l.blue = 70;
l.charsize = 26;
l.align = Align.Centre;
l.style = Style.Bold;

l = fe.add_text( "[manufacturer]", 70, 714, 280, 30 );
l.charsize = 18;
l.align = Align.Centre;

l = fe.add_text( "[year]", 362, 714, 160, 31 );
l.charsize = 18;
l.align = Align.Centre;

l = fe.add_text( "[category]", 588, 714, 370, 31 );
l.charsize = 18;
l.align = Align.Centre;

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
