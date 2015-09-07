//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "06. SpriteAnimation: Presets";
OBJECTS.tutorial2.msg = "Sprite presets will be provided as well\nMore will be added soon!";

OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;
OBJECTS.sprite.visible = true;

///////////////////
//  Sprite Presets Sample
///////////////////

local preset1 = fe.add_image("resources/joystick-move.png", 50, 50, 128, 128 );
local preset2 = fe.add_image("resources/joystick-move.png", 200, 50, 128, 128 );
local preset3 = fe.add_image("resources/joystick-move.png", 350, 50, 128, 128 );
local preset4 = fe.add_image("resources/joystick-move.png", 475, 50, 128, 128 );

//Presets are in SpriteAnimations

animation.add( SpriteAnimation( preset1, SpriteAnimations.joystick_updown ) );
animation.add( SpriteAnimation( preset2, SpriteAnimations.joystick_leftright ) );
animation.add( SpriteAnimation( preset3, SpriteAnimations.button_red ) );
animation.add( SpriteAnimation( preset4, SpriteAnimations.button_white ) );

animation.add( SpriteAnimation( OBJECTS.sprite, SpriteAnimations.pacland ) );
