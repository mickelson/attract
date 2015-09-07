//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "07. Combine Animations: SpriteAnimation + PropertyAnimation";
OBJECTS.tutorial2.msg = "Use multiple animations on one object for some interesting effects\nNavigate to the next game to see the animation";

OBJECTS.sprite.visible = true;
OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;
OBJECTS.snap.visible = true;
OBJECTS.marquee.visible = true;

///////////////////
//  Multiple Animations Sample
///////////////////

OBJECTS.sprite.x = 1350;
OBJECTS.sprite.y = 400;

//PropertyAnimation
local movex_cfg = { when = Transition.ToNewSelection, property = "x", start = 1300, end = 500, time = 1000 }
animation.add( PropertyAnimation( OBJECTS.sprite, movex_cfg ) );

//SpriteAnimation (preset)
animation.add( SpriteAnimation( OBJECTS.sprite, SpriteAnimations.pacland ) );
