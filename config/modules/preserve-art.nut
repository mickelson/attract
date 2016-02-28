/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - PreserveArt/PreserveImage Module
//
/////////////////////////////////////////////////////////
// PreserveArt - creates a surface with artwork that will fit or fill the specified dimensions.
// This preserves art aspect ratio, but gives option to anchor the art to one side of its dimensions.
//
// "fit" will ensure the entire artwork fits within the surface dimensions, then anchor to the specified side.
// "fill" will keep art aspect ratio, and anchor the art to the specified side.
//
// Usage:
// local img = PreserveImage( "bg.png", 0, 0, 640, 360 );
// img.set_fit_or_fill( "fill" );
// img.set_anchor( ::Anchor.Top );
//
// local art = PreserveArt( "snap", 0, 0, 320, 240 );
// art.set_fit_or_fill( "fit" );  // fit, fill or stretch
// art.set_anchor( ::Anchor.Bottom ); // Top, Left, Center, Centre, Right, Bottom
//

::Anchor <-
{
    Top = "Top",
    Left = "Left",
    Centre = "Center",
    Center = "Center",
    Right = "Right",
    Bottom = "Bottom",
}

class PreserveArt
{
    VERSION = 1.0;
    debug = false;
    surface = null;
    art = null;
    isArt = true;
    anchor = ::Anchor.Center;
    fit_or_fill = "fit";
    request_size = true;
    
    constructor( name, x, y, w, h, parent = ::fe )
    {
        surface = parent.add_surface( w, h );
        surface.x = x;
        surface.y = y;
        if ( debug ) surface.add_image( ::fe.script_dir + "pixel.png", 0, 0, surface.width, surface.height );
        art = ( isArt ) ? surface.add_artwork( name, 0, 0, w, h ) : surface.add_image( name, 0, 0, w, h );
        ::fe.add_transition_callback( this, "onTransition" );
        ::fe.add_ticks_callback( this, "onTick" );
    }
    
    function set_anchor( a )
    {
        anchor = a;
        request_size = true;
    }
    function set_fit_or_fill( f )
    {
        fit_or_fill = f;
        request_size = true;
    }
    
    function update()
    {
        if ( art.texture_width != 0 && art.texture_height != 0 )
        {
            request_size = false;
            local aspect = surface.width / surface.height.tofloat();
            local texture_aspect = art.texture_width / art.texture_height.tofloat();
            
            if ( fit_or_fill == "fit" )
            {
                if ( aspect > 1.0 )
                {
                    //wide
                    if ( texture_aspect > 1.0 )
                    {
                        print("wide, wide, fit");
                        art.width = surface.width;
                        art.height = art.width / texture_aspect;
                        if ( art.height > surface.height )
                        {
                            //reverse
                            art.height = surface.height;
                            art.width = art.height * texture_aspect;
                        }
                    } else
                    {
                        print("wide, tall, fit");
                        art.height = surface.height;
                        art.width = art.height * texture_aspect;
                        if ( art.width > surface.width )
                        {
                            //reverse
                            art.width = surface.width;
                            art.height = art.width / texture_aspect;
                        }
                    }
                } else
                {
                    //tall
                    if ( texture_aspect > 1.0 )
                    {
                        print("tall, wide, fit");
                        art.width = surface.width;
                        art.height = art.width / texture_aspect;
                        if ( art.height > surface.height )
                        {
                            //reverse
                            art.height = surface.height;
                            art.width = art.height * texture_aspect;
                        }
                    } else
                    {
                        print("tall, tall, fit");
                        art.height = surface.height;
                        art.width = art.height * texture_aspect;
                        if ( art.width > surface.width )
                        {
                            //reverse
                            art.width = surface.width;
                            art.height = art.width / texture_aspect;
                        }
                    }
                }
            } else if ( fit_or_fill == "fill" )
            {
                if ( aspect > 1.0 )
                {
                    //wide
                    if ( texture_aspect > 1.0 )
                    {
                        print("wide, wide, fill");
                        art.height = surface.height;
                        art.width = art.height * texture_aspect;
                        if ( art.width < surface.width )
                        {
                            //reverse
                            art.width = surface.width;
                            art.height = art.width / texture_aspect;
                        }
                    } else
                    {
                        print("wide, tall, fill");
                        art.width = surface.width;
                        art.height = art.width / texture_aspect;
                        if ( art.height < surface.height )
                        {
                            //reverse
                            art.height = surface.height;
                            art.width = art.height * texture_aspect;
                        }
                    }
                } else
                {
                    //tall
                    if ( texture_aspect > 1.0 )
                    {
                        print("tall, wide, fill");
                        art.width = surface.width;
                        art.height = art.width / texture_aspect;
                        if ( art.height < surface.height )
                        {
                            art.height = surface.height;
                            art.width = art.height * texture_aspect;
                        }
                    } else
                    {
                        print("tall, tall, fill");
                        art.height = surface.height;
                        art.width = art.height * texture_aspect;
                        if ( art.width < surface.width )
                        {
                            art.width = surface.width;
                            art.height = art.width / texture_aspect;
                        }
                    }
                }
            } else
            {
                //stretch
                art.preserve_aspect_ratio = false;
                art.width = surface.width;
                art.height = surface.height;            
            }
            
            switch ( anchor )
            {
                case ::Anchor.Left:
                    art.x = 0;
                    art.y = ( surface.height - art.height ) / 2;
                    break;
                case ::Anchor.Right:
                    art.x = surface.width - art.width;
                    art.y = ( surface.height - art.height ) / 2;
                    break;
                case ::Anchor.Top:
                    art.x = ( surface.width - art.width ) / 2;
                    art.y = 0;
                    break;
                case ::Anchor.Bottom:
                    art.x = ( surface.width - art.width ) / 2;
                    art.y = surface.height - art.height;
                    break;
                case ::Anchor.Center:
                default:
                    art.x = ( surface.width - art.width ) / 2;
                    art.y = ( surface.height - art.height ) / 2;
                    break;
            }
        } else
        {
            //fallback to preserve_aspect_ratio
            print("WARNING: FALLBACK TO PRESERVE_ASPECT_RATIO" );
            art.x = 0;
            art.y = 0;
            art.width = surface.width;
            art.height = surface.height;
            art.preserve_aspect_ratio = true;
        }
    }
    
    function _get( idx ) { return surface[idx]; }
    function _set( idx, val )
    {
        if ( idx == "index_offset" ) art.index_offset = val; else if ( idx == "filter_offset" ) art.filter_offset = val; else if ( idx == "file_name" ) art.file_name = val; else surface[idx] = val;
    }
    
    function print( msg )
    {
        if ( debug ) ::print("PreserveArt: " + msg + "\n" );
    }
    
    function set_rgb( r, g, b ) { surface.set_rgb( r, g, b ); }
    
    function onTransition( ttype, var, ttime )
    {
        if ( ttype == Transition.StartLayout || ttype == Transition.ToNewList || ttype == Transition.FromOldSelection ) request_size = true;
    }
    
    function onTick( ttime )
    {
        if ( request_size && art.texture_width != 0 && art.texture_height != 0 ) update();
    }
}

class PreserveImage extends PreserveArt
{
    isArt = false;
}
