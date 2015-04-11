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
			options="marquee,wheel,snap,none",
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

//
mm_arts <- [];
mm_arts_back <- [];
for ( local i=1; i<fe.monitors.len(); i++ )
{
	print( " + MultiMon Plug-in: Using Monitor #"
		+ ( fe.monitors[i].num + 1 )
		+ " (" + fe.monitors[i].width
		+ "x" + fe.monitors[i].height + ")\n" );

	local tag = "mon"+(fe.monitors[i].num+1);

	mm_arts_back.append( fe.monitors[i].add_image( "",
		0,
		0,
		fe.monitors[i].width,
		fe.monitors[i].height ) );

	mm_arts.append( fe.monitors[i].add_artwork(
		my_config[tag+"_art"],
		0,
		0,
		fe.monitors[i].width,
		fe.monitors[i].height ) );

	mm_arts.top().trigger = Transition.EndNavigation;

	if ( my_config[tag+"_preserve_aspect_ratio"] == "Yes" )
	{
		mm_arts_back.top().preserve_aspect_ratio = true;
		mm_arts.top().preserve_aspect_ratio = true;
	}
}

if ( mm_arts.len() > 0 )
{
	fe.add_ticks_callback( "MultiMon_tick" );
	fe.add_transition_callback( "MultiMon_trans" );
}

local last_tick=0;
local last_trigger=-1;
local first_tick = true;

function MultiMon_tick( ttime )
{
	if ( first_tick )
	{
		// copy artworks to mm_arts_back so we have the old image to
		// fade into when the next artwork comes up...
		for ( local i=0; i<mm_arts.len(); i++ )
			mm_arts_back[i].file_name = mm_arts[i].file_name;

		first_tick=false;
	}

	last_tick=ttime;
	if ( mm_arts[0].alpha < 255 )
	{
		local val = last_tick - last_trigger;
		if ( val < 256 )
			return;
		else if ( val < 512 )
		{
			foreach ( a in mm_arts )
				a.alpha = val - 256;

			foreach ( a in mm_arts_back )
				a.alpha = 512 - val;
		}
		else
		{
			for ( local i=0; i<mm_arts.len(); i++ )
			{
				mm_arts[i].alpha = 255;
				mm_arts_back[i].alpha = 255;
				mm_arts_back[i].file_name = mm_arts[i].file_name;
			}
		}
	}
	if ( ScreenSaverActive && ( last_tick > last_trigger + 3000 ))
	{
		foreach ( a in mm_arts )
		{
			a.alpha=0;
			a.index_offset = rand();
		}

		last_trigger = last_tick;
	}
}

function MultiMon_trans( ttype, var, ttime )
{
	if ( ttype == Transition.EndNavigation )
	{
		if ( last_trigger != last_tick )
		{
			last_trigger = last_tick;

			foreach ( a in mm_arts )
				a.alpha=0;

			return true;
		}
	}

	return false;
}
