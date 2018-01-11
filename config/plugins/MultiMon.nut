/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - Basic multi-monitor support plugin
//
//
class UserConfig </ help="This plugin provides basic multi-monitor support" />
{
}

// Add UserConfig entries for each extra monitor
local tagOrder = 1;
for ( local i=1; i<fe.monitors.len(); i++ )
{
	local monDisplayNum = (fe.monitors[i].num+1);
	local tag="mon" + monDisplayNum;
	UserConfig[tag+"_art"] <- "marquee";
	UserConfig.setattributes(tag+"_art",
		{
			label="Monitor " + monDisplayNum + ": Artwork",
			help="The artwork to display on this monitor",
			options="marquee,wheel,snap,flyer,fanart,none",
			order=tagOrder++
		} );
	
	UserConfig[tag+"_preserve_aspect_ratio"] <- "Yes";
	UserConfig.setattributes(tag+"_preserve_aspect_ratio",
		{
			label="Monitor " + monDisplayNum
				+ ": Preserve Aspect Ratio",
			help="Preserve the artwork aspect ratio?",
			options="Yes,No",
			order=tagOrder++
		} );

	UserConfig[tag+"_x"] <- 0;
	UserConfig.setattributes(tag+"_x",
		{
			label="Monitor " + monDisplayNum + ": X",
			help="X coordinate of the top of the artwork",
			order=tagOrder++
		} );

	UserConfig[tag+"_y"] <- 0;
	UserConfig.setattributes(tag+"_y",
		{
			label="Monitor " + monDisplayNum + ": Y",
			help="Y coordinate of the top of the artwork",
			order=tagOrder++
		} );

	UserConfig[tag+"_width"] <- fe.monitors[i].width;
	UserConfig.setattributes(tag+"_width",
		{
			label="Monitor " + monDisplayNum + ": Width",
			help="Width of the artwork",
			order=tagOrder++
		} );

	UserConfig[tag+"_height"] <- fe.monitors[i].height;
	UserConfig.setattributes(tag+"_height",
		{
			label="Monitor " + monDisplayNum + ": Height",
			help="Height of the artwork",
			order=tagOrder++
		} );
}

local my_config = fe.get_config();

fe.load_module( "fade" );

class MultiMon
{
	ss_interval = 3000; // time (in ms) between switches (screensaver)

	constructor()
	{
       ::fe.add_transition_callback( this, "onTransition" );
	}


	function onTransition( ttype, var, transition_time )
	{
		// Initialize on StartLayout
		if ((_initialized == false) && (ttype == Transition.StartLayout))
		{
			return initializeArtwork();
		}

		// If we're transitioning to game, make sure any artwork that's mid-fade completes.
		if ((_initialized == true) && (ttype == Transition.ToGame))
		{
			return transitionToGame();
		}

		return false;
	}

	function initializeArtwork()
	{
		// If we're already initialized, return.
		if (_initialized == true)
		{
			return false;
		}

		_initialized = true;

		for ( local i=1; i<fe.monitors.len(); i++ )
		{
			print( " + MultiMon Plug-in: Using Monitor #"
				+ ( fe.monitors[i].num + 1 )
				+ " (" + fe.monitors[i].width
				+ "x" + fe.monitors[i].height + ")\n" );

			local tag = "mon"+(fe.monitors[i].num+1);

			local mon = fe.monitors[i];

			mon.add_image("", 0, 0, 100, 100);

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
	
			// Support configurable artwork sizes
			local left = 0;
			local top = 0;
			local width = fe.monitors[i].width;
			local height = fe.monitors[i].height;

			if ( (tag+"_x") in my_config)
				left = my_config[tag+"_x"].tointeger();
			if ( (tag+"_y") in my_config)
				top = my_config[tag+"_y"].tointeger();
			if ( (tag+"_height") in my_config)
				height = my_config[tag+"_height"].tointeger();
			if ( (tag+"_width") in my_config)
				width = my_config[tag+"_width"].tointeger();
				
			print("+ MultiMon Plug-in: Calling FadeArt: " + my_config[tag+"_art"] + "," + left + "," + top + "," + width + "," + height + "," + fe.monitors[i] + "\n");

			local n = FadeArt(
				my_config[tag+"_art"],
				left,
				top,
				width,
				height,
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

		return false;
	}

	function transitionToGame()
	{
		local redraw = false;

		// If we're transitioning to game, make sure any artwork that's mid-fade completes.
		foreach(fadeArt in _list)
		{
			if ( fadeArt._in_fade )
			{
				fadeArt.flip();
				// We need to redraw 2 frames because drawing appears to be double-buffered.
				_redraw_frames=2;
			}
		}

		if (_redraw_frames > 0)
		{
			_redraw_frames--;
			redraw = true;
		}

		// This tells Attract Mode to immediately refresh the screen if we return true.
		return redraw;
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
	_initialized = false;
	_last_trigger=0;
	_redraw_frames = 0;
}

fe.plugin["MultiMon"] <- MultiMon();
