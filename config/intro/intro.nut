//////////////////////////////////////////////////
//
// Attract-Mode Frontend - Enhanced intro layout
//
// Based on original intro script, updated by Chris Van Graas
//
///////////////////////////////////////////////////
class UserConfig
{
    </ label="Play intro", help="Toggle playback of intro video when Attract-Mode starts", options="Yes,No", order=1 />
    play_intro = "Yes";

    </ label="Detect aspect ratio", help="Toggle detection of aspect ratio (if disabled, default video will play)", options="Yes,No", order=2 />
    detect_aspect = "Yes";

    </ label="Layout rotation", help="Set the playback rotation to suit your monitor", options="none,right,flip,left", order=3 />
    layout_rotation="none";

    </ label="Default video", help="Default video to play at startup. Used if aspect ratio detection failed or disabled", order=4 />
    video_default = "intro.mp4"

    </ label="16:9 video", help="Video to play at startup when 16:9 aspect ratio is detected", order=5 />
    video_16x9 = "intro.mp4"

    </ label="4:3 video", help="Video to play at startup when 4:3 aspect ratio is detected", order=6 />
    video_4x3 = "intro_4x3.mp4"

    </ label="9:16 video", help="Video to play at startup when 9:16 aspect ratio is detected", order=7 />
    video_9x16 = "intro_9x16.mp4"

    </ label="3:4 video", help="Video to play at startup when 3:4 aspect ratio is detected", order=8 />
    video_3x4 = "intro_3x4.mp4"
}

// any signal will cause intro mode to exit
function end_mode()
{
    fe.signal("select");
}

local config = fe.get_config();

local screen_width = null;
local screen_height = null;
local layout_rotation = null;
local layout_width = null;
local layout_height = null;
local ar = null;
local layout_aspect = "default";
local vid = null;
local Aspect = [ "16x9", "4x3", "9x16", "3x4" ];

screen_width = ScreenWidth;
screen_height = ScreenHeight;

switch (config["play_intro"])
{
    case "No":
        return end_mode();
        break;
    case "Yes":
    default:
        switch (config["layout_rotation"])
        {
            case "none":
                layout_rotation = RotateScreen.None;
                layout_width = screen_width;
                layout_height = screen_height;
                break;
            case "right":
                layout_rotation = RotateScreen.Right;
                layout_width = screen_height;
                layout_height = screen_width;
                break;
            case "flip":
                layout_rotation = RotateScreen.Flip;
                layout_width = screen_width;
                layout_height = screen_height;
                break;
            case "left":
                layout_rotation = RotateScreen.Left;
                layout_width = screen_height;
                layout_height = screen_width;
                break;
        }

        fe.layout.base_rotation = layout_rotation;

        if (config["detect_aspect"] == "Yes")
        {
            local _layout_ar = layout_width / layout_height.tofloat();
            local _best_ar = 0.0;
            foreach (_ratio in Aspect)
            {
                local _x = split(_ratio, "x");
                local _try_ar = _x[0].tofloat()/_x[1].tofloat();

                if (!(_best_ar != 0.0 && fabs(_layout_ar - _best_ar) < fabs(_layout_ar - _try_ar)))
                {
                    _best_ar = _try_ar;
                    layout_aspect = _ratio;
                }
            }
        }

        vid = fe.add_image(config["video_" + layout_aspect], 0, 0, ScreenWidth, ScreenHeight);
        if (vid.file_name.len() == 0 && layout_aspect != "default")
        {
            vid.file_name = config["video_default"];
        }

        if (vid.file_name.len() == 0)
        {
            return end_mode();
        }
        fe.add_ticks_callback("intro_tick");
        break;
}

function intro_tick(ttime)
{
    // check if the video has stopped yet
    //
    if (vid.video_playing == false)
    {
        end_mode();
    }
    return false;
}
