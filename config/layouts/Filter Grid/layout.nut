///////////////////////////////////////////////////
//
// Attract-Mode Frontend - filter-grid layout
//
///////////////////////////////////////////////////
class UserConfig {
	</ label="Grid Artwork", help="The artwork to display in the grid", options="snap,marquee,flyer,wheel", order=1 />
	art="snap";

	</ label="Entries", help="The number of entries to show for each filter", options="1,2,3,4,5,6,7,8", order=2 />
	entries="3";

	</ label="Filters", help="The number of filters to show", options="1,2,3,4,5,6,7,8", order=3 />
	filters="3";

	</ label="Flow Direction", help="Select the flow direction for entries", options="Horizontal,Vertical", order=4 />
	flow="Vertical";

	</ label="Preserve Aspect Ratio", help="Preserve artwork aspect ratio", options="Yes,No", order=5 />
	aspect_ratio="Yes";

	</ label="Transition Time", help="The amount of time (in milliseconds) that it takes to scroll to another grid entry", order=6 />
	ttime="100";
}

fe.layout.width = 800;
fe.layout.height = 800;

fe.load_module( "conveyor" );

local my_config = fe.get_config();

const TOP_SPACE = 30;
const BOTTOM_SPACE = 30;
const PAD = 12;
enum TState { None, Next, Prev };

local transition_state = TState.None;
local filters = [];
local f_labels = [];
local ftr_count = my_config["filters"].tointeger();
local ftr_index = ftr_count / 2;
local old_ftr_index = ftr_index;
local sel_count = my_config["entries"].tointeger();
local sel_index = 0;
local x_dim;
local y_dim;

local transition_ms = 100;
try { transition_ms = my_config["ttime"].tointeger(); } catch( e ) {}

if ( my_config["flow"] == "Vertical" )
{
	x_dim = fe.layout.width / ftr_count;
	y_dim = ( fe.layout.height - TOP_SPACE - BOTTOM_SPACE ) / sel_count;
}
else
{
	x_dim = fe.layout.width / sel_count;
	y_dim = ( fe.layout.height - TOP_SPACE - BOTTOM_SPACE ) / ftr_count;
}

for ( local i=0; i<ftr_count; i++ )
{
	local new_strip;
	local new_label;

	if ( my_config["flow"] == "Vertical" )
	{
		new_strip = SimpleArtStrip(
			my_config["art"],
			sel_count,
			i * x_dim,
			TOP_SPACE,
			x_dim,
			fe.layout.height - TOP_SPACE - BOTTOM_SPACE,
			PAD );

		new_strip.transition_ms = transition_ms;

		new_label = fe.add_text( "[FilterName]", i * x_dim, 0, x_dim, TOP_SPACE );
	}
	else
	{
		new_strip = SimpleArtStrip(
			my_config["art"],
			sel_count,
			0,
			TOP_SPACE + i * y_dim,
			fe.layout.width,
			y_dim,
			PAD );

		new_strip.transition_ms = transition_ms;

		new_label = fe.add_text( "[FilterName]", 0, TOP_SPACE + i * y_dim, fe.layout.width, y_dim );
		new_label.alpha = 125;
		new_label.set_rgb( 0, 0, 0 );
	}
	new_strip.set_selection( 0 );
	new_strip.video_flags = Vid.NoAudio;
	new_strip.preserve_aspect_ratio = (my_config["aspect_ratio"]=="Yes");

	local temp = i - ftr_index;
	if ( temp != 0 )
	{
		new_strip.filter_offset = temp;
		new_label.filter_offset = temp;
		new_strip.enabled = false;
	}

	filters.push( new_strip );
	f_labels.push( new_label );
}

local frame = fe.add_image( "frame.png", 0, TOP_SPACE, x_dim, y_dim );
local frame_texts = fe.add_text( "[ListEntry]/[ListSize]", 1, TOP_SPACE + 1, x_dim, y_dim / 8 );
local frame_text = fe.add_text( "[ListEntry]/[ListSize]", 0, TOP_SPACE, x_dim, y_dim / 8 );
frame_texts.set_rgb( 0, 0, 0 );

local name_label = fe.add_text( "[Title]", 0, fe.layout.height - BOTTOM_SPACE - 2, fe.layout.width, BOTTOM_SPACE - 2 );

function update_filters()
{
	for ( local i=0; i<filters.len(); i++ )
	{
		foreach ( o in filters[i].m_objs )
			o.m_obj.rawset_filter_offset( i - ftr_index );

		f_labels[i].filter_offset = i - ftr_index;
	}

	foreach ( f in filters )
		f.enabled = false;
	filters[ftr_index].enabled = true;

	transition_state = TState.None;
	fe.list.filter_index += ftr_index - old_ftr_index;
	old_ftr_index = ftr_index;
}

function update_audio()
{
	foreach ( f in filters )
	{
		foreach ( o in f.m_objs )
			o.m_obj.video_flags = Vid.NoAudio;
	}

	filters[ftr_index].m_objs[sel_index].m_obj.video_flags = 0;
}

function update_frame()
{
	if ( my_config["flow"] == "Vertical" )
	{
		frame.x = x_dim * ftr_index;
		frame.y = TOP_SPACE + y_dim * sel_index;
	}
	else
	{
		frame.x = x_dim * sel_index;
		frame.y = TOP_SPACE + y_dim * ftr_index;
	}

	frame_text.x = frame.x + 5;
	frame_text.y = frame.y + frame.height - frame_text.height - 5;
	frame_texts.x = frame_text.x + 1;
	frame_texts.y = frame_text.y + 1;

	frame_texts.filter_offset
			= frame_text.filter_offset
			= name_label.filter_offset
			= filters[ftr_index].m_objs[0].m_obj.filter_offset;

	frame_texts.index_offset
			= frame_text.index_offset
			= name_label.index_offset
			= sel_index - filters[ftr_index].selection_index;

	update_audio();
}

update_frame();

fe.add_signal_handler( "on_signal" );
function on_signal( sig )
{
	switch ( sig )
	{
	case "up":
		if ( sel_index > 0 )
		{
			sel_index--;
			update_frame();
		}
		else
		{
			fe.signal( "prev_game" );
		}
		return true;
	case "down":
		if ( sel_index < sel_count - 1 )
		{
			sel_index++;
			update_frame();
		}
		else
		{
			fe.signal( "next_game" );
		}
		return true;
	case "left":
		if ( ftr_index > 0 )
		{
			ftr_index--;
			update_filters();
			update_frame();
			return true;
		}
		else
		{
			// swap images to reduce reloading
			for ( local i=ftr_count-1; i>0; i-- )
			{
				for ( local j=0; j<sel_count; j++ )
				{
					filters[i].m_objs[j].m_obj.swap(
						filters[i-1].m_objs[j].m_obj );
				}
			}
			transition_state = TState.Prev;

			fe.signal( "prev_filter" );
			return true;
		}
		break;
	case "right":
		if ( ftr_index < ftr_count - 1 )
		{
			ftr_index++;
			update_filters();
			update_frame();
			return true;
		}
		else
		{
			// swap images to reduce reloading
			for ( local i=1; i<ftr_count; i++ )
			{
				for ( local j=0; j<sel_count; j++ )
				{
					filters[i].m_objs[j].m_obj.swap(
						filters[i-1].m_objs[j].m_obj );
				}
			}
			transition_state = TState.Next;

			fe.signal( "next_filter" );
			return true;
		}
		break;
	case "next_game":
	case "prev_game":
	case "next_filter":
	case "prev_filter":
	case "exit":
	case "exit_no_menu":
		break;
	case "select":
	default:
		// Correct the list index if it doesn't align with
		// the game our frame is on
		filters[ftr_index].enabled=false;
		fe.list.index += sel_index - filters[ftr_index].selection_index;

		foreach ( f in filters )
			f.set_selection( sel_index );

		update_frame();
		filters[ftr_index].enabled=true;
		break;
	}

	return false;
}

fe.add_transition_callback( "on_transition" );
function on_transition( ttype, var, ttime )
{
	if ( ttype == Transition.FromOldSelection )
		update_audio();

	if ( ttype != Transition.ToNewList )
		return false;

	update_audio();

	if (( transition_state == TState.Next ) && ( ttime < transition_ms ))
	{
		if ( my_config["flow"] == "Vertical" )
		{
			for ( local i=0; i<ftr_count; i++ )
			{
				local val = PAD/2 + x_dim * ( i + 1 )
						- ( ttime / transition_ms.tofloat() ) * x_dim;

				filters[i].x = f_labels[i].x = val;
			}
		}
		else
		{
			for ( local i=0; i<ftr_count; i++ )
			{
				local val = TOP_SPACE + PAD/2 + y_dim * ( i + 1 )
						- ( ttime / transition_ms.tofloat() ) * y_dim;
				filters[i].y = f_labels[i].y = val;
			}
		}

		return true;
	}
	else if (( transition_state == TState.Prev ) && ( ttime < transition_ms ))
	{
		if ( my_config["flow"] == "Vertical" )
		{
			for ( local i=0; i<ftr_count; i++ )
			{
				local val = PAD/2 + x_dim * ( i - 1 )
						+ ( ttime / transition_ms.tofloat() ) * x_dim;
				filters[i].x = f_labels[i].x = val;
			}
		}
		else
		{
			for ( local i=0; i<ftr_count; i++ )
			{
				local val = TOP_SPACE + PAD/2 + y_dim * ( i - 1 )
						+ ( ttime / transition_ms.tofloat() ) * y_dim;
				filters[i].y = f_labels[i].y = val;
			}
		}

		return true;
	}

	if ( my_config["flow"] == "Vertical" )
	{
		for ( local i=0; i<ftr_count; i++ )
			filters[i].x = f_labels[i].x = PAD/2 + x_dim * i;
	}
	else
	{
		for ( local i=0; i<ftr_count; i++ )
			filters[i].y = f_labels[i].y = TOP_SPACE + PAD/2 + y_dim * i;
	}

	return false;
}
