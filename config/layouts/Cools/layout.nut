// Layout by cools / Arcade Otaku
// http://www.arcadeotaku.com
// Uses snap, logo and flyer images. Will use title images if desired.
// 
// History/changes: too many. I use this so it's forever WIP
///////////////////////////////////////////////////////// 
// Layout User Options
class UserConfig {
 </ label="Background Image", help="Choose snap/video snap, title, fanart, user image (bg.jpg in layout folder) or no background", options="snap,video,title,fanart,user,none" /> bg_image = "video";
 </ label="Preview Image", help="Choose snap/video snap, title or none.", options="snap,video,title,none" /> preview_image = "none";
 </ label="Title Flicker", help="Flicker the game title", options="Yes,No" /> enable_flicker="Yes";
 </ label="Display List Name", help="Show ROM list name", options="Yes,No" /> enable_list="Yes";
 </ label="Display Filter Name", help="Show filter name", options="Yes,No" /> enable_filter="Yes";
 </ label="Display Entries", help="Show quantity of ROMs in current filter", options="Yes,No" /> enable_entries="Yes";
 </ label="Display Category", help="Show game category", options="Yes,No" /> enable_category="Yes";
 </ label="Flyer Angle", help="Rotation of the game flyer in degrees (0-15)", options="0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15" /> flyer_angle="5";
 </ label="Display Flyer", help="Hides the flyer/game box.", options="Yes,No" /> enable_flyer="Yes";
 </ label="Logo Position", help="Positions the logo on screen.", options="Left,Centre,Right" /> logo_position="Left";
}
local my_config = fe.get_config();
// Layout Constants
fe.layout.width=320;
fe.layout.height=240;
local lw=fe.layout.width;
local lh=fe.layout.height;
local bgx=(lw/8)*-1;
local bgy=(lh/8)*-1
local bgw=(lw/4)*5;
local bgh=(lh/4)*5;
// Game name text. We do this in the layout as the frontend doesn't chop up titles with a forward slash
function gamename( index_offset ) {
 local s = split( fe.game_info( Info.Title, index_offset ), "(/[" );
 if ( s.len() > 0 ) return s[0];
 return "";
}
// Copyright text
function copyright( index_offset ) {
 local s = split( fe.game_info( Info.Manufacturer, index_offset ), "(" );
 if ( s.len() > 0 ) return "© " + fe.game_info( Info.Year, index_offset ) + " " + s[0];
 return "";
}
// Returns a random number below 255. Set randrange to higher values to hit closer to 255
function highrand( randrange ) {
 return 255-(rand()/randrange);
}
// Random high colour values
local red = highrand( 255 );
local green = highrand( 255 );
local blue = highrand( 255 );
local grey = highrand( 1024 );
/////////////////////////////////////////////////////////
// On Screen Objects
// Background Image
if ( my_config["bg_image"] == "video") {
 local bg = fe.add_artwork( "snap", bgx, bgy, bgw, bgh );
}
if ( my_config["bg_image"] == "snap") {
 local bg = fe.add_artwork( "snap", bgx, bgy, bgw, bgh );
 bg.video_flags = Vid.ImagesOnly;
}
if ( my_config["bg_image"] == "title") {
 local bg = fe.add_artwork( "title", bgx, bgy, bgw, bgh );
}
if ( my_config["bg_image"] == "fanart") {
 fe.load_module( "fade" );
 local bg = FadeArt( "fanart", bgx, bgy, bgw, bgh );
}
if ( my_config["bg_image"] == "user") {
 local bg = fe.add_image( "bg.jpg", 0, 0, lw, lh);
}
local bgmask = fe.add_image ("bgmask.png", 0, 0, lw, lh);
// Preview image
local pvx=lw*0.02;
local pvy=lh*0.15;
local pvw=lw*0.55;
local pvh=(pvw/4)*3;
if ( my_config["preview_image"] == "video") {
 local previewoutline = fe.add_image ("black.png",pvx-1,pvy-1,pvw+2,pvh+2);
 local preview = fe.add_artwork( "snap", pvx, pvy, pvw, pvh);
}
if ( my_config["preview_image"] == "snap") {
 local previewoutline = fe.add_image ("black.png",pvx-1,pvy-1,pvw+2,pvh+2);
 local preview = fe.add_artwork( "snap", pvx, pvy, pvw, pvh);
 preview.video_flags = Vid.ImagesOnly;
}
if ( my_config["preview_image"] == "title") {
 local previewoutline = fe.add_image ("black.png",pvx-1,pvy-1,pvw+2,pvh+2);
 local preview = fe.add_artwork( "title", pvx, pvy, pvw, pvh);
}
// Game title background
local titlemask = fe.add_image ("titlemask.png", 0, 198, lw, lh-198);
titlemask.set_rgb (0,0,0);
// Flyer image
if ( my_config["enable_flyer"] == "Yes") {
 local flw=lw*0.4;
 local flh=170;
 local flx=(lw-flw)-(lw*0.02);
 local fly=20;
 local flyeroutline = fe.add_artwork( "flyer", flx, fly, flw, flh);
 flyeroutline.preserve_aspect_ratio = true;
 flyeroutline.rotation = my_config["flyer_angle"].tofloat();
// flyeroutline.x = flyeroutline.x + (my_config["flyer_angle"].tointeger())*20;
// flyeroutline.y = flyeroutline.y - my_config["flyer_angle"].tointeger();  
 local flyer = fe.add_clone( flyeroutline );
 flyeroutline.x = flyeroutline.x - 1;
 flyeroutline.y = flyeroutline.y - 1;
 flyeroutline.width = flyeroutline.width + 2;
 flyeroutline.height = flyeroutline.height + 2;
 flyeroutline.set_rgb (0,0,0);
 flyeroutline.alpha = 192;
}
// Game title text
local gametitleshadow = fe.add_text( gamename ( 0 ), 11, 203, lw - 10, 15 );
gametitleshadow.align = Align.Left;
gametitleshadow.set_rgb (0,0,0);
local gametitle = fe.add_text( gamename ( 0 ), 10, 202, lw - 10, 15 );
gametitle.align = Align.Left;
// Make the game title flicker. Added bonus - currently fixes graphical corruption and screen not refreshing bugs.
fe.add_ticks_callback("flickertitle");
function flickertitle( ttime ) {
 if ( my_config["enable_flicker"] == "Yes" ) {
  grey = highrand( 1024 );
  gametitle.set_rgb (grey,grey,grey);
 } else {
  gametitle.set_rgb (255,255,255);
 }
}
local copy = fe.add_text( copyright ( 0 ), 12, 217, lw - 30, 10 );
copy.align = Align.Left;
// Game logo image
local logox = 7;
switch ( my_config["logo_position"] ) {
 case "Centre":
  logox = 62;
  break;
 case "Right":
  logox = 113;
}
local logoshadow = fe.add_artwork( "wheel", logox, 40, 200, 100);
logoshadow.preserve_aspect_ratio = true;
logoshadow.set_rgb(0,0,0);
logoshadow.alpha = 192;
local logo = fe.add_clone( logoshadow );
logo.set_rgb (255,255,255);
logo.x = logo.x - 2;
logo.y = logo.y - 2;
logo.alpha=255; 
// Loading screen message.
//local message = fe.add_text("Loading...",0,100,lw,40);
//message.alpha = 0;
// Optional game texts
local romlist = fe.add_text( "[DisplayName]", 2, 202, 315, 10 );
romlist.align = Align.Right;
local cat = fe.add_text( fe.game_info (Info.Category), 2, 212, 315, 10 );
cat.align = Align.Right;
local entries = fe.add_text( "[ListEntry]/[ListSize]", 2, 222, 315, 10 );
entries.align = Align.Right;
local filter = fe.add_text( "[FilterName]", -20, 190, lw+40, lh-191 );
filter.align = Align.Right;
filter.alpha = 32;
// Switch texts on and off
if ( my_config["enable_category"] == "Yes" ) { cat.visible = true; } else { cat.visible = false; }
if ( my_config["enable_list"] == "Yes" ) { romlist.visible = true; } else { romlist.visible = false; }
if ( my_config["enable_filter"] == "Yes" ) { filter.visible = true; } else { filter.visible = false; }
if ( my_config["enable_entries"] == "Yes" ) { entries.visible = true; } else { entries.visible = false; }
// Transitions
fe.add_transition_callback( "fade_transitions" );
function fade_transitions( ttype, var, ttime ) {
 switch ( ttype ) {
  case Transition.ToNewList:
   var = 0;
  case Transition.ToNewSelection:
   gametitleshadow.msg = gamename ( var );
   gametitle.msg = gametitleshadow.msg;
   copy.msg = copyright ( var );
   cat.msg = fe.game_info (Info.Category, var);  
   red = highrand( 255 );
   green = highrand( 255 );
   blue = highrand( 255 );
   romlist.set_rgb (red,green,blue);
   filter.set_rgb (red,green,blue);
   entries.set_rgb (red,green,blue);
   copy.set_rgb (red,green,blue);
   cat.set_rgb (red,green,blue);
   break;
//  case Transition.FromGame:
//   if ( ttime < 255 ) {
//    foreach (o in fe.obj) o.alpha = ttime;
//    message.alpha = 0;     
//    return true;
//   } else {
//    foreach (o in fe.obj) o.alpha = 255;
//    message.alpha = 0;
//   }
//   break;  
//  case Transition.EndLayout:
//   if ( ttime < 255 ) {
//    foreach (o in fe.obj) o.alpha = 255 - ttime;
//    message.alpha = 0; 
//    return true;
//   } else {
//    foreach (o in fe.obj) o.alpha = 255;
//    message.alpha = 0;
//   }
//   break;
//  case Transition.ToGame:
//   if ( ttime < 255 ) {
//    foreach (o in fe.obj) o.alpha = 255 - ttime;
//    message.alpha = ttime;
//    return true;
//   }   
//   break; 
  }
 return false;
}
