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
// - Doesn't play well with plugins that may be using the animate module
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
   override_lag_ms="400";

   </ label="Wheel Speed", help="Wheel spin speed (in ms)", order=3 />
   wheel_ms="100";

   </ label="Theme the Displays Menu", help="Use Hyperspin themes for the Displays Menu?", order=4, options="yes,no" />
   displays_menu="yes";

   </ label="Show Flyers", help="Show flyers in Artwork2", order=5, options="yes,no" />
   show_flyers="yes";
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
override_lag_ms <- 100;
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
// - art2
// - art1
// - video 
// - art3
// - art4
//
::hs_ent.background <- {
		dir = "Images/Backgrounds/"
		obj = fe.add_image( "" )
		zorder = 0
	};

::hs_ent.artwork2 <- {
		dir = "Images/Artwork2/"
		obj = fe.add_image( "" )
		zorder = 1
	};

::hs_ent.artwork1 <- {
		dir = "Images/Artwork1/"
		obj = fe.add_image( "" )
		zorder = 2
	};

::hs_ent.video <- {
		dir = "Video/"
		obj = fe.add_image( "" )
		zorder = 3
	};

::hs_ent.artwork3 <- {
		dir = "Images/Artwork3/"
		obj = fe.add_image( "" )
		zorder = 4
	};

::hs_ent.artwork4 <- {
		dir = "Images/Artwork4/"
		obj = fe.add_image( "" )
		zorder = 5
	};

::hs_ent.override_transition <- {
		dir = "Video/Override Transitions/"

		// override transition video gets loaded into obj on initialization,
		// but doesn't get played.  It gets swapped to obj2 to play when
		// the transition actually occurs.
		//
		obj = fe.add_image( "", 0, 0, fe.layout.width, fe.layout.height )
		zorder = 9999
	};

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

//
// idx is -1 if getting system match map for the current game.  
// other idx values are for the system match map for the displays menu.  
//
function get_system_match_map( idx=-1 )
{
	local i = idx;
	if ( i == -1 )
		i = fe.list.display_index;

	local smm = [];
	smm.push( fe.displays[i].name.tolower() );

	if ( idx < 0 )
	{
		foreach ( t in split( fe.game_info( Info.System), ";" ) )
			smm.push( t );
	}

	return smm;
}

function get_match_map()
{
	local mm = [];
	mm.push( fe.game_info( Info.Name ).tolower() );
	if ( fe.game_info( Info.AltRomname ).len() > 0 )
		mm.push( fe.game_info( Info.AltRomname ).tolower() );
	if ( fe.game_info( Info.CloneOf ).len() > 0 )
		mm.push( fe.game_info( Info.CloneOf ).tolower() );

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

function get_theme_file( sys_d, match_map )
{
	local default_theme="";
	local theme = "";
	local temp = DirectoryListing( sys_d + "Themes/", false );

	foreach ( t in temp.results )
	{
		local temp = strip_ext( t ).tolower();

		foreach ( m in match_map )
		{
			if ( m == temp )
			{
				theme = sys_d + "Themes/" + t;
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
				+ fe.game_info( Info.Name ) + "\n" );
		}

		theme = sys_d + "Themes/" + default_theme;
	}

	return theme;
}

function get_art_file( tag, sys_d, theme_fname, match_map )
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

	if ( theme_fname.len() > 0 )
	{
		// check in the theme file
		local temp2 = DirectoryListing( theme_fname, false );

		foreach ( f in temp2.results )
		{
			if ( strip_ext( f ).tolower() == tag )
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

function load( ttype, var, ttime, match_map, hs_sys_dir )
{
	local theme = get_theme_file( hs_sys_dir, match_map );
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
		}
	}

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
	// We abuse the animate module a bit, which will cause problems
	// for any plugins that do animation using the module as well.
	//
	animation.animations.clear();
	local call_into_transition=false;
	
	foreach ( c in current_theme_root.children )
	{
		if ( ::hs_ent.rawin( c.tag ) )
		{
			local obj = ::hs_ent[c.tag].obj;
			obj.visible = true;

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
			local type = "";
			local start = "";

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
				case "type": type = v; break;
				case "start": start = v; break;
				case "below":
					if ( v == "true" )
						obj.zorder = 1;
					break;
				default:
					break;
				}
			}

			if ( r != 0 )
			{
				// Hyperspin rotates around the centre of the image
				// (instead of top left corner), so correct for that
				//
				local mr = PI * r / 180;
				x += cos( mr ) * (-w/2) - sin( mr ) * (-h/2) + w/2;
				y += sin( mr ) * (-w/2) + cos( mr ) * (-h/2) + h/2;
			}

			obj.x        = x - w/2;
			obj.y        = y - h/2;
			obj.width    = w;
			obj.height   = h;
			obj.rotation = r;

			local tint = (t * 1000).tointeger();

			// Hack around animation times being too slow on Linux
			if ( OS == "Linux" )
				tint = (t * 250).tointeger();

			switch ( type )
			{
			case "fade":
				if ( tint > 0 )
				{
					local cfg = {
						when = Transition.EndNavigation,
						property="alpha",
						start=0,
						end =255,
						time = tint
					};

					animation.add( PropertyAnimation( obj, cfg ) );
					call_into_transition=true;
				}
				break;
			case "ease":
				if ( tint > 0 )
				{
					local vert = ((start=="top") || (start=="bottom"));
					local flip = ((start=="bottom") || (start=="right"));

					local cfg = {
						when = Transition.EndNavigation,
						property= vert ? "y" : "x",
						start = flip ?
							( vert ? fe.layout.height : fe.layout.width )
							: -300,
						end = vert ? y - h/2 : x - w/2,
						time = tint,
						easing = Easing.In
					};

					animation.add( PropertyAnimation( obj, cfg ) );
					call_into_transition=true;
				}
				break;
			}
		}
	}
	if ( call_into_transition )
		return animation.transition_callback( ttype, var, ttime );
	else
		return false;
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

fe.add_transition_callback( "hs_transition" );
function hs_transition( ttype, var, ttime )
{
	switch ( ttype )
	{
	case Transition.ToNewList:
	case Transition.EndNavigation:
		navigate_in_progress = false;
		return load( ttype, var, ttime, get_match_map(), get_hs_system_dir() );

	case Transition.ToNewSelection:
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
			get_hs_system_dir(), get_match_map() );

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

			local unmapped_index=0;
			for ( local i=0; i < dm_map.len(); i++ )
			{
				if ( dm_map[i] > fe.list.display_index )
				{
					unmapped_index = i;
					break;
				}
			}

			return load( Transition.EndNavigation,
				0,
				ttime,
				get_system_match_map( fe.list.display_index ),
				work_d + "Main Menu/" );
		}
		else if ( doing_display_menu )
		{
			if ( ttype == Transition.HideOverlay )
			{
				doing_display_menu = false;
				fe.overlay.clear_custom_controls();

				wheel.visible = true;

				// Reload to the newly selected display
				return load( Transition.EndNavigation,
					0,
					ttime,
					get_match_map(),
					get_hs_system_dir() );
			}
			else if ( ttype == Transition.NewSelOverlay )
			{
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

				local mapped_var = ( var < dm_map.len() ) ? dm_map[var] : 0;
				local mm = get_system_match_map( mapped_var );

				//
				// We switch the transition type here because we want to
				// trick the animate module into handling this like a typical
				// game navigation...
				//
				local r1 = load_override_transition(
					work_d + "Main Menu/", mm );

				local r2 = load( Transition.EndNavigation,
					0, ttime, mm, work_d + "Main Menu/" );

				return (r1 || r2);
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
local wheel_a = [  80,  80,  80,  80,  80,  80, 255,  80,  80,  80,  80,  80, ];
local wheel_r = [  30,  25,  20,  15,  10,   5,   0, -10, -15, -20, -25, -30, ];

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

// make sure the override transition object is drawn over the wheel
::hs_ent["override_transition"].obj.zorder=9999;
