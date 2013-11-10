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
::PM_UP    <- Key.W;
::PM_DOWN  <- Key.S;
::PM_LEFT  <- Key.A;
::PM_RIGHT <- Key.D;

// Define the keys that speed up/slow down game:
//
::SPEED_UP 		<- Key.Num3;
::SPEED_DOWN 	<- Key.Num2;

// Define the default game speed, should be between 0.5 and 2.0
//
const DEFAULT_SPEED = 1.0;

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
lb.rows = 15;
lb.charsize = 8;

fe.add_artwork( "marquee", 144, 24, 168, 48 );
fe.add_artwork( "screen", 240, 96, 192, 192 );
fe.add_image( "field.png", 0, 0, 456, 336 );
fe.add_text( "[ListTitle]", 118, 316, 220, 14 )

//
// Now run the game...
//
fe.do_nut( "engine.nutr" );
