///////////////////////////////////////////////////
//
// Attract-Mode Frontend - AutoRotate plugin
//
///////////////////////////////////////////////////
//
// Define the user-configurable options:
//
class UserConfig </ help="This plugin can automatically rotate the frontend display to match the last game played, and can also change the default rotation" /> {

	</ label="Default Rotation", help="Set the default rotation for the frontend", options="0,90,180,270" order=1 />
	default_rot="0";

	</ label="Automatic Rotation", help="Set the additional rotation to apply when automatic rotation is needed (set this to None to disable automatic rotation)", options="None,90,180,270", order=2 />
	auto_rot="270";

	</ label="Preserve Layout Aspect Ratio", help="Change the default setting for whether the overall layout aspect ratio should be preserved", options="Default,Yes,No", order=3 />
	preserve_aspect_ratio="Default";
}

// There is nothing to do if the screen saver is running
//
if ( ScreenSaverActive )
	return;

local config=fe.get_config();

if ( config["preserve_aspect_ratio"] == "Yes" )
	fe.layout.preserve_aspect_ratio = true;
else if ( config["preserve_aspect_ratio"] == "No" )
	fe.layout.preserve_aspect_ratio = false;

class AutoRotate
{
	width = 640;
	height = 480;
	adjust_base = RotateScreen.None;
	is_vert=false;
	auto_rot = RotateScreen.None;

	constructor()
	{
		width = ScreenWidth;
		height = ScreenHeight;
		adjust_base = RotateScreen.None;

		switch ( config["default_rot"] )
		{
		case "90":
			width = ScreenHeight;
			height = ScreenWidth;
			adjust_base = RotateScreen.Right;
			break;

		case "180":
			adjust_base = RotateScreen.Flip;
			break;

		case "270":
			width = ScreenHeight;
			height = ScreenWidth;
			adjust_base = RotateScreen.Left;
			break;

		case "0":
		default:
			break;
		};

		is_vert=false;
		if ( width < height )
			is_vert=true;

		if ( adjust_base != RotateScreen.None )
		{
			fe.layout.base_rotation =
				( fe.layout.base_rotation + adjust_base ) % 4;
		}

		auto_rot = RotateScreen.None;
		switch ( config["auto_rot"] )
		{
		case "90": auto_rot = RotateScreen.Right; break;
		case "180": auto_rot = RotateScreen.Flip; break;
		case "270": auto_rot = RotateScreen.Left; break;
		case "None":
		default:
			break;
		}

		if ( auto_rot != RotateScreen.None )
		{
			//
			// We only register the callback function if we will be doing
			// autorotations (the user might enable the plugin just to change
			// the default rotations)
			//
			fe.add_transition_callback( this, "on_transition" );
		}
	}

	function do_auto_rotate( r )
	{
		return (
			(( is_vert ) && (( r == "0" ) || ( r == "180" )))
			|| (( !is_vert ) && (( r == "90" ) || ( r == "270" )))
			);
	}

	function on_transition( ttype, var, ttime )
	{
		if ( ttype == Transition.FromGame )
		{
			if ( do_auto_rotate( fe.game_info( Info.Rotation ) ) )
				fe.layout.toggle_rotation = auto_rot;
			else
				fe.layout.toggle_rotation = RotateScreen.None;
		}

		return false; // must return false
	}
}

fe.plugin[ "AutoRotate" ] <- AutoRotate();
