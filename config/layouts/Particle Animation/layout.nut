//
// This layout demonstrates liquid8d's extended object and animate modules
// It is a slightly modified version of the "3. ExtendedObjects Advanced" layout available at:
//
//               https://github.com/liquid8d/attract-extra
//
// See modules/extended/README.md for more info
//
//we will add some user config options
class UserConfig {
    </ label="Enable Debug", help="Enable/Disable debug info", options="Yes,No" />
    enable_debug="No";
    </ label="Enable Animations", help="Enable/Disable animations", options="Yes,No" />
    enable_anims="Yes";
    </ label="Enable Particles", help="Enable/Disable particle effects", options="Yes,No" />
    enable_particles="Yes";
}

local config = fe.get_config();

fe.load_module("extended/extended");
fe.load_module("extended/animate");
//we can add additional animation modules, we'll try the particules module
fe.load_module("extended/animations/particles/particles.nut");
fe.load_module("extended/animations/example/example.nut");

ExtendedObjects.add_image("bg", "bg.png", 0, 0, fe.layout.width, fe.layout.height);
ExtendedObjects.get("bg").setPreserveAspectRatio(false);

//we'll attach a particles animation to our background

//uncomment others to see them
if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "sparkle" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "snow" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "default" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "test" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "cloudstoon" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "bubbles1" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "arc1" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "invaders" } );
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "cloudstoon2" } );

//Here we'll use a custom animation
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "example", when = When.Always, duration = 20000 } );

//DOESN'T WORK - haven't implemented bounding rectangles
//if (config.enable_particles == "Yes") ExtendedObjects.get("bg").animate({ which = "particles", preset = "bounce1" } );



local title = ExtendedObjects.add_text("title", "[Title]", 0, 80, fe.layout.width / 2, 60);
    //property functions follow a setFunction or getFunction standard
    title.setColor(220, 220, 220);
    title.setCharSize(36);
    //ExtendedTexts and ExtendedImages can make use of a shadow
    title.setShadow(true);
    title.setShadowColor(240, 240, 20);
    title.setShadowAlpha(75);
    title.setShadowOffset(4);

local list = ExtendedObjects.add_listbox("list", fe.layout.width / 2, 0, fe.layout.width / 2, fe.layout.height);
    list.setCharSize(15);
    list.setStyle(Style.Bold);
    list.setColor(80, 80, 80);
    list.setSelectionColor(200, 200, 200);
    list.setSelectionBGColor(60, 60, 125);
    list.setSelectionBGAlpha(100);
    //you can set positions of objects by string names
    list.setPosition("offright");

local listAnim =    {
                        which = "property",
                        when = When.StartLayout,
                        property = "x",
                        delay = 750,
                        from = "offright",
                        to = "right"
                    }
if (config.enable_anims == "Yes") list.animate(listAnim) else list.setPosition("right");

local logo = ExtendedObjects.add_image("logo", "logo.png", 0, 0, 262, 72);
    logo.setPosition("top");

local logoAnim =  {
                        which = "translate",
                        when = When.StartLayout,
                        delay = 750,
                        duration = 1000,
                        from = "current",
                        to = "offtoprightx",
                        easing = "out",
                        tween = "bounce"
                    };
if (config.enable_anims == "Yes") logo.animate(logoAnim) else logo.setPosition("top");


local snap = ExtendedObjects.add_artwork("snap", "snap", 100, 100, 480, 360);
    snap.setPosition( [ 100, (fe.layout.height / 2) - 180 ]);
    snap.setShadow(false);
    snap.setTrigger( Transition.EndNavigation );

//You can use some predefined animation sets (a group of animations)
// NOTE: uncommenting the following will seriously slow down the front-end's responsiveness
//if (config.enable_anims == "Yes") snap.animate_set("fade_in_out" );

                    
local marquee = ExtendedObjects.add_artwork("marquee", "marquee", 0, 0, 500, 156);
    marquee.setPosition("offleft");
    marquee.setTrigger( Transition.EndNavigation );

//You can delay animations to get a step1, step2 approach
//step 1: move from offscreen left to center using the out/bounce tween
local marqueeAnim1 =  {
                        which = "translate",
                        when = When.EndNavigation,
                        wait = false,
                        duration = 750,
                        from = "offleft",
                        to = "center",
                        easing = "out",
                        tween = "bounce"
                    };
//step 2: move from center position to bottom after a delay using the out/bounce tween
local marqueeAnim2 =  {
                        which = "translate",
                        when = When.EndNavigation,
                        delay = 1000,
                        duration = 750,
                        from = "center",
                        to = "bottom",
                        easing = "out",
                        tween = "bounce"
                    };
if (config.enable_anims == "Yes") {
    marquee.animate(marqueeAnim1);
    marquee.animate(marqueeAnim2);
} else {
    marquee.setPosition("bottom");
}

//The debugger adds debug text ontop of every object, helpful for... debugging
if (config.enable_debug == "Yes") {
    local debug = ExtendedObjects.debugger();
    debug.setVisible(true);
}
