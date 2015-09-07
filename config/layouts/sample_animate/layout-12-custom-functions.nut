//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "12. Custom Functions";
OBJECTS.tutorial2.msg = "You can hook into the animation functions: onInit, onStart, onUpdate, onCancel, onRestart, onReverse and onStop\nYou can create custom 'when' functions to determine when an animation should run\nLook at the debug window to see when the animation starts, updates and stops";

OBJECTS.snap.visible = true;
OBJECTS.marquee.visible = true;
OBJECTS.listbg.visible = true;
OBJECTS.list.visible = true;

///////////////////
//  Custom Functions Sample
//
//  You can hook up functions to the init, start, update, cancel and restart methods of the animation
///////////////////

local debug = {
    onInit = function( anim ) {
        print(anim.tostring() + "anim: initialized\n");
    },
    onStart = function( anim ) {
        print(anim.tostring() + "anim started: " + animation.when(anim.transition.ttype) + " start: " + anim.startTime + " end: " + anim.endTime + "\n");
    },
    onUpdate = function( anim ) {
        print(anim.tostring() + "anim update: " + animation.when(anim.transition.ttype) + " elapsed: " + anim.time + "\n");
    },
    onCancel = function( anim ) {
        print(anim.tostring() + "anim: cancelled!\n");
    },
    onRestart = function( anim ) {
        print(anim.tostring() + "anim: restart\n");
    },
    onReverse = function( anim ) {
        print(anim.tostring() + "anim: reversed\n");
    },
    onStop = function( anim ) {
        print(anim.tostring() + "anim stopped" + "\n");
    }
}

local star = fe.add_image("resources/default.png", OBJECTS.snap.x - 32, OBJECTS.snap.y - 32, 64, 64);
star.preserve_aspect_ratio = true;
local pos_cfg = {
    onInit = debug.onInit,
    onStart = debug.onStart,
    onUpdate = debug.onUpdate,
    onStop = debug.onStop,
    when = Transition.ToNewSelection,
    property = "position",
    start = { x = OBJECTS.snap.x - 32, y = OBJECTS.snap.y - 32 },
    end = { x = OBJECTS.snap.x - 32, y = OBJECTS.snap.y + OBJECTS.snap.height - 32 },
    time = 1000
}
animation.add( PropertyAnimation( star, pos_cfg ) );

//This custom config will check if the current game is a favourite, and if so - set the alpha of the star image to 255 otherwise set it to 0
/*
star.alpha = 0;

local fav_cfg = {
    property = "alpha",
    start = 0,
    end = 255,
    time = 1500,
    when = function( anim ) {
        //this when function allows us to customize when the animation will run

        //if ( anim.transition.ttype == When.ToNewSelection )
        //{
            //if we don't use transition info here, when will continuously return true - which you may or may not want to do
            //this will run this animation only when the game we are on is a favourite
        //print( anim.transition.var + "\n" );
        if ( fe.game_info( Info.Favourite, fe.list.index ) == 1 )
        {
            //::print("This is a fav\n");
            star.alpha = 255;
            return true;
        } else
        {
            //::print("This is NOT a fav\n");
            star.alpha = 0;
            return false;
        }
        //we could also look for specific years, or any other info provided
        //if ( fe.game_info(Info.Year, fe.list.index)  == "1980" ) return true;
    }
}

animation.add( PropertyAnimation( star, fav_cfg ) );
*/