//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "02. PropertyAnimation: Additional Properties";
OBJECTS.tutorial2.msg = "Additional object properties can be used for scale, position and color";

OBJECTS.snap.visible = true;
OBJECTS.marquee.visible = true;
OBJECTS.title.visible = true;
OBJECTS.list.visible = true;

OBJECTS.listbg.visible = true;
OBJECTS.listbg.alpha = 0;

//some additional properties are available to make things easier

//use 'scale' property to scale an object in its current position
local scale_cfg = { when = Transition.ToNewSelection, property = "scale", start = 1.0, end = 1.5, time = 1500 }

//use 'position' property to move an object from one position to another
local pos_cfg = { when = Transition.ToNewSelection, property = "position", start = { x = 0, y = -40 }, end = { x = 0, y = OBJECTS.title.y }, time = 1000 }

//use 'color' property to change from one color to another
local color_cfg = { when = Transition.ToNewSelection, property = "color", start = { red = 255, green = 255, blue = 0 }, end = { red = 255, green = 0, blue = 255 }, time = 3000 }

local list_pos_cfg = { when = Transition.StartLayout, property = "position", start = { x = OBJECTS.list.x + OBJECTS.list.width, y = OBJECTS.list.y }, end = { x = OBJECTS.list.x, y = OBJECTS.list.y }, time = 2000 }

//now, add the PropertyAnimations to an object with the desired config table
animation.add( PropertyAnimation( OBJECTS.marquee, scale_cfg ) );
animation.add( PropertyAnimation( OBJECTS.title, pos_cfg ) );
animation.add( PropertyAnimation( OBJECTS.title, color_cfg ) );
animation.add( PropertyAnimation( OBJECTS.list, list_pos_cfg ) );

animation.add( PropertyAnimation( OBJECTS.listbg, { property = "alpha" start = 0, end = 255, delay = 2500 } ) );
