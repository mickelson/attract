/*

Attract-Mode Frontend - Pan and Scan module
-------------------------------------------

Extends the PreserveArt module by liquid8d. Tested with version 1.0 of PreserveArt.

Displays an art or image object, and animates it by scrolling and zooming.

More information at Attract-Mode forums:
http://forum.attractmode.org/index.php?topic=624.0

Example usage:

fe.load_module("pan-and-scan");

local my_art = PanAndScanArt("flyer", 0, 0, 640,480);

my_art.set_fit_or_fill("fill");
my_art.set_anchor(::Anchor.Center);
my_art.set_zoom(4.5, 0.00008);
my_art.set_animate(::AnimateType.Bounce, 0.07, 0.07)
my_art.set_randomize_on_transition(true);
my_art.set_start_scale(1.15);


CHANGELOG
    Version 0.1     (8th December 2015)     - Initial release (included in Blastcity layout)
    Version 1.0     (14th March 2016)       - Complete rewrite to extend PreserveArt module
    Version 1.1     (14th March 2016)       - Added start_scale and randomize_on_transition
    Version 1.2     (22th March 2016)       - Fixed bug with bounce animation when art is smaller than surface

*/

fe.load_module("preserve-art");

::AnimateType <-
{
    None = "None",
    Bounce = "Bounce",
    HorizBounce = "HorizBounce",
    VertBounce = "VertBounce"
}

::MoveDirection <-
{
    None = "None",
    Left = "Left",
    Right = "Right",
    Up = "Up",
    Down = "Down"
}

class PanAndScanArt extends PreserveArt
{
    VERSION = 1.2;
    PRESERVEART_VERSION = 1.0;

    debug = false;

    animate_type = ::AnimateType.None;
    move_speed_x = 0.5;
    move_speed_y = 0.5;

    zoom = false;
    zoom_speed = 0.0;
    zoom_scale_to = 1.0;

    start_scale = 1.0;
    randomize_on_transition = false;

    _move_direction_x = ::MoveDirection.None;
    _move_direction_y = ::MoveDirection.None;
    _current_scale = 0.0;
    _art_width = 0;
    _art_height = 0;
    _prev_art_width = 0;
    _prev_art_height = 0;

    constructor(name, x, y, w, h, parent = ::fe)
    {
        if (base.VERSION != PRESERVEART_VERSION)
        {
            ::print("\n***\n*** WARNING: PreserveArt module version mismatch.\n*** PreserveArt version installed: " + base.VERSION + "\n*** PanAndScan expecting version: " + PRESERVEART_VERSION + "\n***\n\n")
        }
        base.constructor(name, x, y, w, h, parent = ::fe);

    }

    function set_anchor(a)
    {
        base.set_anchor(a);
        print("set_anchor: " + a)
    }

    function set_start_scale(scale)
    {
        start_scale = scale;
        print("set_start_scale: " + start_scale)
    }

    function set_randomize_on_transition(b)
    {
        randomize_on_transition = b;
        print("set_randomize_on_transition: " + b)
    }

    function set_fit_or_fill(f)
    {
        base.set_fit_or_fill(f);
        print("set_fit_or_fill: " + f)
    }

    function set_zoom(scale, speed)
    {
        zoom = true;
        zoom_scale_to = scale;
        zoom_speed = speed;
        print("set_zoom: " + zoom_scale_to + ", " + zoom_speed)
    }

    function set_animate(animtype, speed1, speed2 = 0)
    {
        animate_type = animtype;
        switch (animate_type)
        {
            case ::AnimateType.HorizBounce:
                _move_direction_x = ::MoveDirection.Right;
                move_speed_x = speed1;
                break;
            case ::AnimateType.VertBounce:
                _move_direction_y = ::MoveDirection.Down;
                move_speed_y = speed1;
                break;
            case ::AnimateType.Bounce:
                _move_direction_x = ::MoveDirection.Right;
                _move_direction_y = ::MoveDirection.Down;
                move_speed_x = speed1;
                move_speed_y = speed2;
                break;
        }
        print("set_animate: " + animate_type + ", " + move_speed_x + ", " + move_speed_y)
    }

    function update()
    {
        base.update();

            _prev_art_width = art.width;
            _prev_art_height = art.height;

            art.width = art.width * start_scale;
            art.height = art.height * start_scale;

            art.x -= (art.width - _prev_art_width) / 2
            art.y -= (art.height - _prev_art_height) / 2

        _art_width = art.width;
        _art_height = art.height;
    }

    function update_panandscan()
    {
        if (!request_size)
        {
            if (zoom)
            {
                if (_current_scale < zoom_scale_to)
                {
                    _prev_art_width = art.width;
                    _prev_art_height = art.height;

                    art.width = _art_width * _current_scale;
                    art.height = _art_height * _current_scale;
                    _current_scale += zoom_speed;

                    switch (anchor)
                    {
                        case ::Anchor.Left:
                            art.x = 0;
                            art.y -= (art.height - _prev_art_height) / 2
                            break;
                        case ::Anchor.Right:
                            art.x = surface.width - art.width;
                            art.y = ( surface.height - art.height ) / 2;
                            break;
                        case ::Anchor.Top:
                            art.x -= (art.width - _prev_art_width) / 2
                            art.y = 0;
                            break;
                        case ::Anchor.Bottom:
                            art.x = ( surface.width - art.width ) / 2;
                            art.y = surface.height - art.height;
                            break;
                        case ::Anchor.Center:
                        default:
                            art.x -= (art.width - _prev_art_width) / 2
                            art.y -= (art.height - _prev_art_height) / 2
                            break;
                    }
                }
            }

            if (art.width > surface.width)
            {
                if (animate_type == ::AnimateType.HorizBounce || animate_type == ::AnimateType.Bounce)
                {
                    if (art.x <= surface.width - art.width)
                    {
                        _move_direction_x = ::MoveDirection.Left;
                    }
                    if (art.x >= 0)
                    {
                        _move_direction_x = ::MoveDirection.Right;
                    }

                    if (_move_direction_x == ::MoveDirection.Right)
                    {
                        art.x -= move_speed_x;
                    }
                    else if (_move_direction_x == ::MoveDirection.Left)
                    {
                        art.x += move_speed_x;
                    }
                }
            }
            else
            {
                if (animate_type == ::AnimateType.HorizBounce || animate_type == ::AnimateType.Bounce)
                {
                    if (art.x >= surface.width - art.width)
                    {
                        _move_direction_x = ::MoveDirection.Left;
                    }
                    if (art.x <= 0)
                    {
                        _move_direction_x = ::MoveDirection.Right;
                    }

                    if (_move_direction_x == ::MoveDirection.Right)
                    {
                        art.x += move_speed_x;
                    }
                    else if (_move_direction_x == ::MoveDirection.Left)
                    {
                        art.x -= move_speed_x;
                    }
                }
            }
            if (art.height > surface.height)
            {
                if (animate_type == ::AnimateType.VertBounce || animate_type == ::AnimateType.Bounce)
                {
                    if (art.y <= surface.height - art.height)
                    {
                        _move_direction_y = ::MoveDirection.Up;
                    }
                    if (art.y >= 0)
                    {
                        _move_direction_y = ::MoveDirection.Down;
                    }

                    if (_move_direction_y == ::MoveDirection.Down)
                    {
                        art.y -= move_speed_y;
                    }
                    else if (_move_direction_y == ::MoveDirection.Up)
                    {
                        art.y += move_speed_y;
                    }
                }
            }
            else
            {
                if (animate_type == ::AnimateType.VertBounce || animate_type == ::AnimateType.Bounce)
                {
                    if (art.y >= surface.height - art.height)
                    {
                        _move_direction_y = ::MoveDirection.Up;
                    }
                    if (art.y <= 0)
                    {
                        _move_direction_y = ::MoveDirection.Down;
                    }

                    if (_move_direction_y == ::MoveDirection.Down)
                    {
                        art.y += move_speed_y;
                    }
                    else if (_move_direction_y == ::MoveDirection.Up)
                    {
                        art.y -= move_speed_y;
                    }
                }
            }
        }
    }

    function print(msg)
    {
        if (debug) ::print("PanAndScanArt: " + msg + "\n");
    }

    function onTransition(ttype, var, ttime)
    {
        base.onTransition(ttype, var, ttime);

        if (request_size)
        {
            _current_scale = 1.0;
            if (randomize_on_transition)
            {
                local rnd_x = ::rand() * 2 / ::RAND_MAX;
                local rnd_y = ::rand() * 2 / ::RAND_MAX;

                (rnd_x) ? rnd_x = ::MoveDirection.Left : rnd_x = ::MoveDirection.Right;
                (rnd_y) ? rnd_y = ::MoveDirection.Up : rnd_y = ::MoveDirection.Down;

                switch (animate_type)
                {
                    case ::AnimateType.HorizBounce:
                        _move_direction_x = rnd_x;
                        break;
                    case ::AnimateType.VertBounce:
                        _move_direction_y = rnd_y;
                        break;
                    case ::AnimateType.Bounce:
                        _move_direction_x = rnd_x;
                        _move_direction_y = rnd_y;
                        break;
                }
                print("randomize: x(" + rnd_x + ") y(" + rnd_y + ")");
            }
        }
        else
        {
        }

        update_panandscan();

    }

    function onTick(ttime)
    {
        base.onTick(ttime);
        update_panandscan();
    }
}

class PanAndScanImage extends PanAndScanArt
{
    isArt = false;

    function print( msg )
    {
        if ( debug ) ::print("PanAndScanImage: " + msg + "\n" );
    }
}
