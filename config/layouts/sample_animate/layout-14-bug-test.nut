//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "14. Bug Testing";
OBJECTS.tutorial2.msg = "Animations can be really tricky depending on which ones you combine and the settings for each.\nIt's important to understand what you want to do and the limitations and complications of this.\nHere will try some funky combinations to highlight some 'bugs' or in programmers terms - unexpected features!";

OBJECTS.joystick.visible = true;

animation.add( SpriteAnimation( OBJECTS.joystick, SpriteAnimations.button_white ) );
