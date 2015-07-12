/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - MALA Layout Loader/MaLadaptor
//
// Copyright (c) 2015 Andrew Mickelson
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
// Known Issues:
//
// - Animation1-4 not implemented
// - No image lists
// - missing some sounds
// - Missing some text substitutions ~etc
//
/////////////////////////////////////////////////////////
class UserConfig </ help="MALA Layout: " + ::file_to_load />
{
	</ label="Orientation",
		help="Select layout orientation",
		options="Horizontal,Vertical", order=1 />
	orient="Horizontal";

	</ label="Animate Game List",
		help="Set if the game list should be animated",
		options="Yes,No", order=2 />
	animate_list="Yes";

	</ label="Lazy Art",
		help="Set whether artworks should only update when navigation is completed",
		options="Yes,No", order=3 />
	lazy_art="No";

	</ label="Fade Transitions",
		help="Enable fade transitions",
		options="Yes,No", order=4 />
	fade="Yes";

}

fe.load_module( "conveyor" );

function mll_str( key, tag )
{
	if ( !mll[key].rawin( tag ) )
		return "";
	else
		return mll[key][tag];
}

function mll_int( key, tag, default_val=0 )
{
	if ( !mll[key].rawin( tag ) )
		return default_val;
	else
		return mll[key][tag].tointeger();
}

function hex2int( key, tag, pos )
{
	if ( !mll[key].rawin( tag ) )
		return 255;

	local s=mll[key][tag];
	local val=0;

	if ( s[pos+1] < 58 ) // 0-9
		val = s[pos+1] - 48;
	else // A-F
		val = s[pos+1] - 55;

	if ( s[pos] < 58 )
		val += (s[pos] - 48) << 4;
	else
		val += (s[pos] - 55) << 4;

	return val;
}
function mll_red( key, tag ) { return hex2int( key, tag, 0 ); }
function mll_green( key, tag ) { return hex2int( key, tag, 2 ); }
function mll_blue( key, tag ) { return hex2int( key, tag, 4 ); }

function mll_file( key, tag, prefix="" )
{
	if ( !mll[key].rawin( tag ) )
		return "";

	local t=split(mll[key][tag],"\\/");
	return prefix + t[t.len()-1];
}

function mll_text( key, tag )
{
	if ( !mll[key].rawin( tag ) )
		return "";

	local d = date();

	local rep_map = {
		"e_name": "[Emulator]",
		"g_name": "[Title]",
		"g_cloneof": "[CloneOf]",
		"g_genre": "[Category]",
		"g_year": "[Year]",
		"g_manufacturer": "[Manufacturer]",
		"g_control": "[Control]",
		"g_orientation": "[Rotation]",
		"g_players": "[Players]",
		"g_screen": "[DisplayType]",
		"g_played": "[PlayedCount]",
		"s_status": "[Status]",
		"l_name": "[FilterName]",
		"l_position": "[ListEntry]",
		"l_count": "[ListSize]",
		"m_time": "12:00",
		"m_date": ( d["year"] + "/" + d["month"] * d["day"] )
	};

	local str = mll[key][tag];

	local ex = regexp( "~[_a-z]+" );
	local pos=0;

	local token = ex.search( str, pos );
	while ( token )
	{
		local tmp = str.slice( token.begin+1, token.end );
		local new_end = str.slice( token.end );

		str = str.slice( 0, token.begin )
			+ ( rep_map.rawin(tmp) ? rep_map[tmp] : "" );

		pos = str.len();
		str += new_end;

		token = ex.search( str, pos );
	}

	return str;
}

function mll_align( key, tag )
{
	if ( !mll[key].rawin( tag ) )
		return Align.Centre;

	switch ( mll_int( key, tag ) )
	{
	case 1: return Align.Left;
	case 2: return Align.Right;
	}

	return Align.Centre; // 0
}

function mll_style( bold, italic )
{
	local style=0;
	if ( bold == "1" )
		style = Style.Bold;
	if ( italic == "1" )
		style = style | Style.Italic;

	return style;
}


function add_bgimage()
{
	local img = fe.add_image(
		mll_file( lkey, "BGImage", ::mala_subdir ),
		(vert ? fe.layout.width : 0),
		0,
		(vert? fe.layout.height : fe.layout.width),
		(vert? fe.layout.width : fe.layout.height) );

	if ( img.file_name.len() < 1 )
		img.file_name = mll_file( lkey, "BGImage" );

	if ( vert ) img.rotation = 90;	
}

::list_points <- [];

class ListEntry extends ConveyorSlot
{
	_t=null;
	constructor()
	{
		_t = fe.add_text( "[Title]", 0, 0, 0, 0 );
		base.constructor();
	}

	_pro_attrs = ["x","y","width","height","red","green","blue","charsize"];
	function on_progress( progress, var )
	{
		local num = ::list_points.len()-1;
		local p = progress * num;
		local s = p.tointeger();
		p -= s;

		// don't show list entries off the edges of the mala list
		if ( (progress<0) || (progress>( 1.0*(num-1)/num)) )
			_t.visible=false;
		else
			_t.visible=true;

		if ( s < 0 )
			s=0;
		else if ( s >= num )
			s = num-1;
	
		foreach (attr in _pro_attrs )
		{
			local tmp = ::list_points[s+1][attr]
				- ::list_points[s][attr];
			_t[attr] = ::list_points[s][attr] + p * tmp;
		}
	}

	function swap( other ) {}
	function set_index_offset( io ) { _t.index_offset=io; }
	function reset_index_offset() { set_index_offset( m_base_io ); }
};

function step_color( c, step )
{
	c += step;
	if ( c < 0 ) c=0;
	if ( c > 255 ) c=255;

	return c;
}

function add_list()
{
	// we want this to return true if ListVisible doesn't exist
	if ( mll_int( lkey, "ListVisible", 1 ) != 1 )
		return;

	local slots = [];

	const LIST="List";
	const TFC="TopFontColor";
	const CFC="CenterFontColor";
	const BFC="BottomFontColor";
	const ROUND_DIVIDE=2.4;

	local x = mll_int( lkey, "ListLeft");
	local y = mll_int( lkey, "ListTop");
	local w = mll_int( lkey, "ListWidth");
	local h = mll_int( lkey, "ListHeight");
	local align = mll_align( lkey, "ListAlignment" );

	if ( vert )
	{
		// swap w&h, x&y
		local temp=w;
		w=h;
		h=temp;

		temp=x;
		x = fe.layout.width - y - w;
		y = temp;
	}

	local dist = mll_int( lkey, "ListDistance");
	local s_grad = mll_int( lkey, "ListSizeGradient");

	local count_backup = mll_int( LIST, "TopBottomCount" );
	local topcount = mll_int( lkey, "ListTopCount", count_backup);
	local bottomcount = mll_int( lkey, "ListBottomCount", count_backup);

	local red = mll_red( LIST, TFC );
	local green = mll_green( LIST, TFC );
	local blue = mll_blue( LIST, TFC );

	local red_step = 0;
	local green_step = 0;
	local blue_step = 0;

	enum ColorStyle
	{
		Flat		=0,
		Highlight	=1,
		Gradient 	=2
	}

	switch ( mll_int( LIST, "ColorStyle" ) )
	{
	case ColorStyle.Flat:
		red = mll_red( LIST, CFC );
		green = mll_green( LIST, CFC );
		blue = mll_blue( LIST, CFC );
		break;

	case ColorStyle.Highlight:
		break;

	case ColorStyle.Gradient:
		local grad_depth = mll_int( LIST, "GradientDepth" );
		red_step = ( mll_red( LIST, CFC ) - red ) / ( topcount + grad_depth );
		green_step = ( mll_green( LIST, CFC ) - green ) / ( topcount + grad_depth );
		blue_step = ( mll_blue( LIST, CFC ) - blue ) / ( topcount + grad_depth );
		break;
	}

	local tb_fontsize = mll_int( LIST, "TopBottomFontSize" );

	enum Trans
	{
		NoTransform	=0,
		ListDiagonal	=1,
		ListArrow	=2,
		ListRound	=3
	}
	local trans = mll_int( lkey, "ListTransformation" );
	local trans_adj=0;

	switch ( trans )
	{
	case Trans.ListDiagonal:
		local diag = mll_int( lkey, "ListDiagonal");
		trans_adj = diag/25.0 * w / ( topcount + bottomcount );

		local old_w = w;
		w -= fabs( trans_adj ) * ( topcount + bottomcount );

		if ( diag < 0 )
			x += old_w - w;

		break;

	case Trans.ListArrow:
		local arrow = mll_int( lkey, "ListArrow");
		trans_adj = arrow/25.0 * w / ( topcount + bottomcount );

		local old_w = w;
		w -= fabs( trans_adj ) * ( topcount + bottomcount ) / 2;

		if ( arrow < 0 )
			x += old_w - w;
		break;

	case Trans.ListRound:
		local round = mll_int( lkey, "ListRound" );
		trans_adj = round/25.0 * w / ( topcount + bottomcount );

		local old_w = w;
		w -= fabs( trans_adj ) * ( topcount + bottomcount ) / 6;

		if ( round < 0 )
			x += old_w - w;
		break;

	case Trans.NoTransform:
	default:
		// None
	};

	local ent;
	local tmp;
	for ( local i=0; i< topcount; i++ )
	{
		ent =
		{
			x=x,
			y=y,
			width=w,
			height=h,
			red=red,
			green=green,
			blue=blue,
			charsize=tb_fontsize
		};
		::list_points.append( ent );

		tmp = ListEntry();
		tmp._t.font = mll_str( LIST, "TopBottomFontName" );
		tmp._t.style = mll_style( mll_str( LIST, "TopBottomFontBold" ),
				mll_str( LIST, "TopBottomFontItalic" ) );
		tmp._t.align = align;

		slots.append( tmp );

		y += h + dist;

		if ( trans == Trans.ListRound )
			trans_adj -= trans_adj / ROUND_DIVIDE;
		x += trans_adj;

		tb_fontsize += s_grad;
		red = step_color( red, red_step );
		green = step_color( green, green_step );
		blue = step_color( blue, blue_step );
	}

	local ch = mll_int( lkey, "ListCenterHeight", h );

	local ent =
	{
		x=x,
		y=y,
		width=w,
		height=ch,
		charsize = mll_int( LIST, "CenterFontSize" ),
		red = mll_red( LIST, CFC ),
		green = mll_green( LIST, CFC ),
		blue = mll_blue( LIST, CFC )
	}

	::list_points.append( ent );

	tmp = ListEntry();
	tmp._t.font = mll_str( LIST, "CenterFontName" );
	tmp._t.style = mll_style( mll_str( LIST, "CenterFontBold" ),
			mll_str( LIST, "CenterFontItalic" ) );
	tmp._t.align = align;

	if ( mll_int( LIST, "CenterFontUseBGColor" ) == 1 )
	{
		const CFBG = "CenterFontBGColor";
		tmp._t.bg_red = mll_red( LIST, CFBG );
		tmp._t.bg_green = mll_green( LIST, CFBG );
		tmp._t.bg_blue = mll_blue( LIST, CFBG );
		tmp._t.bg_alpha = 255;
	}

	slots.append( tmp );

	y += ch + dist;

	red = step_color( red, -red_step );
	green = step_color( green, -green_step );
	blue = step_color( blue, -blue_step );

	for ( local i=0; i< bottomcount; i++ )
	{
		if ( trans == Trans.ListDiagonal )
			x += trans_adj;
		else
		{
			if ( trans == Trans.ListRound )
				trans_adj += 1.25 * trans_adj / ROUND_DIVIDE;

			x -= trans_adj;
		}

		local ent =
		{
			x=x,
			y=y,
			width=w,
			height=h,
			red=red,
			green=green,
			blue=blue,
			charsize=tb_fontsize
		};
		::list_points.append( ent );

		tmp = ListEntry();
		tmp._t.font = mll_str( LIST, "TopBottomFontName" );
		tmp._t.style = mll_style( mll_str( LIST, "TopBottomFontBold" ),
				mll_str( LIST, "TopBottomFontItalic" ) );
		tmp._t.align = align;

		slots.append( tmp );

		y += h + dist;

		tb_fontsize -= s_grad;
		red = step_color( red, -red_step );
		green = step_color( green, -green_step );
		blue = step_color( blue, -blue_step );
	}

	local ent =
	{
		x=x,
		y=y,
		width=w,
		height=h,
		red=red,
		green=green,
		blue=blue,
		charsize=tb_fontsize
	};

	::list_points.append( ent );

	mala_conveyor <- Conveyor();
	mala_conveyor.set_slots( slots, topcount );
	mala_conveyor.transition_swap_point = 1.0;
	mala_conveyor.transition_ms = 80;

	if ( am_options["animate_list"] != "Yes" )
		mala_conveyor.enabled = false;
}

function add_art( mll_label, am_label="", extra="Image" )
{
	if ( mll_int( lkey, mll_label + "Visible" ) != 1 )
		return;

	local x = mll_int( lkey, mll_label+extra+"Left" );
	local y = mll_int( lkey, mll_label+extra+"Top" );
	local w = mll_int( lkey, mll_label+extra+"Width" );
	local h = mll_int( lkey, mll_label+extra+"Height" );

	if ( vert )
	{
		// swap w&h, x&y
		local temp=w;
		w=h;
		h=temp;

		temp=x;
		x = fe.layout.width - y - w;
		y = temp;
	}

	// If there is a default image and attract-mode doesn't have
	// a label for this artwork then just go and load the default image
	//
	local img=null;
	if ( am_label.len() == 0 )
	{
		if ( mll_str( lkey, mll_label+"Default" ).len() > 0 )
		{
			img = fe.add_image(
				mll_file( key,
					mll_label+"Default",
					::mala_subdir ),
				x, y, w, h );

			if ( img.file_name.len() < 1 )
				img.file_name = mll_file( key,
					mll_label+"Default" );
		}
	}
	else
	{
		img = fe.add_artwork( am_label, x, y, w, h );
		if ( am_options["lazy_art"] == "Yes" )
			img.trigger = Transition.EndNavigation;
	}
}

function add_custom( key )
{
	local ex = lkey.slice( 0, 1 );

	if ( mll_int( key, "Visible" + ex ) != 1 )
		return;

	local x = mll_int( key, "Left" + ex );
	local y = mll_int( key, "Top" + ex );
	local w = mll_int( key, "Width"  + ex );
	local h = mll_int( key, "Height" + ex );

	if ( vert )
	{
		// swap w&h, x&y
		local temp=w;
		w=h;
		h=temp;

		temp=x;
		x = fe.layout.width - y - w;
		y = temp;
	}

	local do_transparent = mll_int( key, "Transparent" );

	local img = fe.add_image(
			mll_file( key, "Image" + ex, ::mala_subdir ),
			x, y, w, h );

	if ( img.file_name.len() < 1 )
		img.file_name = mll_file( key, "Image" + ex );

	if ( do_transparent && (img.file_name.len() > 0) )
		img.fix_masked_image();
}

function add_video2()
{
	if ( mll_int( lkey, "Video2Visible" ) != 1 )
		return;

	local x = mll_int( lkey, "Video2Left" );
	local y = mll_int( lkey, "Video2Top" );
	local w = mll_int( lkey, "Video2Width" );
	local h = mll_int( lkey, "Video2Height" );

	local vid = fe.add_image(
		mll_file( lkey, "Video2File", ::mala_subdir ),
		x, y, w, h );
	
	if ( vid.file_name.len() < 1 )
		vid.file_name = mll_file( lkey, "Video2File" );
}

function add_text( num )
{
	local ex = lkey.slice( 0, 1 );

	local k = "Text" + num;
	if ( mll_int( k, "Visible"+ex ) == 1 )
	{
		local x = mll_int( k, "Left"+ex );
		local y = mll_int( k, "Top"+ex );
		local w = mll_int( k, "Width"+ex );
		local h = mll_int( k, "Height"+ex );

		if ( vert )
		{
			// swap w&h, x&y
			local temp=w;
			w=h;
			h=temp;

			temp=x;
			x = fe.layout.width - y - w;
			y = temp;
		}

		local text = fe.add_text(
			mll_text( k, "Pattern" ),
			x, y, w, h );

		text.font = mll_str( k, "FontName" );
		text.charsize = mll_int( k, "FontSize" );
		text.red = mll_red( k, "FontColor" );
		text.green = mll_green( k, "FontColor" );
		text.blue = mll_blue( k, "FontColor" );
		text.align = mll_align( k, "FontAlignment"+ex );
	}
}

function add_logo()
{
	if ( mll_int( lkey, "LogoVisible" ) != 1 )
		return;

	local x = mll_int( lkey, "LogoLeft" );
	local y = mll_int( lkey, "LogoTop" );
	local w = mll_int( lkey, "LogoWidth" );
	local h = mll_int( lkey, "LogoHeight" );

	if ( vert )
	{
		// swap w&h, x&y
		local temp=w;
		w=h;
		h=temp;

		temp=x;
		x = fe.layout.width - y - w;
		y = temp;
	}

	local text = fe.add_text( "[DisplayName]", x, y, w, h );

	const LOGO="Logo";
	text.font = mll_str( LOGO, "FontName" );
	text.charsize = mll_int( LOGO, "FontSize" );
	text.red = mll_red( LOGO, "FontColor" );
	text.green = mll_green( LOGO, "FontColor" );
	text.blue = mll_blue( LOGO, "FontColor" );
	text.align = mll_align( lkey, "LogoFontAlignment" );
}

function add_sound( label )
{
	if (( mll["Sounds"].rawin( label ) )
		&& ( mll["Sounds"][label].len() > 0 ))
	{
		local snd = fe.add_sound(
			fe.script_dir
			+ mll_file("Sounds",label,::mala_subdir) );

		if ( snd.file_name.len() < 1 )
			snd.file_name = fe.script_dir
				+ mll_file("Sounds",label);

		return snd;

	}

	return null;
}

//
// Get the mll file settings
//
mll <- {};

local entity=null;
local f = ReadTextFile( fe.script_dir, ::file_to_load );

while ( !f.eos() )
{
	local line = f.read_line();
	if (( line.len() > 0 ) && ( line[0] == '[' ))
	{
		entity = line.slice( 1, line.len()-1 );
		mll[ entity ] <- {};
	}
	else
	{
		local temp = split( line, "=" );
		local v = ( temp.len() > 1 ) ? temp[ 1] : "";

		if ( entity ) mll[entity][temp[0]] <- v;
		else mll[temp[0]] <- v;
	}
}

function add_config_attribute( mll_tag, porder )
{
	if ( mll_int( "Vertical", mll_tag + "Visible" )
		|| mll_int( "Horizontal", mll_tag + "Visible" ) )
	{
		local layout_tag = mll_tag.tolower();
		UserConfig[layout_tag] <- "";
		UserConfig.setattributes( layout_tag,
			{
				label=mll_tag + " Artwork",
				help="Name the artwork to put in the "
					+ mll_tag + " slot",
				order=porder
			} );
	}
}

//
// Load mll-dependent options into the UserConfig table
//
{
	local config_attrs = [ "CPanel", "Video", "Definable1",
		"Definable2", "Definable3", "Definable4", "Definable5" ];

	local o=10;
	foreach ( a in config_attrs )
		add_config_attribute( a, o++ );
}

//
// Execution will end here when the script is run to get config settings
//
am_options <- fe.get_config();

local temp = split( ::file_to_load, "\\/" );
print( " - Found MALA layout: " + temp[ temp.len()-1 ] + "\n" );

//
//
lkey <- am_options["orient"];
vert <- ( lkey == "Vertical" );

local tmp = mll_str( "Layout", "Name" );
if ( tmp.len() > 4 )
	::mala_subdir <- tmp.slice( 0, tmp.len()-4 ) + "/";
else
	::mala_subdir <- "";

//
// Initialize the layout
//
if ( vert )
{
	fe.layout.width = mll_int( "Layout", "FixedSizeHeight" );
	fe.layout.height = mll_int( "Layout", "FixedSizeWidth" );
	fe.layout.base_rotation = RotateScreen.Left;
}
else
{
	fe.layout.width = mll_int( "Layout", "FixedSizeWidth" );
	fe.layout.height = mll_int( "Layout", "FixedSizeHeight" );
}

// set the default font to the list center font
fe.layout.font = mll_str( "List", "CenterFontName" );

// Draw artworks according to ZOrder
for ( local i=0; i<29; i++ )
{
	switch( mll_int( "ZOrder", i.tostring(), i ) )
	{
	case  0: add_bgimage(); break;
	// 1-4 (Animation4-Animation1)
	case  5:
		add_art( "Video",
			am_options.rawin("video")?am_options["video"]:"",
			"" );
		break;
	case  6:
		local t="Definable4";
		local t2=t.tolower();
		add_art( t, am_options.rawin(t2)?am_options[t2]:"" );
		break;
	case  7:
		local t="Definable3";
		local t2=t.tolower();
		add_art( t, am_options.rawin(t2)?am_options[t2]:"" );
		break;
	case  8:
		local t="Definable2";
		local t2=t.tolower();
		add_art( t, am_options.rawin(t2)?am_options[t2]:"" );
		break;
	case  9:
		local t="Definable1";
		local t2=t.tolower();
		add_art( t, am_options.rawin(t2)?am_options[t2]:"" );
		break;
	case 10:
		add_art( "CPanel",
			am_options.rawin("cpanel")?am_options["cpanel"]:"" );
		break;

	case 11: add_art( "Marquee", "marquee" ); break;
	case 12: add_art( "Snap", "snap" ); break;
	case 13: add_custom( "Custom4" ); break;
	case 14: add_custom( "Custom3" ); break;
	case 15: add_custom( "Custom2" ); break;
	case 16: add_custom( "Custom1" ); break;
	case 17: add_text( 8 ); break;
	case 18: add_text( 7 ); break;
	case 19: add_text( 6 ); break;
	case 20: add_text( 5 ); break;
	case 21: add_text( 4 ); break;
	case 22: add_text( 3 ); break;
	case 23: add_text( 2 ); break;
	case 24: add_text( 1 ); break;
	case 25: add_logo(); break;
	case 26: add_list(); break;
	case 27: add_video2(); break;
	case 28:
		local t="Definable5";
		local t2=t.tolower();
		add_art( t, am_options.rawin(t2)?am_options[t2]:"" );
		break;
	}
}

//
// Handle layout sounds
//
local move_sound = add_sound( "Movement" );
local to_game_sound = add_sound( "StartGame" );
local from_game_sound = add_sound( "QuitGame" );
local list_switch_sound = add_sound( "ListSwitch" );
local screen_saver_sound = add_sound( "AttractMode" );
local exit_sound = add_sound( "Exit" );

if ( mll_str( "Sounds", "BGMusic" ).len() > 0 )
{
	fe.ambient_sound.file_name = fe.script_dir
		+ mll_file( "Sounds", "BGMusic", ::mala_subdir );
	if ( fe.ambient_sound.file_name.len() < 1 )
		fe.ambient_sound.file_name = fe.script_dir
			+ mll_file( "Sounds", "BGMusic" );

	fe.ambient_sound.playing=true;
}

fe.add_transition_callback( "mala_sound_callback" );
function mala_sound_callback( ttype, var, ttime )
{
	switch ( ttype )
	{
	case Transition.ToNewSelection:
		if ( move_sound ) move_sound.playing=true;
		break;

	case Transition.ToGame:
		if ( to_game_sound ) to_game_sound.playing=true;
		break;

	case Transition.FromGame:
		if ( from_game_sound ) from_game_sound.playing=true;
		break;

	case Transition.ToNewList:
		if ( list_switch_sound ) list_switch_sound.playing=true;
		break;

	case Transition.EndLayout:
		if (( var == FromTo.ScreenSaver ) && ( screen_saver_sound ))
			screen_saver_sound.playing=true;
		else if (( var == FromTo.Frontend ) && ( exit_sound ))
			exit_sound.playing=true;
		break;
	}

	return false;
}

if ( am_options["fade"] == "Yes" )
	fe.add_transition_callback( "mala_transition_callback" );

function mala_transition_callback( ttype, var, ttime )
{
	switch ( ttype )
	{
	case Transition.ToGame:
	case Transition.EndLayout:
		if ( ttime < 255 )
		{
			foreach ( o in fe.obj )
				o.alpha = 255 - ttime;

			return true;
		}

		if ( !fe.obj[0].alpha == 0 )
		{
			foreach ( o in fe.obj )
				o.alpha=0;
			return true;
		}
		break;

	case Transition.FromGame:
	case Transition.StartLayout:
		if ( ttime < 255 )
		{
			foreach ( o in fe.obj )
				o.alpha = ttime;

			return true;
		}

		foreach ( o in fe.obj )
			o.alpha = 255;

		break;
	}

	return false;
}

mll=null;
