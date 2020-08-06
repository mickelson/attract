///////////////////////////////////////////////////
//
// Attract-Mode Frontend - FPS Monitor Plugin v2.2
// Radek Dutkiewicz (Oomek) 2020
//
///////////////////////////////////////////////////

class UserConfig </ help="A plugin that shows an FPS meter v2.2" />
{
	</ label="Graph Size",
		help="Set the graph size",
		options="Very Small,Small,Medium,Large,Extra Large",
		order=1 />
	graph_size="Small"

	</ label="Frame time graph",
		help="Show the frame time graph",
		options="Yes,No",
		order=2 />
	ft_enabled="No"

	</ label="Frame time graph speed",
		help="Set the scroll speed of the frame time graph",
		options="Slow,Medium,Fast",
		order=3 />
	ft_speed="Fast"

	</ label="On/Off key",
		help="The hotkey that enables/disables the FPS monitor. This key can be mapped under Attract-Mode's 'Controls' configuration",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6,Disabled",
		order=4 />
	on_off="Disabled"

	</ label="Layout reload key",
		help="Maps a key to layout reload signal. This key can be mapped under Attract-Mode's 'Controls' configuration",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6,Disabled",
		order=5 />
	reload="Disabled"

}

class FPSMonitor
{
	static FPS_ZORDER = 2147483647 // maximum signed int so it's always sitting on top
	fps_flw = null
	fps_flh = null
	fps_config = null
	fps_on_off_key = null
	fps_reload_key = null
	fps_reload_trigger = false
	fps_scale_y = 0.0
	fps_text = null
	fps_enabled = true
	fps_ttime_old = 0
	fps_time_delta = 0
	fps_max = 0
	fps_array = null
	fps_array_idx = 0
	fps_array_size = 10
	fps_width = 17
	fps_height = 11
	fps_scale = 2
	fps_frame_time = 0

	fps_graph_width = 64
	fps_graph_height = 0.0
	fps_background = null
	fps_graph = []

	ft_enabled = false
	ft_background = null
	ft_bar_width = 0.0
	ft_ttime_old = 0
	ft_graph = []
	ft_array = null
	ft_array_idx = 0
	ft_array_size = 60
	ft_text = null
	ft_max = 0

	constructor()
	{
		fps_flw = fe.layout.width
		fps_flh = fe.layout.height
		fps_frame_time = 1000.0 / ScreenRefreshRate
		fps_array = array( fps_array_size, 0 )

		fps_config = fe.get_config()
		fps_on_off_key=fps_config["on_off"].tolower();
		fps_reload_key=fps_config["reload"].tolower();
		if ( fps_config["ft_enabled"].tolower() == "yes" )
			ft_enabled = true

		switch ( fps_config["graph_size"].tolower() )
		{
			case "very small":
				fps_scale = 1
				break
			case "small":
				fps_scale = 2
				break
			case "medium":
				fps_scale = 3
				break
			case "large":
				fps_scale = 4
				break
			case "extra large":
				fps_scale = 6
				break
			default:
				break
		}

		fe.add_signal_handler( this, "fps_signal" )
		fe.add_ticks_callback( this, "fps_tick" )

		fps_scale_y = fe.layout.height / ScreenHeight.tofloat()
		fps_width *= fps_scale * fps_scale_y
		fps_height *= fps_scale
		fps_graph_width *= fps_scale * fps_scale_y
		fps_graph_height = fps_height * fps_scale_y

		fps_background = fe.add_text( "", 0, 0, 0, 0 )
		fps_background.zorder = FPS_ZORDER
		fps_background.width = fps_width + fps_graph_width
		fps_background.width = floor( fps_width + fps_graph_width )
		fps_background.height = fps_graph_height
		fps_background.set_bg_rgb( 10, 25, 15 )
		fps_background.bg_alpha = 240
		fps_background.char_size = 0

		fps_text = fe.add_text( "00", 0, 0, 0, 0 )
		fps_text.zorder = FPS_ZORDER
		fps_text.font = "fps_font.ttf"
		fps_text.set_rgb( 100, 255, 100 )
		fps_text.width = fps_width
		fps_text.height = fps_graph_height
		fps_text.char_size = floor( 10.0 * ( fps_scale * fps_scale_y ))
		fps_text.margin = 0
		fps_text.align = Align.MiddleCentre

		// FPS Graph
		fps_graph.push( fe.add_image( "fps_graph.png", 0, 0, 0, 0 ))
		fps_graph[0].zorder = FPS_ZORDER
		fps_graph[0].set_rgb( 100, 255, 100 )
		fps_graph[0].smooth = false
		fps_graph[0].subimg_x = 0 // graph type
		fps_graph[0].subimg_width = 1
		fps_graph[0].x = fps_width
		fps_graph[0].width = 1
		fps_graph[0].height = fps_graph_height
		fps_graph[0].subimg_height = fps_graph[0].height / fps_scale_y

		fps_graph[0].subimg_y = -ScreenHeight
		for ( local i = 1; i < floor( fps_graph_width ); i++ )
		{
			local obj = fe.add_clone( fps_graph[0] )
			obj.x = fps_width + i
			fps_graph.push( obj )
		}
		fps_background.width = fps_graph.top().x + fps_graph.top().width

		// Frame Time Graph
		if ( ft_enabled )
		{
			switch ( fps_config["ft_speed"].tolower() )
			{
				case "slow":
					ft_bar_width = 1 * fps_scale_y
					break
				case "medium":
					ft_bar_width = 2 * fps_scale_y
					break
				case "fast":
					ft_bar_width = 4 * fps_scale_y
					break
				default:
					ft_bar_width = 2 * fps_scale_y
					break
			}
			ft_background = fe.add_text( "", 0, 0, 0, 0 )
			ft_background.zorder = FPS_ZORDER
			ft_background.width = floor( fps_flw / 2.0 / ft_bar_width ) * ft_bar_width
			ft_background.height = fps_background.height * 4
			ft_background.x = floor( fps_flw / 4.0 )
			ft_background.y = fps_flh - ft_background.height
			ft_background.bg_alpha = 200
			ft_background.char_size = 0
			ft_background.set_bg_rgb( 10, 25, 15 )
			ft_background.bg_alpha = 240

			ft_graph.push( fe.add_clone( fps_graph[0] ))
			ft_graph[0].zorder = FPS_ZORDER
			ft_graph[0].set_rgb( 100, 255, 100 )
			ft_graph[0].smooth = false
			ft_graph[0].subimg_x = 1 // graph type
			ft_graph[0].subimg_width = 1
			ft_graph[0].x = ft_background.x
			ft_graph[0].y = ft_background.y
			ft_graph[0].width = ft_bar_width
			ft_graph[0].height = ft_background.height
			ft_graph[0].subimg_y = -ScreenHeight
			ft_graph[0].subimg_height = ft_graph[0].height / fps_scale_y
			local ft_bars = floor( fps_flw / 2.0 / ft_bar_width )
			for ( local i = 1; i < ft_bars; i++ )
			{
				local obj = fe.add_clone( ft_graph[0] )
				obj.x = ft_background.x + i * ft_bar_width
				ft_graph.push( obj )
			}
			ft_text = fe.add_text( "", 0, 0, 0, 0 )
			ft_text.zorder = FPS_ZORDER
			ft_text.font = "fps_font"
			ft_text.set_rgb ( 100, 255, 100 )
			ft_text.x = ft_background.x
			ft_text.y = ft_background.y
			ft_text.width = fps_width * 3
			ft_text.height = fps_width * 3
			ft_text.char_size = ceil( 10.0 * ( fps_scale * fps_scale_y ))
			ft_text.align = Align.TopLeft
			ft_text.margin = 4 * fps_scale_y * fps_scale

			ft_array = array( ft_array_size, 0 )
		}
	}

	function fps_signal( signal )
	{
		if ( signal == fps_on_off_key )
		{
			if ( fps_enabled == false ) fps_enabled = true
			else fps_enabled = false
			fps_text.visible = fps_enabled
			fps_background.visible = fps_enabled
			foreach ( c in fps_graph )
			{
				c.visible = fps_enabled
				c.subimg_y = -ScreenHeight
			}
			if ( ft_enabled )
			{
				ft_text.visible = fps_enabled
				ft_background.visible = fps_enabled
				foreach ( c in ft_graph )
				{
					c.visible = fps_enabled
					c.subimg_y = -ScreenHeight
				}
			}
			return true;
		}
		if ( signal == fps_reload_key )
		{
			fps_reload_trigger = true
		}
		return false;
	}

	function fps_tick( ttime )
	{
		if (( ttime - fps_ttime_old ) > fps_frame_time )
		{
			fps_time_delta = ttime - fps_ttime_old
			fps_ttime_old = ttime
		}
		else
		{
			fps_time_delta = fps_frame_time
			fps_ttime_old += fps_time_delta
		}

		if ( fps_enabled )
		{
			if ( fps_time_delta > 0.0 )
			{
				fps_array[fps_array_idx] = floor( 1000.0 / fps_time_delta.tofloat() + 0.5 )
				for ( local i = 0; i < fps_graph.len() - 1; i++ )
					fps_graph[i].subimg_y = fps_graph[i+1].subimg_y

				fps_graph[fps_graph.len() - 1].subimg_y = -fps_height + ( fps_array[fps_array_idx] / ScreenRefreshRate * fps_height ) + 2

				fps_array_idx ++
				if ( fps_array_idx >= fps_array_size ) fps_array_idx = 0
				fps_max = 0
				foreach ( f in fps_array )
					fps_max += f

				fps_max = floor( fps_max / fps_array_size + 0.5 )
			}
			if ( fps_max < 10 )
				fps_text.msg = "0" + fps_max
			else
				fps_text.msg = fps_max

			if ( ft_enabled )
			{
				for ( local i = 0; i < ft_graph.len() - 1; i++ )
					ft_graph[i].subimg_y = ft_graph[i+1].subimg_y

				ft_graph[ft_graph.len() - 1].subimg_y = (( ttime - ft_ttime_old ) * ft_graph[0].subimg_height ) / ( fps_frame_time * 2.0 ) - ft_graph[0].subimg_height

 				ft_array[ft_array_idx] = ttime - ft_ttime_old
				ft_array_idx ++
				if ( ft_array_idx >= ft_array_size ) ft_array_idx = 0
				ft_max = 0
				foreach ( f in ft_array )
					if ( f > ft_max ) ft_max = f

				ft_text.msg = ft_max
			}
		}

		ft_ttime_old = ttime
		if ( fps_reload_trigger ) fe.signal( "reload" )
	}
}

fe.plugin["FPSMonitor"] <- FPSMonitor();
