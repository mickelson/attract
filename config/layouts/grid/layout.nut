///////////////////////////////////////////////////
//
// Attract-Mode Frontend - Grid layout
//
///////////////////////////////////////////////////
class UserConfig </ help="Navigation controls: Up/Down (to move up and down) and Page Up/Page Down (to move left and right)" />{
	</ label="Grid Artwork", help="The artwork to display in the grid", options="snap,marquee,flyer,wheel", order=1 />
	art="snap";

	</ label="Rows", help="The number of grid rows", options="1,2,3,4,5,6,7,8", order=2 />
	rows="4";

	</ label="Columns", help="The number of grid columns", options="1,2,3,4,5,6,7,8", order=3 />
	columns="4";

	</ label="Preserve Aspect Ratio", help="Preserve artwork aspect ratio", options="Yes,No", order=4 />
	aspect_ratio="Yes";

	</ label="Enable Video", help="Enable video artwork in the grid", options="Yes,No", order=5 />
	video="Yes";

	</ label="Transition Time", help="The amount of time (in milliseconds) that it takes to scroll to another grid entry", order=6 />
	ttime="220";
}

fe.load_module( "conveyor" );

fe.layout.width = 800;
fe.layout.height = 800;

local my_config = fe.get_config();
local rows = my_config[ "rows" ].tointeger();
local cols = my_config[ "columns" ].tointeger();
local height = ( fe.layout.height * 11 / 12 ) / rows.tofloat();
local width = fe.layout.width / cols.tofloat();

const PAD=4;

class Grid extends Conveyor
{
	frame=null;
	name_t=null;
	num_t=null;
	sel_x=0;
	sel_y=0;

	constructor()
	{
		base.constructor();

		sel_x = cols / 2;
		sel_y = rows / 2;
		stride = fe.layout.page_size = rows;
		fe.add_signal_handler( this, "on_signal" );
	
		try
		{
			transition_ms = my_config["ttime"].tointeger();
		}
		catch ( e )
		{
			transition_ms = 220;
		}
	}

	function update_frame()
	{
		frame.x = width * sel_x;
		frame.y = fe.layout.height / 24 + height * sel_y;
		
		name_t.index_offset = num_t.index_offset
				= sel_x * rows + sel_y - selection_index;	
	}

	function on_signal( sig )
	{
		switch ( sig )	
		{
		case "up":
			if ( sel_y > 0 )
			{
				sel_y--;
				update_frame();
				return true;
			}
			transition_swap_point=0.5;
			break;
		case "down":
			if ( sel_y < rows - 1 )
			{
				sel_y++;
				update_frame();
				return true;
			}
			transition_swap_point=0.5;
			break;
		case "page_up":
			if ( sel_x > 1 )
			{
				sel_x--;
				update_frame();
				return true;
			}
			transition_swap_point=0.0;
			break;
		case "page_down":
			if ( sel_x < cols - 2 )
			{
				sel_x++;
				update_frame();
				return true;
			}
			transition_swap_point=0.0;
			break;


		case "select":
		default:
			// Correct the list index if it doesn't align with
			// the game our frame is on
			//
			local frame_index = sel_x * rows + sel_y;
			fe.list.index += frame_index - selection_index;

			set_selection( frame_index );
			update_frame();
			break;

		}

		return false;
	}

	function on_transition( ttype, var, ttime )
	{
		switch ( ttype )
		{
		case Transition.StartLayout:
		case Transition.FromGame:
			if ( ttime < transition_ms )
			{
				for ( local i=0; i< m_objs.len(); i++ )
				{
					local r = i % rows;
					local c = i / rows;
					local num = rows + cols - 2;

					local temp = 510 * ( num - r - c ) / num * ttime / transition_ms;
					m_objs[i].m_obj.alpha = ( temp > 255 ) ? 255 : temp;
				}

				frame.alpha = 255 * ttime / transition_ms;
				return true;
			}

			local old_alpha = m_objs[ m_objs.len()-1 ].m_obj.alpha;

			foreach ( o in m_objs )
				o.m_obj.alpha = 255;

			frame.alpha = 255;

			if ( old_alpha != 255 )
				return true;

			break;

		case Transition.ToGame:
		case Transition.EndLayout:
			if ( ttime < transition_ms )
			{
				for ( local i=0; i< m_objs.len(); i++ )
				{
					local r = i % rows;
					local c = i / rows;
					local num = rows + cols - 2;

					local temp = 255 - 510 * ( num - r - c ) / num * ttime / transition_ms;
					m_objs[i].m_obj.alpha = ( temp < 0 ) ? 0 : temp;
				}
				frame.alpha = 255 - 255 * ttime / transition_ms;
				return true;
			}

			local old_alpha = m_objs[ m_objs.len()-1 ].m_obj.alpha;

			foreach ( o in m_objs )
				o.m_obj.alpha = 0;

			frame.alpha = 0;

			if ( old_alpha != 0 )
				return true;

			break;
		}

		return base.on_transition( ttype, var, ttime );
	}
}

::gridc <- Grid();

class MySlot extends ConveyorSlot
{
	m_num = 0;
	m_shifted = false;

	constructor( num )
	{
		m_num = num;

		local my_art = fe.add_artwork( my_config["art"], 0, 0,
				width - 2*PAD, height - 2*PAD );

		my_art.preserve_aspect_ratio = (my_config["aspect_ratio"]=="Yes");
		my_art.video_flags = Vid.NoAudio;
		if ( my_config["video"] == "No" )
			my_art.video_flags = Vid.ImagesOnly | Vid.NoAudio;

		my_art.alpha = 0;
		base.constructor( my_art );
	}

	function on_progress( progress, var )
	{
		if ( var == 0 )
			m_shifted = false;

		local r = m_num % rows;
		local c = m_num / rows;

		if ( abs( var ) < rows )
		{
			m_obj.x = c * width + PAD;
			m_obj.y = fe.layout.height / 24
				+ ( fe.layout.height * 11 / 12 ) * ( progress * cols - c ) + PAD;
		}
		else
		{
			local prog = ::gridc.transition_progress;
			if ( prog > ::gridc.transition_swap_point )
			{
				if ( var > 0 ) c++;
				else c--;
			}

			if ( var > 0 ) prog *= -1;

			m_obj.x = ( c + prog ) * width + PAD;
			m_obj.y = fe.layout.height / 24 + r * height + PAD;
		}
	}
}

local my_array = [];
for ( local i=0; i<rows*cols; i++ )
	my_array.push( MySlot( i ) );

gridc.set_slots( my_array );
gridc.frame=fe.add_image( "frame.png", width * 2, height * 2, width, height );

gridc.name_t =  fe.add_text( "[Title]", 0,
		fe.layout.height - fe.layout.height / 24 - PAD,
		fe.layout.width, fe.layout.height / 24 );

local title = fe.add_text( "[ListTitle]/[ListFilterName]",
			0, 0, fe.layout.width/2, fe.layout.height / 32 );
title.align = Align.Left;

gridc.num_t = fe.add_text( "[ListEntry]/[ListSize]",
			fe.layout.width/2, 0, fe.layout.width/2, fe.layout.height / 32 );
gridc.num_t.align = Align.Right;

gridc.update_frame();
