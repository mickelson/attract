fe.layout.width=640;
fe.layout.height=480;
fe.layout.font="coolvetica";

// GPS
local wheelx = 10;
local flyerx = 360; 
// Image shadow/outline thickness
local offset = 4;

// Trim functions
function trimmed_name( index_offset ) {
    local s = split( fe.game_info( Info.Title, index_offset ), "/(" );
    if ( s.len() > 0 )
        return s[0];
    return "";
}

function trimmed_copy( index_offset ) {
    local s = split( fe.game_info( Info.Manufacturer, index_offset ), "(" );
    if ( s.len() > 0 )
        return "© " + fe.game_info( Info.Year, index_offset ) + " " + s[0];
    return "";
}

// Gives us a nice high random number for the RGB levels
function brightrand() {
	return 255-(rand()/255);
}

local red = brightrand();
local green = brightrand();
local blue = brightrand();

// Game snapshot
local snap = fe.add_artwork( "snap", -80, -60, 800, 600);

// Game title box
local titlebg = fe.add_image ("mask.png", 0, 0, fe.layout.width, fe.layout.height);
titlebg.set_rgb (0,0,0);

// Game flyer
local flyeroutline = fe.add_artwork( "flyer", flyerx, 20, 300, 400);
flyeroutline.rotation = 5;
local flyer = fe.add_clone( flyeroutline );
flyeroutline.x = flyeroutline.x - (offset / 2);
flyeroutline.y = flyeroutline.y - (offset / 2);
flyeroutline.width = flyeroutline.width + offset ;
flyeroutline.height = flyeroutline.height + offset ;
flyeroutline.set_rgb (32,32,32);

// Filters
local romlist = fe.add_text( "[ListTitle]", 5, 20, fe.layout.width - 5, 20 );
romlist.align = Align.Left;
romlist.style = Style.Bold;
local filter = fe.add_text( "[ListFilterName]", 5, 20, fe.layout.width - 5, 20 );
filter.align = Align.Right;
filter.style = Style.Bold;
local details = fe.add_text( "[ListEntry]/[ListSize]", 0, 0, fe.layout.width, 16 );
details.align = Align.Right;
details.style = Style.Bold;

// Game title block
local copy = fe.add_text( trimmed_copy ( 0 ), 30, 435, 320, 20 );
copy.align = Align.Left;
//local man = fe.add_text( "©" + trimmed_copy( 0 ) + " [Year]", 42, 435, 320, 20 );
//man.align = Align.Left;
//local yearshadow = fe.add_text( "[Year]", 21, 376, fe.layout.width-10, 94 );
//yearshadow.align = Align.Right;
//yearshadow.style = Style.Bold;
//yearshadow.set_rgb (0,0,0);
//local year = fe.add_text( "[Year]", 20, 375, fe.layout.width-10, 94 );
//year.align = Align.Right;
//year.style = Style.Bold;
local gametitleshadow = fe.add_text( trimmed_name( 0 ), 21, 411, fe.layout.width, 24 );
gametitleshadow.align = Align.Left;
gametitleshadow.style = Style.Bold;
gametitleshadow.set_rgb (0,0,0);
local gametitle = fe.add_text( trimmed_name( 0 ), 20, 410, fe.layout.width, 24 );
gametitle.align = Align.Left;
gametitle.style = Style.Bold;

// Category
local cat = fe.add_text( "[Category]", fe.layout.width-24, fe.layout.height, fe.layout.height, 24 );
cat.set_rgb( 0, 0, 0 );
cat.rotation = -90; 
cat.align = Align.Left;

// Game wheel image
local wheelshadow = fe.add_artwork( "wheel", wheelx, 220, 0, 0);
local wheel = fe.add_clone( wheelshadow);
wheelshadow.x = wheelshadow.x + offset;
wheelshadow.y = wheelshadow.y + offset;
wheelshadow.set_rgb (0,0,0);
wheel.set_rgb (255,255,255);

local message = fe.add_text("Loading...",0,200,fe.layout.width,80)
message.alpha = 0;
message.style = Style.Bold;

// Transitions
fe.add_transition_callback( "fancy_transitions" );

function fancy_transitions( ttype, var, ttime ) {
	switch ( ttype )
	{
	case Transition.ToNewList:
	case Transition.ToNewSelection:
		gametitleshadow.msg = trimmed_name( var );
		gametitle.msg = trimmed_name( var );
		copy.msg = trimmed_copy ( var );
		red = brightrand();
		green = brightrand();
                blue = brightrand();
		//year.set_rgb (red,green,blue);
		copy.set_rgb (red,green,blue);
		//man.set_rgb (red,green,blue);
		break;

	case Transition.FromGame:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = ttime;
			message.alpha = 0;     
			return true;
		}
		else
		{
			foreach (o in fe.obj)
				o.alpha = 255;
			message.alpha = 0;
		}
		break;
    
	case Transition.EndLayout:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = 255 - ttime;
			message.alpha = 0; 
			return true;
		}
		else
		{
			foreach (o in fe.obj)
				o.alpha = 255;
			message.alpha = 0;
		}
		break;
     
	case Transition.ToGame:
		if ( ttime < 255 )
		{
			foreach (o in fe.obj)
				o.alpha = 255 - ttime;
			message.alpha = ttime;
			return true;
		}   
		break; 
	}
	return false;
}
