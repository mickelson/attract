//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "01. PropertyAnimation: Object Properties";
OBJECTS.tutorial2.msg = "Any property with a number value can be animated\nHere, title 'y' is animated when the layout is started and marquee 'x' and snap 'alpha' are animated on Transition.ToNewSelection\nAnimations can start with any Transition or some special 'When' values like When.StartLayout\nYou can specify 'start' and 'end' values, the animation 'time', 'loop' count and much more";

OBJECTS.snap.visible = true;
OBJECTS.marquee.visible = true;
OBJECTS.title.visible = true;

///////////////////
//  Property Sample
///////////////////

//change 'alpha' property from 0 to 255 when ToNewSelection occurs - take 3 seconds (3000ms) to do it
//you can change any object property whose value is a number
local alpha_cfg = {
    when = Transition.ToNewSelection,
    property = "alpha",
    start = 0,
    end = 255,
    time = 3000
}
animation.add( PropertyAnimation( OBJECTS.snap, alpha_cfg ) );

local movey_cfg = {
    when = Transition.StartLayout,
    property = "y",
    start = -50,
    end = OBJECTS.title.y,
    time = 3000
}
animation.add( PropertyAnimation( OBJECTS.title, movey_cfg ) );

local movex_cfg = {
    when = Transition.ToNewSelection,
    property = "x",
    start = -OBJECTS.marquee.width - 20,
    end = OBJECTS.marquee.x,
    time = 2000
}
animation.add( PropertyAnimation( OBJECTS.marquee, movex_cfg ) );
