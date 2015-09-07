//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "05. SpriteAnimation: Konami Code";
OBJECTS.tutorial2.msg = "With the config, you have control over the sprite sheet.\nHere, the order of frames is specified to show the konami code!";

OBJECTS.joystick.visible = true;

///////////////////
//  Sprite Sample
///////////////////

local sprite_cfg = {
    when = When.StartLayout,
    width = 128,
    frame = 0,
    time = 5000,
    order = [ 0, 3, 0, 3, 0, 4, 0, 4, 0, 1, 0, 2, 0, 1, 0, 2, 0 ],
    delay = 2000
}
animation.add( SpriteAnimation( OBJECTS.joystick, sprite_cfg ) );

//Just for fun
local woo = fe.add_text("Woo, 30 lives!", OBJECTS.joystick.x + 90, OBJECTS.joystick.y + 50, 300, 30 );
    woo.set_rgb( 220, 220, 0 );
    woo.alpha = 0;
animation.add( PropertyAnimation( woo, { property = "alpha", start = 0, end = 255, delay = 7500 } ) );
animation.add( PropertyAnimation( woo, { property = "y", start = OBJECTS.joystick.y + 50, end = OBJECTS.joystick.y, delay = 7500, time = 3000 } ) );
animation.add( PropertyAnimation( woo, { property = "alpha", start = 255, end = 0, delay = 10000, time = 500 } ) );
