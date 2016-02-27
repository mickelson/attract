//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.logo.visible = true;

OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;

OBJECTS.tutorial1.msg = "08. Chaining Animations";
OBJECTS.tutorial2.msg = "Each animation can have a delay, so you can 'chain' one after the other\nThe first animation changes the 'y' property of the logo, the second 'scale' animation delays the length of the first, then starts\nThis is better than using 'wait' (returning true in a transition function), because you can still use the interface :)";

OBJECTS.logo.x = 175;
OBJECTS.logo.y = 150;

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Bounce, time = 2500 } ) );
animation.add( PropertyAnimation( OBJECTS.logo, { property = "scale", start = 1.0, end = 1.5, time = 15000, delay = 5000 } ) );
