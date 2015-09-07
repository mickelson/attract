//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.logo.visible = true;
OBJECTS.tutorialtitle.visible = true;

OBJECTS.tutorial1.msg = "AttractMode: Animate module";
OBJECTS.tutorial2.msg = "This demonstration shows the features of the animate module. Each example is in a different layout.\nYou need to configure your Toggle Layout key for AttractMode. Press TAB, go to Controls, and set a key for the Toggle Layout control.\nMake sure you have your snap and marquee setup for this display for full effect.\nUse the Toggle Layout key to move to the next example!";

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Bounce, time = 2500 } ) );

animation.add( PropertyAnimation( OBJECTS.tutorialtitle, { property = "x", start = OBJECTS.tutorialtitle.x, end = 250, tween = Tween.Back, time = 2500, delay = 2500 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "scale", start = 1.0, end = 1.5, time = 15000, delay = 5000 } ) );
