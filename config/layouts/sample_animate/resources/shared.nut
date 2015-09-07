fe.load_module("animate");

fe.layout.width = 1280;
fe.layout.height = 720;

//Some objects we will use animations on
//Some objects we will use animations on
::OBJECTS <- {
    bg = fe.add_image( "resources/bg.png", 0, 0, fe.layout.width, fe.layout.height ),
    debugbg = fe.add_image( "resources/debug-720p.png", 0, 0, fe.layout.width, fe.layout.height ),
    surface = fe.add_surface( fe.layout.width, fe.layout.height ),
    snap = fe.add_artwork("snap", 200, 60, 320, 240),
    marquee = fe.add_artwork("marquee", 170, 360, 400, 100),
    red_block = fe.add_image("resources/pixel.png", 100, 100, 100, 100),
    green_block = fe.add_image("resources/pixel.png", 300, 100, 100, 100),
    blue_block = fe.add_image("resources/pixel.png", 600, 100, 100, 100),
    yellow_block = fe.add_image("resources/pixel.png", 100, 300, 100, 100),
    box = fe.add_image("resources/debug.png", 900, 100, 100, 100),
    title = fe.add_text("[Title]", 0, 15, fe.layout.width, 72),
    joystick = fe.add_image("resources/joystick-move.png", 100, 260, 128, 128),
    sprite = fe.add_image("resources/pixel.png", 80, 260, 64, 64),
    logo = fe.add_image("resources/logo-verion.png", 400, 100, 432, 160 ),
    listbg = fe.add_image("resources/pixel.png", 800, 80, 420, 400 ),
    list = fe.add_listbox( 800, 80, 420, 400 ),
    tutorialbg = fe.add_image("resources/tutorialbg.png", 12, fe.layout.height - 196, 1265, 165 ),
    tutorialtitle = fe.add_image("resources/tutorial_title.png", 1300, 350, 720, 80 ),
    tutorial1 = fe.add_text("", 15, fe.layout.height - 180, fe.layout.width, 30 ),
    tutorial2 = fe.add_text("", 20, fe.layout.height - 150, fe.layout.width, 90 ),
    tutorial3 = fe.add_text("Press Toggle Layout key to see the next example", 0, fe.layout.height - 40, fe.layout.width, 30 )
}

//these will set to visible for each layout that uses them
OBJECTS.debugbg.visible = false;
OBJECTS.snap.visible = false;
OBJECTS.marquee.visible = false;
OBJECTS.red_block.visible = false;
OBJECTS.green_block.visible = false;
OBJECTS.blue_block.visible = false;
OBJECTS.yellow_block.visible = false;
OBJECTS.box.visible = false;
OBJECTS.title.visible = false;
OBJECTS.joystick.visible = false;
OBJECTS.sprite.visible = false;
OBJECTS.logo.visible = false;
OBJECTS.listbg.visible = false;
OBJECTS.list.visible = false;
OBJECTS.tutorialtitle.visible = false;

OBJECTS.snap.preserve_aspect_ratio = true;
OBJECTS.snap.video_flags = Vid.NoAudio;
OBJECTS.marquee.preserve_aspect_ratio = true;

OBJECTS.tutorial1.style = Style.Bold;
OBJECTS.tutorial1.align = Align.Left;
OBJECTS.tutorial2.align = Align.Left;
OBJECTS.tutorial2.charsize = 18;
OBJECTS.tutorial2.word_wrap = true;
OBJECTS.tutorial3.set_rgb( 200, 200, 50 );

OBJECTS.red_block.set_rgb(255, 0, 0);
OBJECTS.red_block.alpha = 100;
OBJECTS.green_block.set_rgb(0, 255, 0);
OBJECTS.green_block.alpha = 100;
OBJECTS.blue_block.set_rgb(0, 0, 255);
OBJECTS.blue_block.alpha = 100;
OBJECTS.box.set_rgb(240, 0, 240);
OBJECTS.box.alpha = 255;
OBJECTS.yellow_block.set_rgb(250, 250, 0);

OBJECTS.list.set_selbg_rgb(50, 160, 50);
OBJECTS.list.set_rgb(50, 50, 50);
OBJECTS.listbg.set_rgb(100, 200, 100);

OBJECTS.title.charsize = 32;
OBJECTS.title.set_rgb( 255, 255, 0 );
OBJECTS.joystick.subimg_width = 128;
OBJECTS.joystick.subimg_height = 128;

local pulse = {
    when = When.Always,
    property = "alpha",
    start = 255,
    end = 100,
    time = 1000,
    loop = 5,
}
animation.add( PropertyAnimation( OBJECTS.tutorial3, pulse ) );

