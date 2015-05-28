/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - Basic multi-monitor support plugin
//
//
class UserConfig </ help="This plugin provides basic multi-monitor support" />
{
}

// Add UserConfig entries for each extra monitor
for ( local i=1; i<fe.monitors.len(); i++ )
{
	local tag="mon" + (fe.monitors[i].num+1);
	UserConfig[tag+"_art"] <- "marquee";
	UserConfig.setattributes(tag+"_art",
		{
			label="Monitor " + fe.monitors[i].num + ": Artwork",
			help="The artwork to display on this monitor",
			options="marquee,wheel,snap,flyer,fanart,none",
			order=i*2
		} );
	
	UserConfig[tag+"_preserve_aspect_ratio"] <- "Yes";
	UserConfig.setattributes(tag+"_preserve_aspect_ratio",
		{
			label="Monitor " + fe.monitors[i].num
				+ ": Preserve Aspect Ratio",
			help="Preserve the artwork aspect ratio?",
			options="Yes,No",
			order=i*2+1
		} );
}

local my_config = fe.get_config();

fe.load_module( "fade" );

class MultiMon
{
	ss_interval = 3000; // time (in ms) between switches (screensaver)

	constructor()
	{
		for ( local i=1; i<fe.monitors.len(); i++ )
		{
			print( " + MultiMon Plug-in: Using Monitor #"
				+ ( fe.monitors[i].num + 1 )
				+ " (" + fe.monitors[i].width
				+ "x" + fe.monitors[i].height + ")\n" );

			local tag = "mon"+(fe.monitors[i].num+1);

			// Force the whole screen to black in case there
			// is spillover from the main screen's layout
			local blank = fe.monitors[i].add_text( "",
				0,
				0,
				fe.monitors[i].width,
				fe.monitors[i].height );
			blank.set_bg_rgb( 0, 0, 0 );
			blank.bg_alpha = 255;

			if ( my_config[tag+"_art"] == "none" )
				continue;

			local n = FadeArt(
				my_config[tag+"_art"],
				0,
				0,
				fe.monitors[i].width,
				fe.monitors[i].height,
				fe.monitors[i] );

			n.video_flags = Vid.NoAudio;

			if ( my_config[tag+"_preserve_aspect_ratio"] == "Yes" )
				n.preserve_aspect_ratio = true;

			if ( ScreenSaverActive )
				n.interval = ss_interval;

			_list.append( n );
		}

		if (( _list.len() > 0 ) && ScreenSaverActive )
			fe.add_ticks_callback( this, "on_tick" );
	}

	function on_tick( ttime )
	{
		if ( ttime > _last_trigger + ss_interval )
		{
			foreach ( o in _list )
				o.index_offset = rand();

			_last_trigger = ttime;
		}
	}

	_list=[];
	_last_trigger=0;
}

fe.plugin["MultiMon"] <- MultiMon();
