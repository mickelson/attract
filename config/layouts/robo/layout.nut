//
// Attract-Mode Front-End - "Robo" sample layout
//
// Robotron Model Copyright Andrew Kator & Jennifer Legaz
// http://3dmodels.katorlegaz.com/arcade_machines/248/index.php
//
fe.layout.width = 1024;
fe.layout.height = 768;

local marquee = fe.add_artwork( "marquee", 578, 30, 400, 105 );
marquee.shear_x = -80;
marquee.rotation = 1;

local l = fe.add_listbox( 0, 0, 490, 768 );
l.charsize = 19;

l = fe.add_artwork( "", 733, 250, 225, 200 );
l.shear_x = -12;
l.rotation = 5;

fe.add_image( "robo.png", 0, 0, 1024, 768 );
