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

	</ label="Flow", help="Select the flow direction of the grid", options="Horizontal,Vertical", order=4 />
	flow="Vertical";

	</ label="Preserve Aspect Ratio", help="Preserve artwork aspect ratio", options="Yes,No", order=5 />
	aspect_ratio="Yes";

	</ label="Enable Video", help="Enable video artwork in the grid", options="Yes,No", order=6 />
	video="Yes";

	</ label="Overlay Wheel", help="Overlay the wheel artwork over grid entries?", options="Yes,No", order=7 />
	wheel="No";


	</ label="Transition Time", help="The amount of time (in milliseconds) that it takes to scroll to another grid entry", order=8 />
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

local vert_flow = ( my_config["flow"] != "Horizontal" );

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
		stride = fe.layout.page_size = vert_flow ? rows : cols;
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

	function update_audio()
	{
		local flag1 = Vid.NoAudio;
		local flag2 = 0;
		if ( my_config["video"] == "No" )
		{
			flag1 = Vid.ImagesOnly | Vid.NoAudio;
			flag2 = Vid.ImagesOnly;
		}

		foreach ( o in m_objs )
			o.m_art.video_flags = flag1;

		local sel = get_sel();
		m_objs[ sel ].m_art.video_flags = flag2;
	}

	function update_frame()
	{
		update_audio();

		frame.x = width * sel_x;
		frame.y = fe.layout.height / 24 + height * sel_y;
		
		name_t.index_offset = num_t.index_offset = get_sel() - selection_index;	
	}

	function do_correction()
	{
		local corr = get_sel() - selection_index;
		foreach ( o in m_objs )
		{
			local idx = o.m_art.index_offset - corr;
			o.m_art.rawset_index_offset( idx );
			if ( o.m_wheels )
			{
				o.m_wheel.rawset_index_offset( idx );
				o.m_wheels.rawset_index_offset( idx );
				o.m_wheelss.rawset_index_offset( idx );
			}
		}
	}

	function get_sel()
	{
		return vert_flow ? ( sel_x * rows + sel_y ) : ( sel_y * cols + sel_x );
	}

	function on_signal( sig )
	{
		switch ( sig )	
		{
		case "up":
			if ( vert_flow && ( sel_y > 0 ) )
			{
				sel_y--;
				update_frame();
			}
			else if ( !vert_flow && ( sel_x > 0 ) )
			{
				sel_x--;
				update_frame();
			}
			else
			{
				transition_swap_point=0.5;
				do_correction();
				fe.signal( "prev_game" );
			}
			return true;

		case "down":
			if ( vert_flow && ( sel_y < rows - 1 ))
			{
				sel_y++;
				update_frame();
			}
			else if ( !vert_flow && ( sel_x < cols - 1 ) )
			{
				sel_x++;
				update_frame();
			}
			else
			{
				transition_swap_point=0.5;
				do_correction();
				fe.signal( "next_game" );
			}
			return true;

		case "left":
			if ( vert_flow && ( sel_x > 0 ))
			{
				sel_x--;
				update_frame();
			}
			else if ( !vert_flow && ( sel_y > 0 ) )
			{
				sel_y--;
				update_frame();
			}
			else
			{
				transition_swap_point=0.0;
				do_correction();
				fe.signal( "prev_page" );
			}
			return true;

		case "right":
			if ( vert_flow && ( sel_x < cols - 1 ) )
			{
				sel_x++;
				update_frame();
			}
			else if ( !vert_flow && ( sel_y < rows - 1 ) )
			{
				sel_y++;
				update_frame();
			}
			else
			{
				transition_swap_point=0.0;
				do_correction();
				fe.signal( "next_page" );
			}
			return true;


		case "exit":
		case "exit_no_menu":
			break;
		case "select":
		default:
			// Correct the list index if it doesn't align with
			// the game our frame is on
			//
			enabled=false; // turn conveyor off for this switch
			local frame_index = get_sel();
			fe.list.index += frame_index - selection_index;

			set_selection( frame_index );
			update_frame();
			enabled=true; // re-enable conveyor
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
					if ( num < 1 )
						num = 1;

					local temp = 510 * ( num - r - c ) / num * ttime / transition_ms;
					m_objs[i].set_alpha( ( temp > 255 ) ? 255 : temp );
				}

				frame.alpha = 255 * ttime / transition_ms;
				return true;
			}

			local old_alpha = m_objs[ m_objs.len()-1 ].m_art.alpha;

			foreach ( o in m_objs )
				o.set_alpha( 255 );

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
					if ( num < 1 )
						num = 1;

					local temp = 255 - 510 * ( num - r - c ) / num * ttime / transition_ms;
					m_objs[i].set_alpha( ( temp < 0 ) ? 0 : temp );
				}
				frame.alpha = 255 - 255 * ttime / transition_ms;
				return true;
			}

			local old_alpha = m_objs[ m_objs.len()-1 ].m_art.alpha;

			foreach ( o in m_objs )
				o.set_alpha( 0 );

			frame.alpha = 0;

			if ( old_alpha != 0 )
				return true;

			break;
		case Transition.FromOldSelection:
		case Transition.ToNewList:
			update_audio();

			foreach ( o in m_objs )
				o.dim_if_needed();
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
	m_art = null;
	m_wheel = null;
	m_wheels = null;
	m_wheelss = null;

	constructor( num )
	{
		m_num = num;

		m_art = fe.add_artwork( my_config["art"], 0, 0,
				width - 2*PAD, height - 2*PAD );

		m_art.preserve_aspect_ratio = (my_config["aspect_ratio"]=="Yes");
		m_art.video_flags = Vid.NoAudio;
		if ( my_config["video"] == "No" )
			m_art.video_flags = Vid.ImagesOnly | Vid.NoAudio;

		m_art.alpha = 0;

		if ( my_config["wheel"] == "Yes" )
		{
			m_wheels = fe.add_artwork( "wheel", 0, 0,
					width - 4*PAD, height - 4*PAD );
			m_wheels.preserve_aspect_ratio = true;
			m_wheelss = fe.add_clone( m_wheels );
			m_wheel = fe.add_clone( m_wheels );
			m_wheels.set_rgb( 0, 0, 0 );
			m_wheelss.set_rgb( 0, 0, 0 );
		}

		base.constructor();
	}

	function on_progress( progress, var )
	{
		if ( var == 0 )
			m_shifted = false;

		if ( vert_flow )
		{
			local r = m_num % rows;
			local c = m_num / rows;

			if ( abs( var ) < rows )
			{
				m_art.x = c * width + PAD;
				m_art.y = fe.layout.height / 24
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

				m_art.x = ( c + prog ) * width + PAD;
				m_art.y = fe.layout.height / 24 + r * height + PAD;
			}
		}
		else
		{
			local r = m_num / cols;
			local c = m_num % cols;

			if ( abs( var ) < cols )
			{
				m_art.x = fe.layout.width * ( progress * rows - r ) + PAD;
				m_art.y = fe.layout.height / 24 + r * height + PAD;
			}
			else
			{
				local prog = ::gridc.transition_progress;
				if ( prog > ::gridc.transition_swap_point )
				{
					if ( var > 0 ) r++;
					else r--;
				}

				if ( var > 0 ) prog *= -1;

				m_art.x = c * width + PAD;
				m_art.y = fe.layout.height / 24 + ( r + prog ) * height + PAD;
			}
		}

		if ( m_wheel )
		{
			m_wheels.x = m_art.x + PAD + 1;
			m_wheels.y = m_art.y + PAD + 1;
			m_wheelss.x = m_art.x + PAD - 1;
			m_wheelss.y = m_art.y + PAD - 1;
			m_wheel.x = m_art.x + PAD;
			m_wheel.y = m_art.y + PAD;
		}

		dim_if_needed();
	}

	function swap( other )
	{
		m_art.swap( other.m_art );
		if ( m_wheel )
		{
			m_wheel.swap( other.m_wheel );
			m_wheels.swap( other.m_wheels );
			m_wheelss.swap( other.m_wheelss );
		}
	}

	function set_index_offset( io )
	{
		m_art.index_offset = io;
		if ( m_wheel )
		{
			m_wheel.index_offset = io;
			m_wheels.index_offset = io;
			m_wheelss.index_offset = io;
		}
	}

	function reset_index_offset()
	{
		m_art.rawset_index_offset( m_base_io ); 
		if ( m_wheel )
		{
			m_wheel.rawset_index_offset( m_base_io );
			m_wheels.rawset_index_offset( m_base_io );
			m_wheelss.rawset_index_offset( m_base_io );
		}
	}

	function set_alpha( alpha )
	{
		m_art.alpha = alpha; 
		if ( m_wheel )
		{
			m_wheel.alpha = alpha;
			m_wheels.alpha = alpha;
			m_wheelss.alpha = alpha;
		}
	}

	function dim_if_needed()
	{
		if ( m_wheel )
		{
			//
			// If we have an art and a wheel, make the art a bit darker
			//
			if (( m_wheel.file_name.len() > 0 )
					&& ( m_art.file_name.len() > 0 ))
				m_art.set_rgb( 140, 140, 140 );
			else
				m_art.set_rgb( 255, 255, 255 );
		}
	}
}

local my_array = [];
for ( local i=0; i<rows*cols; i++ )
	my_array.push( MySlot( i ) );

gridc.set_slots( my_array, gridc.get_sel() );
gridc.frame=fe.add_image( "frame.png", width * 2, height * 2, width, height );

gridc.name_t =  fe.add_text( "[Title]", 0,
		fe.layout.height - fe.layout.height / 24 - PAD,
		fe.layout.width, fe.layout.height / 24 );

local title = fe.add_text( "[DisplayName]/[FilterName]",
			0, 0, fe.layout.width/2, fe.layout.height / 32 );
title.align = Align.Left;

gridc.num_t = fe.add_text( "[ListEntry]/[ListSize]",
			fe.layout.width/2, 0, fe.layout.width/2, fe.layout.height / 32 );
gridc.num_t.align = Align.Right;

gridc.update_frame();
