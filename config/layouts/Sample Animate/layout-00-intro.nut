//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "Using the Animate module";
OBJECTS.tutorial2.msg = "Make sure the animate.nut and animate folder are in your AM modules folder.\nAdd fe.load_module(\"animate\"); to your layout.nut file.\nCreate an animation config table with the animation attributes.\nAdd the animation, specifying the object (if needed) and animation config";

OBJECTS.title.visible = false;
OBJECTS.snap.visible = false;
OBJECTS.list.visible = false;

local line1 = fe.add_text("fe.load_module(\"animate\");", 20, 50, fe.layout.width, 30);
local line2 = fe.add_text("local animation = { property=\"x\", start = 0, end = 100 }", 20, 80, fe.layout.width, 30);
local line3 = fe.add_text("animation.add( PropertyAnimation( text, animation ) );", 20, 110, fe.layout.width, 30);

local line4 = fe.add_text("//Add the module\n//Create an animation config\n//Add the animation", 700, 60, fe.layout.width, 90);
    line4.charsize = 24;
    line4.word_wrap = true;

local line5 = fe.add_text("Three animation types available right now\nPropertyAnimation\nSpriteAnimation\nParticleAnimation", 20, 200, fe.layout.width, 120);
    line5.set_rgb( 200, 200, 0 );
    line5.charsize = 24;
    line5.word_wrap = true;

line1.align = line2.align = line3.align = line4.align = Align.Left;

