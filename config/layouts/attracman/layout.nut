/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - "Attrac-Man" Layout
//
/////////////////////////////////////////////////////////
//
//	BEGIN User Configurable Settings:
//
// Define the keys that control the Attrac-Man character:
//
::PM_UP    <- Key.R;
::PM_DOWN  <- Key.F;
::PM_LEFT  <- Key.D;
::PM_RIGHT <- Key.G;

// Define the keys that speed up/slow down game:
//
::SPEED_UP 		<- Key.A;
::SPEED_DOWN 	<- Key.S;

// Define the default game speed, should be between 0.5 and 2.0
//
const DEFAULT_SPEED = 0.8;

// Define size of speed change when SPEED_UP/SPEED_DOWN pressed
//
const SPEED_INCREMENT = 0.1;

//
// END User Configurable Settings.
//
/////////////////////////////////////////////////////////

//
// Initialize the layout's frontend-related graphic elements
//
fe.layout.width=456;
fe.layout.height=336;

local lb = fe.add_listbox( 24, 96, 192, 192 );
lb.rows = 13;
lb.charsize = 10;

fe.add_artwork( "marquee", 144, 24, 168, 48 );
fe.add_artwork( "screen", 240, 96, 192, 192 );
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
fe.do_nut( "engine.nutr" );
