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
	fps_scale_x = 0.0
	fps_scale_y = 0.0
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

	fps_graph_width = 64.0
	fps_graph_height = 0.0

	ft_enabled = false
	ft_bar_width = 0.0
	ft_ttime_old = 0
	ft_array = null
	ft_array_idx = 0
	ft_array_size = 60
	ft_text = null
	ft_max = 0

	fps_su = null
	fps_bg = null
	fps_line = null
	fps_clear = true
	fps_text = null

	ft_su = null
	ft_bg = null
	ft_line = null
	ft_clear = true

	constructor()
	{
		local png = fe.add_image( "fps_graph.png", 0, 0, 0, 0 )
		png.subimg_width = 1
		png.subimg_height = 1
		png.set_rgb( 100, 255, 100 )

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

		fps_scale_x = fe.layout.width / ScreenWidth.tofloat()
		fps_scale_y = fe.layout.height / ScreenHeight.tofloat()
		fps_width *= fps_scale * fps_scale_y
		fps_height *= fps_scale

		fps_graph_width *= fps_scale.tofloat() * fps_scale_y
		fps_graph_height = fps_height * fps_scale_y

		fps_text = fe.add_text( "00", 0, 0, 0, 0 )
		fps_text.zorder = FPS_ZORDER
		fps_text.font = "fps_font.ttf"
		fps_text.set_rgb( 100, 255, 100 )
		fps_text.set_bg_rgb( 10, 25, 10 )
		fps_text.bg_alpha = 240
		fps_text.width = fps_width
		fps_text.height = fps_graph_height
		fps_text.char_size = floor( 10.0 * ( fps_scale * fps_scale_y ))
		fps_text.margin = 0
		fps_text.align = Align.MiddleCentre

		// FPS Graph
		local fps_su_width = floor( fps_graph_width / fps_scale_x )
		local fps_su_height = fps_graph_height / fps_scale_y
		fps_su = fe.add_surface( fps_su_width, fps_su_height )
		fps_su.width = fps_su.width * fps_scale_x
		fps_su.height = fps_graph_height
		fps_su.x = floor( fps_text.width / fps_scale_x + 0.5 ) * fps_scale_x
		fps_su.y = 0
		fps_su.repeat = true
		fps_su.clear = false
		fps_su.smooth = false
		fps_su.zorder = FPS_ZORDER
		fps_su.blend_mode = BlendMode.Alpha

		fps_bg = fps_su.add_clone( png )
		fps_bg.smooth = false
		fps_bg.x = 0
		fps_bg.y = 0
		fps_bg.width = fps_su.subimg_width
		fps_bg.height = fps_su.subimg_height
		fps_bg.blend_mode = BlendMode.None

		fps_line = fps_su.add_clone( png )
		fps_line.set_rgb( 100, 255, 100 )
		fps_line.smooth = false
		fps_line.subimg_x = 0 // graph type
		fps_line.width = 1
		fps_line.height = fps_su.subimg_height
		fps_line.subimg_width = 1
		fps_line.subimg_height = fps_su.subimg_height
		fps_line.blend_mode = BlendMode.None

		// Frame Time Graph
		if ( ft_enabled )
		{
			switch ( fps_config["ft_speed"].tolower() )
			{
				case "slow":
					ft_bar_width = 1
					break
				case "medium":
					ft_bar_width = 2
					break
				case "fast":
					ft_bar_width = 4
					break
				default:
					ft_bar_width = 2
					break
			}

			local ft_su_width = ceil( ScreenWidth / 2.0 / ft_bar_width ) * ft_bar_width
			local ft_su_height = fps_graph_height * 4 / fps_scale_y
			ft_su = fe.add_surface( ft_su_width, ft_su_height )
			ft_su.width = ft_su.subimg_width * fps_scale_x
			ft_su.height = fps_graph_height * 4
			ft_su.x = floor( ScreenWidth / 4.0 ) * fps_flw / ScreenWidth
			ft_su.y = fps_flh - ft_su.height
			ft_su.repeat = true
			ft_su.clear = false
			ft_su.smooth = false
			ft_su.zorder = FPS_ZORDER
			ft_su.blend_mode = BlendMode.Alpha

			ft_bg = ft_su.add_clone( png )
			ft_bg.smooth = false
			ft_bg.x = 0
			ft_bg.y = 0
			ft_bg.width = ft_su.subimg_width
			ft_bg.height = ft_su.subimg_height
			ft_bg.blend_mode = BlendMode.None

			ft_line = ft_su.add_clone( png )
			ft_line.set_rgb( 100, 255, 100 )
			ft_line.smooth = false
			ft_line.subimg_x = 1 // graph type
			ft_line.width = ft_bar_width
			ft_line.height = ft_su.subimg_height
			ft_line.subimg_width = 1
			ft_line.subimg_height = ft_su.subimg_height
			ft_line.blend_mode = BlendMode.None

			ft_text = fe.add_text( "", 0, 0, 0, 0 )
			ft_text.zorder = FPS_ZORDER
			ft_text.font = "fps_font"
			ft_text.set_rgb ( 100, 255, 100 )
			ft_text.x = floor( fps_flw / 4.0 )
			ft_text.y = fps_flh - ft_su.height
			ft_text.width = fps_width * 3
			ft_text.height = fps_width * 3
			ft_text.char_size = ceil( 10.0 * ( fps_scale * fps_scale_y ))
			ft_text.align = Align.TopLeft
			ft_text.margin = 4 * fps_scale_y * fps_scale

			ft_array = array( ft_array_size, 0 )
		}
		png.visible = false
	}

	function fps_signal( signal )
	{
		if ( signal == fps_on_off_key )
		{
			if ( fps_enabled == false ) fps_enabled = true
			else fps_enabled = false
			fps_text.visible = fps_enabled
			fps_clear = true
			fps_bg.visible = fps_enabled
			fps_su.visible = fps_enabled

			if ( ft_enabled )
			{
				ft_text.visible = fps_enabled
				ft_clear = true
				ft_bg.visible = fps_enabled
				ft_su.visible = fps_enabled
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

				fps_su.subimg_x = ( fps_su.subimg_x + 1 ) % fps_su.subimg_width
				fps_line.subimg_y = ceil( fps_array[fps_array_idx] / ScreenRefreshRate * fps_su.subimg_height - fps_su.subimg_height ) + 2
				fps_line.x = ( fps_su.subimg_width + fps_su.subimg_x - 1 ) % fps_su.subimg_width

				if ( fps_clear == false ) fps_bg.visible = false
				fps_clear = false

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
 				ft_array[ft_array_idx] = ttime - ft_ttime_old
				ft_array_idx ++
				if ( ft_array_idx >= ft_array_size ) ft_array_idx = 0
				ft_max = 0
				foreach ( f in ft_array )
					if ( f > ft_max ) ft_max = f

				ft_text.msg = ft_max

				ft_su.subimg_x = ( ft_su.subimg_x + ft_bar_width ) % ft_su.subimg_width
				ft_line.subimg_y = (( ttime - ft_ttime_old ) * ft_line.subimg_height ) / ( fps_frame_time * 2.0 ) - ft_line.subimg_height
				ft_line.x = ( ft_su.subimg_width + ft_su.subimg_x - ft_bar_width ) % ft_su.subimg_width // NEW WRAP, doesn't go negative

				if ( ft_clear == false ) ft_bg.visible = false
				ft_clear = false
			}
		}

		ft_ttime_old = ttime
		if ( fps_reload_trigger ) fe.signal( "reload" )
	}
}

fe.plugin["FPSMonitor"] <- FPSMonitor();
