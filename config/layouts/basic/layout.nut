//
// Attract-Mode Front-End - "Basic" sample layout
//
fe.layout.width=640;
fe.layout.height=480;

fe.add_artwork( "screen", 348, 152, 262, 262 );
fe.add_artwork( "marquee", 348, 64, 262, 72 );

local l = fe.add_listbox( 32, 64, 262, 352 );
l.charsize = 16;
l.set_selbg_rgb( 255, 255, 255 );
l.set_sel_rgb( 0, 0, 0 );
l.sel_style = Style.Bold;

fe.add_image( "bg.png", 0, 0 );

l = fe.add_text( "[ListTitle]", 0, 15, 640, 30 );
l.set_rgb( 200, 200, 70 );
l.style = Style.Bold;

// Left side:

l = fe.add_text( "[Title]", 30, 424, 320, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Left;

l = fe.add_text( "[Year] [Manufacturer]", 30, 441, 320, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Left;

l = fe.add_text( "[Category]", 30, 458, 320, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Left;

// Right side:

l = fe.add_text( "[ListEntry]/[ListSize]", 320, 424, 290, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

l = fe.add_text( "[ListFilterName]", 320, 441, 290, 16 );
l.set_rgb( 200, 200, 70 );
l.align = Align.Right;

