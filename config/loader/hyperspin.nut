/////////////////////////////////////////////////////////
//
// Attract-Mode EXPERIMENTAL Hyperspin Theme Loader
//
// Copyright (c) 2016 Andrew Mickelson.
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
// GETTING STARTED INSTRUCTIONS
//
// 1.) Copy the Hyperspin "Media" directory into its own directory in
//     the Attract-Mode layouts directory.
//
// 2.) Create a Display in Attract-Mode and configure the layout to be the
//     directory you copied "Media" into. The Display's name needs
//     to match the name of one of the system subdirectories in the
//     Hyperspin "Media" directory.  This will allow Attract-Mode to find
//     the Hyperspin themes and graphics to use.  So for example, naming the
//     Display "MAME" will cause it to match Hyperspin's MAME/Themes/* for
//     themes, MAME/Images/Artwork1/* for artwork1, etc.
//
// 3.) Wheel images are located using Attract-Mode's built in wheel artwork,
//     the <System>/Images/Wheel/* files from Hyperspin are ignored.
//
// 4.) To use the "Displays Menu" as a Hyperspin Systems list: make sure that
//     the "Theme the Displays Menu" layout option is set to "yes".Hyperspin's
//     "Main Menu" subdirectory is used to get artworks and themes when
//     showing the "Displays Menu".  So for example Main Menu/Themes/*
//     for Themes, Main Menu/Images/Artwork1/* for artwork1, etc.  Theme and
//     artwork names need to match the corresponding Display's name in order
//     to be used.
// 
// KNOWN ISSUES:
//
// - Only proof-of-concept at this point, there are many, many Hyperspin
//   features that are unimplemented and completely missing
//
/////////////////////////////////////////////////////////
class UserConfig </ help="Hyperspin Layout: " + fe.script_dir + ::file_to_load />
{
   </ label="Override Transitions", help="Do override transition effects", options="yes,no", order=1 />
   overrides="yes";

   </ label="Override Lag", help="Time to wait after override starts before switching artwork (in ms)", order=2 />
   override_lag_ms="0";

   </ label="Wheel Speed", help="Wheel spin speed (in ms)", order=3 />
   wheel_ms="100";

   </ label="Theme the Displays Menu", help="Use Hyperspin themes for the Displays Menu?", order=4, options="yes,no" />
   displays_menu="yes";

   </ label="Show Flyers", help="Show flyers in Artwork2", order=5, options="yes,no" />
   show_flyers="yes";

   </ label="Show Prompts", help="Show Hyperspin prompts", order=6, options="yes,no" />
   show_prompts="yes";

   </ label="Animation Speed", help="Set animation speed", order=7, options="default,2X,4X" />
   speed="4X";

   </ label="Fix SWF positioning", help="Try to fix flash swf artwork files positions", order=8, options="yes,no" />
   fix_swf_positions="yes";

   </ label="Wheel Hide Time", help="The time (in milliseconds) that it takes for the wheel to fade.  Set to 0 for no fade", order=9 />
   wheel_fade_ms="1000";
};

local my_config = fe.get_config();

fe.load_module( "file" );
fe.load_module( "file-format" );
fe.load_module( "animate" );

fe.layout.width = 1024;
fe.layout.height = 768;

/////////////////////////////////////////////////////////
//
// Global variables
//
/////////////////////////////////////////////////////////
wheel_ms <- 100;
override_lag_ms <- 0;
work_d <- fe.script_dir + ::file_to_load;	// base directory
hs_systems <- [];	// list of the subdirectories in work_d
current_theme <- ""; // path of the currently loaded theme file
current_theme_root <- null; // root "Theme" node of the current theme
hs_ent <- {}; // table storing the Attract-Mode objects used for display

// indices of the Displays to display when showing the "Displays Menu"
dm_map <- [];

/////////////////////////////////////////////////////////
//
// Minor initialization of globals
//
/////////////////////////////////////////////////////////
try
{
	override_lag_ms = my_config["override_lag_ms"].tointeger();
}
catch ( e ) { }

local temp = DirectoryListing( work_d, false );
foreach ( f in temp.results )
	hs_systems.append( f );

for ( local i=0; i< fe.displays.len(); i++ )
{
	if ( fe.displays[i].in_menu )
		dm_map.push( i );
}

//
// Zorder (?) To be confirmed:
//
// - background
// - art1
// - (video_overlay if overlaybelow=true)
// - video 
// - video_overlay
// - (art1 if below=true)
// - art2
// - art3
// - art4
// - (wheel)
// - (prompts)
// - override transitions
//
::hs_ent.background <- {
		dir = "Images/Backgrounds/"
		obj = fe.add_image( "" )
		zorder = -10
	};

::hs_ent.artwork1 <- {
		dir = "Images/Artwork1/"
		obj = fe.add_image( "" )
		zorder = -9
	};

// zorder -8 - hs_ent.video_overlay zorder goes here if overlaybelow=true

::hs_ent.video <- {
		dir = "Video/"
		obj = fe.add_image( "" )
		zorder = -7
	};

::hs_ent.video_overlay <- {
		dir = ""
		obj = fe.add_image( "" )
		zorder = -6
	};

// zorder -5 - hs_ent.artwork1 zorder goes here if below=true

::hs_ent.artwork2 <- {
		dir = "Images/Artwork2/"
		obj = fe.add_image( "" )
		zorder = -4
	};

::hs_ent.artwork3 <- {
		dir = "Images/Artwork3/"
		obj = fe.add_image( "" )
		zorder = -3
	};

::hs_ent.artwork4 <- {
		dir = "Images/Artwork4/"
		obj = fe.add_image( "" )
		zorder = -2
	};

// wheel and prompts zorder is at 0

::hs_ent.override_transition <- {
		dir = "Video/Override Transitions/"
		obj = fe.add_image( "", 0, 0, fe.layout.width, fe.layout.height )
		zorder = 9999
	};

//
// top_label gets used if no theme is found
//
::top_label <- fe.add_text( "", 0, 0, fe.layout.width, fe.layout.height );
::top_label.charsize = 48;
::top_label.word_wrap = true;

/////////////////////////////////////////////////////////
//
// Functions definitions
//
/////////////////////////////////////////////////////////
function strip_ext( name )
{
	local s = split( name, "." );

	if ( s.len() == 1 )
		return name;
	else
	{
		local retval;

		retval = s[0];

		for ( local i=1; i<s.len()-1; i++ )
			retval = retval + "." + s[i];

		return retval;
	}
}

function ext( name )
{
	local s = split( name, "." );

	if ( s.len() <= 1 )
		return "";
	else
		return s[s.len()-1];
}

//
// idx is -1 if getting system match map for the current game.  
// other idx values are for the system match map for the displays menu.  
//
function get_system_match_map( idx=-1 )
{
	local i = idx;
	if ( i == -1 )
	{
		i = fe.list.display_index;
		if ( i<0 )
			i=0;
	}

	local smm = [];
	smm.push( fe.displays[i].name.tolower() );

	if ( idx < 0 )
	{
		if ( fe.list.display_index < 0 ) // this means we are showing the 'displays menu' w/ custom layout
			smm.push( fe.game_info( Info.AltRomname ) );
		else
			smm.push( fe.game_info( Info.Emulator ) );

		foreach ( t in split( fe.game_info( Info.System), ";" ) )
			smm.push( t );
	}

	return smm;
}

function get_match_map( index_offset=0 )
{
	local mm = [];
	mm.push( fe.game_info( Info.Name, index_offset ).tolower() );
	if ( fe.game_info( Info.AltRomname ).len() > 0 )
		mm.push( fe.game_info( Info.AltRomname, index_offset ).tolower() );
	if ( fe.game_info( Info.CloneOf ).len() > 0 )
		mm.push( fe.game_info( Info.CloneOf, index_offset ).tolower() );

	return mm;
}

function get_hs_system_dir()
{
	local match_map = get_system_match_map(-1);
	local sys_d = "";

	foreach ( s in hs_systems )
	{
		foreach ( m in match_map )
		{
			if ( m == s.tolower() )
			{
				sys_d = work_d + s + "/";
				break;
			}
		}

		if ( sys_d.len() > 0 )
			break;
	}

	if ( sys_d.len() < 1 )
	{
		print( "Couldn't find matching hyperspin system, looking for: " );
		foreach ( m in match_map )
			print( m + " " );

		print( "\n" );

		sys_d = work_d + hs_systems[0] + "/";
	}

	return sys_d;
}

function get_theme_file( theme_d, match_map )
{
	local default_theme="";
	local theme = "";
	local temp = DirectoryListing( theme_d, false );

	foreach ( t in temp.results )
	{
		local temp = strip_ext( t ).tolower();

		foreach ( m in match_map )
		{
			if ( m == temp )
			{
				theme = t;
				break;
			}
		}

		if ( theme.len() > 0 )
			break;

		if ( temp == "default" )
			default_theme = t;
	}

	if ( theme.len() < 1 )
	{
		if ( default_theme.len() < 1 )
		{
			print( "Couldn't find hyperspin theme or default: "
				+ fe.game_info( Info.Name ) + " (" + theme_d + ")\n" );
		}

		theme = default_theme;
	}

	if ( ( theme.len() > 1 ) && !IS_ARCHIVE( theme ) )
		theme += "/";

	return theme;
}

function get_art_file( tag, sys_d, theme_fname, match_map )
{
	if ( tag != "video_overlay" )
	{
		local temp = DirectoryListing( sys_d + ::hs_ent[tag].dir, false );

		foreach ( t in temp.results )
		{
			local temp = strip_ext( t ).tolower();

			foreach ( m in match_map )
			{
				if ( m == temp )
					return sys_d + ::hs_ent[tag].dir + t;
			}
		}
	}

	if (( theme_fname.len() > 0 ) && ( tag != "video" ))
	{
		// check in the theme file
		local temp2 = DirectoryListing( theme_fname, false );

		local test = ( tag == "video_overlay" ) ? "video" : tag;
		foreach ( f in temp2.results )
		{
			if ( strip_ext( f ).tolower() == test )
				return theme_fname + "|" + f;
		}
	}

	//
	// Pick some fallback artworks from what we have in Attract-Mode
	//
	switch ( tag )
	{
		case "video":
			return fe.get_art( "snap" );

		case "artwork2":
			if ( my_config["show_flyers"] == "yes" )
				return fe.get_art( "flyer" );
			break;

		default:
			break;
	}
	return "";
}

function find_theme_node( node )
{
	if ( node.tag == "Theme" )
		return node;

	foreach ( c in node.children )
	{
		local n = find_theme_node( c );
		if ( n ) return n;
	}

	return null;
}

// return true if we should call into the animate on_transition function
function load( ttype, match_map, hs_sys_dir )
{
	local theme_d = hs_sys_dir + "Themes/";
	local theme_f = get_theme_file( theme_d, match_map );

	local theme = theme_d + theme_f;
//	print( " - Loading theme: " + theme + "\n" );

	//
	// Default all to not visible
	//
	foreach ( k,e in ::hs_ent )
	{
		if ( k != "override_transition" )
		{
			e.obj.visible = false;
			e.obj.zorder = e.zorder;
			e.obj.alpha = 255;
		}
	}

	::hs_ent["video"].obj.preserve_aspect_ratio = true;

	//
	// Set up background
	//
	local bg = get_art_file( "background", hs_sys_dir, theme, match_map );
	if ( bg.len() > 0 )
	{
		local bg_obj = ::hs_ent["background"].obj;
		bg_obj.file_name = bg;
		bg_obj.visible = true;
		bg_obj.width = fe.layout.width;
		bg_obj.height = fe.layout.height;
	}

	// handle situation where no theme has been found
	//
	if ( theme_f.len() < 1 )
	{
		::hs_ent["video"].obj.video_playing = false;
		::hs_ent["video"].obj.file_name = "";
		top_label.visible = true;
		top_label.msg = match_map[0];

		return;
	}
	else
		top_label.visible = false;

	if ( theme != current_theme )
	{
		local f = ReadTextFile( theme, "Theme.xml" );

		local raw_xml = "";
		while ( !f.eos() )
			raw_xml = raw_xml + f.read_line();

		// root tag = "Theme"
		local xml_root = null;
		try
		{
			xml_root = xml.load( raw_xml );
		}
		catch ( e ) {};

		if ( !xml_root )
		{
			print( "Error parsing XML: " + raw_xml + "\n" );
			return false;
		}

		current_theme_root = find_theme_node( xml_root );
	}

	//
	hs_animation.animations.clear();
	local call_into_transition=false;

	// tags where we need to clear the resource if we don't find them
	// defined in this theme
	//
	local found_tags = {
		artwork1 = false
		artwork2 = false
		artwork3 = false
		artwork4	= false
		video		= false
	};

	local below=false;

	foreach ( c in current_theme_root.children )
	{
		if ( ::hs_ent.rawin( c.tag ) )
		{
			if ( found_tags.rawin( c.tag ) )
				found_tags[ c.tag ] = true;

			local obj = ::hs_ent[c.tag].obj;
			local obj2 = null;
			obj.visible = true;

			if ( c.tag == "video" )
				::hs_ent["video_overlay"].obj.visible=true;

			local af = get_art_file( c.tag, hs_sys_dir, theme, match_map );
			if ( af == obj.file_name )
				continue;

			obj.file_name = af;

			local x=0;
			local y=0;
			local w=obj.texture_width;
			local h=obj.texture_height;
			local r=0;
			local t=0.0;
			local d=0.0;
			local type = "";
			local start = "";
			local overlay_below=false;
			local overlayoffsetx = 0;
			local overlayoffsety = 0;

			foreach ( k,v in c.attr )
			{
				switch ( k )
				{
				case "x": x=v.tointeger(); break;
				case "y": y=v.tointeger(); break;
				case "w": w=v.tointeger(); break;
				case "h": h=v.tointeger(); break;
				case "r": r=v.tointeger(); break;
				case "time": t=v.tofloat(); break;
//				case "delay": d=v.tofloat(); break;
				case "type": type = v; break;
				case "start": start = v; break;
				case "overlayoffsetx": overlayoffsetx=v.tointeger(); break;
				case "overlayoffsety": overlayoffsety=v.tointeger(); break;
				case "below":
				if ( v == "true" )
					below = true;
					break;
				case "overlaybelow":
					if ( v == "true" )
						overlay_below = true;
					break;
				case "forceaspect":
					if (( c.tag == "video" ) && ( v == "both" ))
						obj.preserve_aspect_ratio=false;
				default:
					break;
				}
			}

			obj.x        = x - w/2;
			obj.y        = y - h/2;

			obj.width    = w;
			obj.height   = h;
			obj.rotation = r;

			//fixes for wrong positions for SWF files
			if ( my_config["fix_swf_positions"] == "yes" && ext( obj.file_name ).tolower() == "swf")
			{
				//beast busters - bombjack - gladiator
				if (x > fe.layout.width || y > fe.layout.height)
				{
					obj.y = 0;
					obj.x = 0;
				}

				//most others
				if (obj.width == fe.layout.width)
				{
					if (obj.x < 0)
						obj.x = 0;
				}

				//most others
				if (obj.height == fe.layout.height)
				{
					if (obj.y < 0)
						obj.y = 0;
				}
			}

			if ( c.tag == "video" )
			{
				// Deal with video overlay
				//
				local af2
					= get_art_file( "video_overlay", hs_sys_dir, theme, match_map );

				obj2 = ::hs_ent["video_overlay"].obj;
				obj2.file_name = af2;
				obj2.x        = x - obj2.texture_width/2 + overlayoffsetx;
				obj2.y        = y - obj2.texture_height/2 + overlayoffsety;
				obj2.width    = obj2.texture_width;
				obj2.height   = obj2.texture_height;
				obj2.rotation = obj.rotation;

				if ( overlay_below )
					obj2.zorder = ::hs_ent["video"].zorder-1;
			}

			if ( r != 0 )
			{
				// Hyperspin rotates around the centre of the image
				// (instead of top left corner), so correct for that
				//
				local mr = PI * r / 180;
				obj.x += cos( mr ) * (-w/2) - sin( mr ) * (-h/2) + w/2;
				obj.y += sin( mr ) * (-w/2) + cos( mr ) * (-h/2) + h/2;

				if ( obj2 )
				{
					local w2 = obj2.width;
					local h2 = obj2.height;
					obj2.x += cos( mr ) * (-w2/2) - sin( mr ) * (-h2/2) + w2/2;
					obj2.y += sin( mr ) * (-w2/2) + cos( mr ) * (-h2/2) + h2/2;
				}
			}

			local tint = (t * 1000);
			local dint = (d * 1000).tointeger();

			if ( my_config["speed"] == "4X" )
			{
				tint /= 4;
				dint /= 4;
			}
			else if ( my_config["speed"] == "2X" )
			{
				tint /= 2;
				dint /= 2;
			}

			switch ( type )
			{
			case "fade":
				if ( tint > 0 )
				{
					local cfg = {
						when = ttype,
						property="alpha",
						start=0,
						end =255,
						time = tint,
						delay = dint
					};

					hs_animation.add( PropertyAnimation( obj, cfg ) );
					if ( obj2 )
						hs_animation.add( PropertyAnimation( obj2, cfg ) );
					call_into_transition=true;
				}
				break;
			case "ease":
			case "elastic":
			case "elastic bounce":
				if ( tint > 0 )
				{
					local vert = ((start=="top") || (start=="bottom"));
					local flip = ((start=="bottom") || (start=="right"));

					local tw = "linear";
					if ( type == "elastic bounce" )
						tw = "bounce";
					else if ( type == "elastic" )
						tw = "elastic";

					local cfg = {
						when = ttype,
						property= vert ? "y" : "x",
						start = flip ?
							( vert ? fe.layout.height : fe.layout.width )
							: ( vert ? -h : -w ),
						end = vert ? y - h/2 : x - w/2,
						time = tint,
						delay = dint,
						tween = tw,
						easing = Easing.In
					};

					hs_animation.add( PropertyAnimation( obj, cfg ) );
					if ( obj2 )
					{
						local cfg2 = clone cfg;
						cfg2["end"] = vert ? y - obj2.height/2 : x - obj2.width/2;
						hs_animation.add( PropertyAnimation( obj2, cfg2 ) );
					}
					call_into_transition=true;
				}
				break;
			case "grow x":
			case "grow":
			case "grow bounce":
				if ( tint > 0 )
				{
					local cfg = {
						when = ttype,
						property= "width",
						start=0,
						end = w,
						tween = ( type == "grow bounce" ) ? "bounce" : "linear",
						time = tint,
						delay = dint
					};

					hs_animation.add( PropertyAnimation( obj, cfg ) );

					if ( obj2 )
					{
						local cfg2 = clone cfg;
						cfg2["end"] = obj2.width;
						hs_animation.add( PropertyAnimation( obj2, cfg2 ) );
					}

					if ( type != "grow x" )
					{
						local cfg2 = {
							when = ttype,
							property= "x",
							start=x-w/2,
							end = x-w/2,
							time = tint,
							delay = dint
						};

						hs_animation.add( PropertyAnimation( obj, cfg2 ) );
						if ( obj2 )
						{
							local cfg3 = clone cfg2;
							cfg3["start"] = x - obj2.width/2;
							cfg3["end"] = x - obj2.width/2;
							hs_animation.add( PropertyAnimation( obj2, cfg3 ) );
						}
					}
					call_into_transition=true;
				}
				break;

			case "grow y":
				if ( tint > 0 )
				{
					local cfg = {
						when = ttype,
						property= "height",
						start=0,
						end = h,
						time = tint,
						delay = dint
					};

					hs_animation.add( PropertyAnimation( obj, cfg ) );
					if ( obj2 )
					{
						local cfg2 = clone cfg;
						cfg2["end"] = x - obj2.height;
						hs_animation.add( PropertyAnimation( obj2, cfg2 ) );
					}
					call_into_transition=true;
				}
				break;
			}
		}
	}

	foreach ( k,v in found_tags )
	{
		if ( !v )
			::hs_ent[k].obj.file_name = "";
	}

	//below seems to swap zorder of video + overlay with artwork 1
	//
	if ( below && found_tags["artwork1"] )
		::hs_ent["artwork1"].obj.zorder = ::hs_ent["video_overlay"].zorder + 1;

	return call_into_transition;
}

function setup_prompts( is_display_menu )
{
	if ( my_config["show_prompts"] != "yes" )
		return;

	if ( !is_display_menu )
	{
		special1.file_name = work_d
			+ "Main Menu/Images/Special/SpecialB1.swf";
		special1.set_pos( 0, 5 );

		special2.file_name = work_d +
			"Main Menu/Images/Special/SpecialB2.swf";
		special2.set_pos( 0, fe.layout.height-special2.texture_height-5 );
	}
	else
	{
		special1.file_name = work_d
			+ "Main Menu/Images/Special/SpecialA1.swf";
		special1.set_pos( 60, fe.layout.height-special1.texture_height-10 );

		special2.file_name = work_d +
			"Main Menu/Images/Special/SpecialA2.swf";
		special2.set_pos( fe.layout.width-special2.texture_width-60,
			fe.layout.height-special2.texture_height-10 );
	}
}

/////////////////////////////////////////////////////////
//
// Now put it all together...
//
/////////////////////////////////////////////////////////

print( "*** Attract-Mode EXPERIMENTAL HyperSpin Theme loader.\n*** Now Spinning: " + work_d + "\n" );

local doing_display_menu = false;
if ( my_config["displays_menu"] == "yes" )
	fe.add_signal_handler( "hs_signal_handler" );

//
// Signal handler (for dealing with Displays Menu)
//
function hs_signal_handler( signal )
{
	if ( signal == "displays_menu" )
	{
		// Let the frontend know that we will manage the displays menu
		// ourself
		fe.overlay.set_custom_controls();
	}
}

//
// Transition management
//
override_first_part <- false;
navigate_in_progress <- false;

function load_override_transition( sys_d, match_map )
{
	local over_file = get_art_file(
		"override_transition", sys_d, current_theme, match_map );

	if ( over_file.len() > 0 )
	{
		local o = ::hs_ent["override_transition"].obj;

		o.file_name = over_file;
		o.video_flags = Vid.NoLoop;
		o.width = fe.layout.width;
		o.height = fe.layout.height;
		o.visible = true;

		override_first_part = true;
		return true;
	}
	return false;
}

// if call_animate is true, we simply pass transitions along to the
// animate module's transition handling
//
local call_animate=false;
local w_alpha_clock = 0;
local w_alpha_triggered = 0;

fe.add_transition_callback( "hs_transition" );
function hs_transition( ttype, var, ttime )
{
	if ( call_animate )
	{
		// As of this writing the animate module's transition function never
		// returns true so this part is unlikely to ever be used...
		//
		if ( hs_animation.transition_callback( ttype, var, ttime ) )
			return true;

		call_animate = false;
		return false;
	}

	switch ( ttype )
	{
	case Transition.ToNewList:
	case Transition.EndNavigation:
		navigate_in_progress = false;

		setup_prompts( false );
		local hs_sys = get_hs_system_dir();

		if ( fe.list.display_index < 0 ) // this means we are showing the 'displays menu' w/ custom layout
			hs_sys = work_d + "Main Menu/";

		local mm = get_match_map();

		if ( override_lag_ms <= 0 )
			load_override_transition( hs_sys, mm );

		if ( load( ttype, mm, hs_sys ) )
		{
			call_animate = hs_animation.transition_callback( ttype, var, ttime );
			return call_animate;
		}
		break;

	case Transition.ToNewSelection:
		w_alpha_triggered = w_alpha_clock;
		if ( override_lag_ms <=  0 )
			break;

		if ( override_first_part )
		{
			//
			// Show the override video for override_lag_ms before starting
			// the switch to the new selection
			//
 			if ( ttime < override_lag_ms )
				return true;

			override_first_part = false;
			return false;
		}

		//
		// Don't start a new override if still navigating...
		//
		if ( navigate_in_progress )
			return false;

		navigate_in_progress = true;
		return load_override_transition(
			get_hs_system_dir(), get_match_map( var ) );

	default:
		break;
	}

	if ( my_config["displays_menu"] == "yes" )
	{
		//
		// Code to Hyperspin-ize the "displays menu"
		//
		if (( ttype == Transition.ShowOverlay ) && ( var == Overlay.Displays ))
		{
			doing_display_menu = true;
			wheel.visible = false; // hide the wheel
			setup_prompts( true );

			if ( load( ttype,
				get_system_match_map( fe.list.display_index ),
				work_d + "Main Menu/" ) )
			{
				call_animate = hs_animation.transition_callback( ttype, var, ttime );
				return call_animate;
			}
		}
		else if ( doing_display_menu )
		{
			if ( ttype == Transition.HideOverlay )
			{
				doing_display_menu = false;
				fe.overlay.clear_custom_controls();

				wheel.visible = true;
				setup_prompts( false );

				if ( load( ttype, get_match_map(), get_hs_system_dir() ) )
				{
					// It seems we need to force a different transition type here?
					//
					call_animate = hs_animation.transition_callback(
						Transition.ToNewList, var, ttime );
					return call_animate;
				}
			}
			else if ( ttype == Transition.NewSelOverlay )
			{
				if ( var >= dm_map.len() )
				{
					// Handle Exit option on the Displays Menu
					//
					foreach ( o in ::hs_ent )
					{
						o.obj.visible = false;
						o.obj.file_name = "";
					}

					::top_label.visible = false;

					::hs_ent["background"].obj.file_name = work_d
						+ "Frontend/Images/Menu_Exit_Background.png";
					::hs_ent["background"].obj.visible = true;

					return false;
				}

				local mm = get_system_match_map( dm_map[var] );

				load_override_transition( work_d + "Main Menu/", mm );

				if ( load( ttype, mm, work_d + "Main Menu/" ) )
					call_animate = hs_animation.transition_callback( ttype, var, ttime );

				return call_animate;
			}
		}
	}

	return false;
}

//
// Ticks
//
fe.add_ticks_callback( "hs_tick" );
function hs_tick( ttime )
{
	local o = ::hs_ent["override_transition"].obj;
	if ( o.visible && !o.video_playing )
	{
		o.visible = false;
		o.file_name = "";
	}
}

//
// Set up the wheel
//
fe.load_module( "conveyor" );

local flw = fe.layout.width;
local flh = fe.layout.height;
local wheel_w = 300;
local wheel_x = [ flw*0.70, flw*0.695, flw*0.656, flw*0.625, flw*0.60, flw*0.58, flw*0.54, flw*0.58, flw*0.60, flw*0.625, flw*0.656, flw*0.66, ];
local wheel_y = [ -flh*0.22, -flh*0.105, flh*0.0, flh*0.105, flh*0.215, flh*0.325, flh*0.436, flh*0.61, flh*0.72 flh*0.83, flh*0.935, flh*0.99, ]
local wheel_w = [ wheel_w, wheel_w, wheel_w, wheel_w, wheel_w, wheel_w, wheel_w*1.5, wheel_w, wheel_w, wheel_w, wheel_w, wheel_w ];
local wheel_a = [  130,  130,  130,  130,  130,  130, 255,  130,  130,  130,  130,  130, ];
local wheel_r = [  30,  25,  20,  15,  10,   5,   0, -10, -15, -20, -25, -30, ];
//
// Wheel alpha
//
local wheel_fade_ms = 0;
try { wheel_fade_ms = my_config["wheel_fade_ms"].tointeger(); } catch ( e ) { }

if ( wheel_fade_ms > 0 )
	fe.add_ticks_callback( "hs_wheel_alpha" );

function hs_wheel_alpha( ttime )
{
	w_alpha_clock = ttime;

	local w_alpha = wheel_fade_ms - ( w_alpha_clock - w_alpha_triggered );
        if (w_alpha < 0) {
            w_alpha = 0;
        }
        for (local i=0; i < wheel.m_objs.len(); i++) {
            if (i == 5) {
                if (w_alpha < wheel_a[i+1]) {
                    wheel.m_objs[i].alpha = w_alpha;
                }
            } else if (w_alpha <= 255 + wheel_a[i+1]) {
if (w_alpha >= 255) {
                wheel.m_objs[i].alpha = w_alpha - 255;
} else {
                wheel.m_objs[i].alpha = 0;
}
            }
        }
}

class WheelEntry extends ConveyorSlot
{
	constructor()
	{
		base.constructor( ::fe.add_artwork( "wheel" ) );
	}

	function on_progress( progress, var )
	{
		local p = progress / 0.1;
		local slot = p.tointeger();
		p -= slot;
		slot++;

		if ( slot < 0 ) slot=0;
		if ( slot >= 10 ) slot=10;

		m_obj.x = wheel_x[slot] + p * ( wheel_x[slot+1] - wheel_x[slot] );
		m_obj.y = wheel_y[slot] + p * ( wheel_y[slot+1] - wheel_y[slot] );

		m_obj.width = wheel_w[slot] + p * ( wheel_w[slot+1] - wheel_w[slot] );
		m_obj.height = 0;
		m_obj.preserve_aspect_ratio=true;

		m_obj.rotation = wheel_r[slot] + p * ( wheel_r[slot+1] - wheel_r[slot] );
		m_obj.alpha = wheel_a[slot] + p * ( wheel_a[slot+1] - wheel_a[slot] );
	}
};

local num_arts = 10;
local wheel_entries = [];
for ( local i=0; i<num_arts/2; i++ )
	wheel_entries.push( WheelEntry() );

local remaining = num_arts - wheel_entries.len();

// we do it this way so that the last wheelentry created is the middle one showing the current
// selection (putting it at the top of the draw order)
for ( local i=0; i<remaining; i++ )
	wheel_entries.insert( num_arts/2, WheelEntry() );

wheel <- Conveyor();
wheel.set_slots( wheel_entries );
wheel.transition_ms = wheel_ms;

if ( my_config["show_prompts"] == "yes" )
{
	::special1 <- fe.add_image( "" );
	::special2 <- fe.add_image( "" );
}

// make sure the override transition object is drawn over the wheel
::hs_ent["override_transition"].obj.zorder=9999;

hs_animation <- AnimationCore();
//
// don't register the AnimationCore transition callback, our transition
// callback will call into AnimationCore itself
//
fe.add_ticks_callback( hs_animation, "ticks_callback" );
