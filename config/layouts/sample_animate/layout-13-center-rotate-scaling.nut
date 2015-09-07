//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "13. Center scale and rotate";
OBJECTS.tutorial2.msg = "The animate module helps by handling center rotation and scaling\nTry moving up, down, page up, page down to see some";

OBJECTS.bg.visible = false;
OBJECTS.debugbg.visible = true;
OBJECTS.red_block.visible = true;
OBJECTS.yellow_block.visible = true;
OBJECTS.green_block.visible = true;
OBJECTS.blue_block.visible = true;
OBJECTS.box.visible = true;
OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;

OBJECTS.list.y = OBJECTS.listbg.y = 250;
OBJECTS.list.height = OBJECTS.listbg.height = 250;
    
///////////////////
//  Center rotation and scaling
//
//  You can hook up functions to the init, start, update, cancel and restart methods of the animation
///////////////////

local enlarge = PropertyAnimations.enlarge;
    enlarge.when <- When.OnPageUp;
local shrink = PropertyAnimations.shrink;
    shrink.when <- When.OnPageDown;
    
animation.add( PropertyAnimation( OBJECTS.red_block, enlarge ) );
animation.add( PropertyAnimation( OBJECTS.green_block, shrink ) );
animation.add( PropertyAnimation( OBJECTS.blue_block, PropertyAnimations.slide_left ) );
animation.add( PropertyAnimation( OBJECTS.blue_block, PropertyAnimations.slide_right ) );
animation.add( PropertyAnimation( OBJECTS.yellow_block, PropertyAnimations.rotate_left_90 ) );
animation.add( PropertyAnimation( OBJECTS.yellow_block, PropertyAnimations.rotate_right_90 ) );
animation.add( PropertyAnimation( OBJECTS.box, PropertyAnimations.slide_right ) ); //currently order matters for position animations
animation.add( PropertyAnimation( OBJECTS.box, PropertyAnimations.enlarge ) ); //currently order matters for position animations
animation.add( PropertyAnimation( OBJECTS.box, PropertyAnimations.rotate_right_45 ) ); //currently order matters for position animations
