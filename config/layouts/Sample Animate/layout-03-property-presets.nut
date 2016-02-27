//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "03. PropertyAnimation: Presets";
OBJECTS.tutorial2.msg = "PropertyAnimation presets are provided\nThese are hooked to common animations for common transition types\nMore will be added soon!";

OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;
OBJECTS.snap.visible = true;
OBJECTS.marquee.visible = true;

///////////////////
//  Property Presets Sample
///////////////////

//Presets are in PropertyAnimations

animation.add( PropertyAnimation( OBJECTS.marquee, PropertyAnimations.enlarge_fun ) );
animation.add( PropertyAnimation( OBJECTS.snap, PropertyAnimations.enlarge ) );
animation.add( PropertyAnimation( OBJECTS.snap, PropertyAnimations.rotate_left_10 ) );
