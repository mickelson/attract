///////////////////////////////////////////////////
//
// Attract-Mode Frontend - UltraStik360 plugin
//
// A plugin that allows the intergration with RocketLauncher  
// to be seamless
//
///////////////////////////////////////////////////

//
// The UserConfig class identifies plugin settings that can be configured
// from Attract-Mode's configuration menu
//

//To Do
// create a copy routine to save the files to Rocket Launcher
// create a routine to configure the emulators to have their settings changed 
// see how both points above can be achieved without it running each time the plugin is used --> ask RayGun

class UserConfig </ help="Configure the RocketLauncher integration with AttractMode" /> {

	</ label="Path to Rocketlauncher", help="Path Rocketlaucher e.g. D:\\Rocketlauncher\\", order=1 />
	rlpath = "D:\\Rocketlauncher\\";
	
	</ label="Path to Emulator Pics", help="Path to picture files representing every emulator", order=2 />
	emulatorpic = "D:\\AttractMode\\media\\systemlogos\\";
	
	</ label="screenshot(RL Background)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher game backgrounds in the fade screen", options="none, snap,wheel, fanart,marquee,flyer", order=3 />
	screenshot = "snap";
	
	</ label="wheel art(RL Logo)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher wheels/box art in the fade screen", options="none, snap,wheel, fanart,marquee,flyer", order=4 />
	wheel = "wheel";
	
	</ label="game_video(RL Video)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher videos in the fade screen", options="none, snap,wheel, fanart,marquee,flyer", order=5 />
	video = "snap";
	
	</ label="game_marquee(RL Artwork)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher game artwork in the fade screen ", options="none, snap,wheel, fanart,marquee,flyer", order=6 />
	marquee = "marquee";
	
	</ label="game_fanart(RL Artwork)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher game artwork in the fade screen", options="none, snap,wheel, fanart,marquee,flyer", order=7 />
	fanart = "fanart";
	
	</ label="game_flyer(RL Artwork)", help="Determine which AttractMode Artwork should be used to show Rocketlauncher game artwork in the fade screen", options="none, snap,wheel, fanart,marquee,flyer", order=8 />
	flyer = "flyer";	
};

// setup plugin
local config = fe.get_config();
local my_dir = fe.script_dir;
dofile( my_dir + "file_util.nut" );




// write the information to a file when a game is started
fe.add_transition_callback( "rocketlauncher_integration_transition" );


function rocketlauncher_integration_transition( ttype, var, ttime ) 
{

	switch ( ttype )
	{
		case Transition.ToGame:
	
		local rlfile = file("rl_integration.txt","wb+")
		local gameInfo = [];
		local gamestring = "#Name|Title|Emulator|CloneOf|Year|Manufacturer|Category|Players|Rotation|Control|Status|DisplayCount|DisplayType|AltRomname|AltTitle|Extra|Buttons|Rating\n";
		gameInfo.push(fe.game_info( Info.Name ));
		gameInfo.push(fe.game_info( Info.Title ));
		gameInfo.push(fe.game_info( Info.Emulator )); // current emulator
		gameInfo.push(fe.game_info( Info.CloneOf )); 
		gameInfo.push(fe.game_info( Info.Year ));
		gameInfo.push(fe.game_info( Info.Manufacturer ));
		gameInfo.push(fe.game_info( Info.Category ));
		gameInfo.push(fe.game_info( Info.Players ));
		gameInfo.push(fe.game_info( Info.Rotation ));
		gameInfo.push(fe.game_info( Info.Control ));
		gameInfo.push(fe.game_info( Info.Status ));
		gameInfo.push(fe.game_info( Info.DisplayCount ));
		gameInfo.push(fe.game_info( Info.DisplayType ));
		gameInfo.push(fe.game_info( Info.AltRomname ));
		gameInfo.push(fe.game_info( Info.AltTitle ));
		gameInfo.push(fe.game_info( Info.Extra ));
		gameInfo.push(fe.game_info( Info.Buttons ));
		gameInfo.push(fe.game_info( Info.Tags));

		foreach (value in gameInfo)
		{
			gamestring = gamestring + value + "|";
		}
		
		local screenshot = (config["screenshot"] == "none") ? "" : config["screenshot"];
		local wheel = (config["wheel"] == "none") ? "" : config["wheel"];
		local emulatorpic = config["emulatorpic"] + gameInfo[2] + ".png"
		local video = (config["video"] == "none") ? "" : config["video"];
		local marquee = (config["marquee"] == "none") ? "" : config["marquee"];
		local fanart = (config["fanart"] == "none") ? "" : config["fanart"];
		local flyer = (config["flyer"] == "none") ? "" : config["flyer"];
		
		screenshot = fe.get_art( screenshot,0,0, Art.ImagesOnly) + "|screenshot(RL Background)"
		wheel = fe.get_art( wheel ) + "|wheel art(RL Logo)"; 	
		emulatorpic = emulatorpic + "|emulator art(RL Logo)"; 
		video = fe.get_art( video ) + "|game_video(RL Video)"; 	
		marquee = fe.get_art( marquee ) + "|game_marquee(RL Artwork)";
		fanart = fe.get_art( fanart ) + "|game_fanart(RL Artwork)";
		flyer = fe.get_art( flyer ) + "|game_flyer(RL Artwork)"; 
		
		local artwork = [];
		local artworkstring = "";
		artwork.push(screenshot); // screenshot (RL Background)
		artwork.push(wheel); // current game wheel (RL Logo)
		artwork.push(emulatorpic); //  current system wheel (RL Logo)
		artwork.push(video); // videos (RL Video)
		artwork.push(marquee); // marquee (RL Artwork)
		artwork.push(fanart); // fanart (RL Artwork)
		artwork.push(flyer); // flyer (RL Artwork)

		// artwork.push("|screenshot(RL Background)");	// screenshot (RL Background)
		// artwork.push("|wheel art(RL Logo)"); 		// current game wheel (RL Logo)
		// artwork.push("|emulator art(RL Logo)"); 		// current system wheel (RL Logo)
		// artwork.push("|game_video(RL Video)"); 		// videos (RL Video)
		// artwork.push("|game_marquee(RL Artwork)"); 	// marquee (RL Artwork)
		// artwork.push("|game_fanart(RL Artwork)"); 	// fanart (RL Artwork)
		// artwork.push("|game_flyer(RL Artwork)"); 	// flyer (RL Artwork)
		
		foreach (value in artwork)
		{
			artworkstring = artworkstring + "\n" + value
		}
			
		write_ln( rlfile, gamestring + artworkstring );
		rlfile.close;
		break;

	}

	return false; // must return false
}
