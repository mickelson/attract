///////////////////////////////////////////////////
//
// Attract-Mode Frontend - FPS Monitor Plugin v1.5
// Oomek 2019
//
///////////////////////////////////////////////////

class UserConfig </ help="A plugin that shows an FPS meter" />
{
	</ label="Graph Size",
		help="Set the graph size",
		options="Very Small,Small,Medium,Large,Extra Large",
		order=1 />
	graph_size="Small"
	</ label="On/Off key",
		help="The hotkey that enables/disables the FPS monitor. This key can be mapped under Attract-Mode's 'Controls' configuration",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6,Disabled",
		order=2 />
	on_off="Disabled"
	</ label="Layout reload key",
		help="Maps a key to layout reload signal. This key can be mapped under Attract-Mode's 'Controls' configuration",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6,Disabled",
		order=3 />
	reload="Disabled"

}

class FPSMonitor
{
	fps_config = null
	fps_on_off_key = null
	fps_reload_key = null
	fps_reload_trigger = false
	fps_scale_x = 0.0
	fps_scale_y = 0.0
	fps_counter = null
	fps_visible = true
	fps_ttime_old = 0
	fps_time_delta = null
	fps_max = 0
	fps_array = null
	fps_array_idx = 0
	fps_array_size = 10
	fps_width = 16
	fps_scale = 2

	fps_graph_width = 128
	fps_graph_height = 10
	fps_background = null
	fps_graph_pixel = null
	fps_graph_surface1 = null
	fps_graph_surface2 = null
	fps_graph_surface2_clone = null
	fps_graph_surface1_clone = null

	constructor()
	{
		fps_array = array( fps_array_size, 0 )

		fps_config = fe.get_config()
		fps_on_off_key=fps_config["on_off"].tolower();
		fps_reload_key=fps_config["reload"].tolower();

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

		fps_scale_x = fe.layout.width / fe.monitors[0].width.tofloat()
		fps_scale_y = fe.layout.height / fe.monitors[0].height.tofloat()
		fps_width *= fps_scale
		fps_graph_width *= fps_scale
		fps_graph_height *= fps_scale

		fps_background = fe.add_image( "black.png" )
		fps_background.smooth = false
		fps_background.alpha = 200
		fps_background.width = ( fps_graph_width - 2.0 ) * fps_scale_x
		fps_background.height = fps_graph_height * fps_scale_y

		fps_graph_surface1 = fe.add_surface( fps_graph_width, fps_graph_height )
		fps_graph_surface1.smooth = false

		fps_graph_surface2 = fe.add_surface( fps_graph_width, fps_graph_height )
		fps_graph_surface2.smooth = false

		fps_counter = fps_graph_surface2.add_text ( "", 0, 0, fps_width * 2, fps_graph_height )
		fps_counter.font = "fps_font"

		fps_counter.set_rgb ( 100, 255, 100 )
		fps_counter.char_size = ceil( 10.0 * ( fps_scale ))
		fps_counter.margin = floor( 3.0 * fps_scale )
		fps_counter.align = Align.MiddleLeft

		fps_graph_pixel = fps_graph_surface1.add_image( "fps_graph.png", fps_graph_width - 2, 0, 1, 128 )
		fps_graph_pixel.smooth = false

		fps_graph_surface1.blend_mode = BlendMode.Premultiplied
		fps_graph_surface2.blend_mode = BlendMode.Premultiplied

		fps_graph_surface2_clone = fps_graph_surface1.add_clone( fps_graph_surface2 )
		fps_graph_surface2_clone.smooth = false

		fps_graph_surface1_clone = fps_graph_surface2.add_clone( fps_graph_surface1 )
		fps_graph_surface1_clone.smooth = false

		fps_graph_surface1.visible = false

		fps_graph_surface1_clone.x = fps_width - 1
		fps_graph_surface1_clone.subimg_x = fps_width
		fps_graph_surface1_clone.subimg_width = fps_graph_width - fps_width - 1
		fps_graph_surface1_clone.width = fps_graph_width - fps_width - 1

		fps_graph_surface2.width = fps_graph_width * fps_scale_x
		fps_graph_surface2.height = fps_graph_height * fps_scale_y
	}

	function fps_signal( signal )
	{
		if ( signal == fps_on_off_key )
		{
			if ( fps_visible == false ) fps_visible = true
			else fps_visible = false
			fps_counter.visible = fps_visible
			fps_background.visible = fps_visible
			fps_graph_surface2.visible = fps_visible
			fps_graph_pixel.visible = fps_visible
			fps_graph_surface1_clone.visible = fps_visible
			return true;
		}
		if ( signal == fps_reload_key )
		{
			fps_graph_surface1_clone.alpha = 1
			fps_reload_trigger = true
		}
		return false;
	}

	function fps_tick( fps_ttime )
	{
		if (( fps_ttime - fps_ttime_old ) > 17 )
		{
			fps_time_delta = fps_ttime - fps_ttime_old
			fps_ttime_old = fps_ttime
		}
		else
		{
			fps_time_delta = 16.6667
			fps_ttime_old += fps_time_delta
		}

		if ( fps_visible )
		{
			if ( fps_time_delta > 0 )
			{
				fps_array[fps_array_idx] = ceil( 1000.0 / fps_time_delta.tofloat() )
				fps_graph_pixel.y = -ceil( fps_array[fps_array_idx] / 60.0 * fps_graph_height ) + fps_graph_height - 1
				fps_array_idx ++
				if ( fps_array_idx >= fps_array_size ) fps_array_idx = 0
				fps_max = 0
				foreach ( f in fps_array )
					fps_max += f
				fps_max /= fps_array_size
				fps_max = ceil(fps_max)
			}
			if ( fps_max < 10 )
				fps_counter.msg = "0" + fps_max.tostring()
			else
				fps_counter.msg = fps_max.tostring()
		}
		if ( fps_reload_trigger ) fe.signal( "reload" )
	}
}

fe.plugin[ "FPSMonitor" ] <- FPSMonitor();
