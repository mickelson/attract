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
    video_16x9 = "intro_16x9.mp4"

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
local layout_aspect = null;
local vid_filename = "";
local vid = null;
local default_used = false;

local Aspect =
{
    RatioUnknown = "Unknown",
    Ratio16x9 = "16x9",
    Ratio16x10 = "16x10",
    Ratio4x3 = "4x3",
    Ratio5x4 = "5x4",
    Ratio9x16 = "9x16",
    Ratio10x16 = "10x16",
    Ratio3x4 = "3x4",
    Ratio4x5 = "4x5"
}

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
            ar = layout_width / layout_height.tofloat();
            switch (ar.tostring())
            {
                // widescreen (horizontal orientation)
                case "1.77865": // 16.00785:9 (eg: 1366x768)
                case "1.77778": // 16:9
                    layout_aspect = Aspect.Ratio16x9;
                    break;
                case "1.6":     // 16:10
                    layout_aspect = Aspect.Ratio16x10;
                    break;
                // standard (horizontal orientation)
                case "1.33333": // 4:3
                    layout_aspect = Aspect.Ratio4x3;
                    break;
                case "1.25":    // 5:4
                    layout_aspect = Aspect.Ratio5x4;
                    break;
                // widescreen (vertical orientation)
                case "0.5625":  // 9:16.00785 (eg: 768x1366)
                case "0.5622":  // 9:16
                    layout_aspect = Aspect.Ratio9x16;
                    break;
                case "0.625":   // 10:16
                    layout_aspect = Aspect.Ratio10x16;
                    break;
                // standard (vertical orientation)
                case "0.75":    // 3:4
                    layout_aspect = Aspect.Ratio3x4;
                    break;
                case "0.8":     // 4:5
                    layout_aspect = Aspect.Ratio4x5;
                    break;
                // unknown aspect ratio
                default:
                    layout_aspect = Aspect.RatioUnknown;
                    break;
            }
        }
        else
        {
            layout_aspect = "default";
        }

        switch (layout_aspect)
        {
            case Aspect.Ratio16x9:
            case Aspect.Ratio16x10:
                vid_filename = config["video_16x9"];
                break;
            case Aspect.Ratio4x3:
            case Aspect.Ratio5x4:
                vid_filename = config["video_4x3"];
                break;
            case Aspect.Ratio9x16:
            case Aspect.Ratio10x16:
                vid_filename = config["video_9x16"];
                break;
            case Aspect.Ratio3x4:
            case Aspect.Ratio4x5:
                vid_filename = config["video_3x4"];
                break;
            case "default":
            default:
                vid_filename = config["video_default"];
                default_used = true;
                break;
        }

        vid = fe.add_image(vid_filename, 0, 0, ScreenWidth, ScreenHeight);
        if (vid.file_name.len() == 0 && !default_used)
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
