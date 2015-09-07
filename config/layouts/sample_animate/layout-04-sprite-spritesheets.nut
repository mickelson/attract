//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");

OBJECTS.tutorial1.msg = "04. SpriteAnimation: Using Sprite Sheets";
OBJECTS.tutorial2.msg = "Sprite sheets can add some nice touches to your layout";

OBJECTS.joystick.visible = true;
OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;

///////////////////
//  Sprite Sample
///////////////////

local joy_text = fe.add_text("Use joystick to move up/down list", OBJECTS.joystick.x + 75, OBJECTS.joystick.y + 50, 600, 30 );
joy_text.charsize = 32;

local sprite_cfg = {
    when = When.Always,
    width = 128,
    frame = 0,
    time = 3000,
    order = [ 0, 3, 0, 3, 0, 4, 0, 4 ],
    loop = true
}

animation.add( SpriteAnimation( OBJECTS.joystick, sprite_cfg ) );