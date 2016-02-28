//////////////////////////////////////
//  ScrollingText: Scrolling Text module
//  author: liquid8d
//
//  Description:
//  Creates a ScrollingText object class, which can create scrolling text. It creates both a surface object (the scroller background)
//  and text object (the scrolling text).
//
//  Can scrolling horizontal or vertical - either looping or a set number of times
//
//  Requirements:
//  the modules/objects/scroller directory along with this module
//
//  Example:
//      local scroller = ScrollingText.add( "[Title]", 0, 0, 500, 30 );
//      scroller.set_bg_rgb( 255, 255, 255 );   //wrapper function to set scroller background
//      scroller.set_rgb( 200, 0, 0);   //wrapper function to set text color
//      //some settings can be modified
//      scroller.settings.delay = 250;
//      scroller.settings.loop = 5;   //fixed loop count
//
//  Returns:
//  A scroll object, which contains the surface the text is scrolling on, the text object, and
//  current scroll values
//
//  ScrollType
//      HORIZONTAL_LEFT
//      HORIZONTAL_RIGHT
//      HORIZONTAL_BOUNCE
//      VERTICAL_UP
//      VERTICAL_DOWN
//      VERTICAL_BOUNCE
//
//  Settings:
//      loop = -1
//      delay = 0
//      speed_x = 1.0
//      speed_y = 0.2
//      fixed_width = 0
//
//  Notes:
//
//  Not all game info magic tokens will work correctly.
//  You may need to set a fixed_width in settings to get scrolling text to be the correct length for looping
//////////////////////////////////////

//available scroll types
class ScrollType {
    HORIZONTAL_LEFT = 0;
    HORIZONTAL_RIGHT = 1;
    HORIZONTAL_BOUNCE = 2;
    VERTICAL_UP = 3;
    VERTICAL_DOWN = 4;
    VERTICAL_BOUNCE = 5;
}

ScrollingText <- {
    VERSION = 1.0,
    objs = [],
    debug = false,
    debug_txt = null,
    init = false,
    initialize = function()
    {
        //initialize scrolling text
        fe.add_ticks_callback( ScrollingText, "tick_callback" );
        fe.add_transition_callback( ScrollingText, "transition_callback" );
        
        if ( ScrollingText.debug )
        {
            //setup debug text
            ScrollingText.debug_txt = fe.add_text("", 0, fe.layout.height - 200, fe.layout.width, 190 );
            ScrollingText.debug_txt.set_rgb( 255, 255, 0 );
            ScrollingText.debug_txt.align = Align.Left;
            ScrollingText.debug_txt.charsize = 20;
            ScrollingText.debug_txt.word_wrap = true;
            if ( ScrollingText.debug ) print("ScrollingText: initialized\n");
        }
        
        ScrollingText.init = true;
    }
    transition_callback = function( ttype, var, ttime ) {
        //handle transitions - i.e. resetting the scrolling text values
        switch ( ttype )
        {
            case Transition.FromOldSelection:
                break;
            case Transition.ToNewList:
                break;
            case Transition.StartLayout:
            case Transition.ToNewSelection:
                for ( local i = 0; i < ScrollingText.objs.len(); i++ )
                {
                    local obj = ScrollingText.objs[i];
                    
                    //resize text object width based on estimated font width (charsize if none provided)
                    obj._text = ScrollingText.actual_text(obj, var);
                    if ( ScrollingText.debug ) print("actual text: " + obj._text + " (" + obj._text.len() + ")\n");
                    obj.text.width = ( obj.settings.fixed_width != 0 ) ? obj.text.width = obj.settings.fixed_width : ScrollingText.measure_width(obj._text, obj.text.charsize);
                    
                    //reset scroll count
                    obj._count = 0;
                    
                    //set scroll start pos
                    switch ( obj._type )
                    {
                        case ScrollType.HORIZONTAL_BOUNCE:
                            obj.text.align = Align.Left;
                            obj._dir = "left";
                            break;
                        case ScrollType.HORIZONTAL_LEFT:
                            obj.text.align = Align.Left;
                            obj.text.x = obj.surface.width;
                            obj._dir = "left";
                            break;
                        case ScrollType.HORIZONTAL_RIGHT:
                            obj.text.align = Align.Right;
                            obj.text.x = -obj.surface.width;
                            obj._dir = "right";
                            break;
                        case ScrollType.VERTICAL_BOUNCE:
                            obj.text.align = Align.Left;
                            obj._dir = "up";
                            break;
                        case ScrollType.VERTICAL_UP:
                            obj.text.align = Align.Left;
                            obj.text.y = obj.surface.height;
                            obj._dir = "up";
                            break;
                        case ScrollType.VERTICAL_DOWN:
                            obj.text.align = Align.Left;
                            obj.text.y = -obj.surface.height;
                            obj._dir = "down";
                            break;
                    }
                }
                break;
        }
    },
    tick_callback = function( ttime ) {
        //we scroll when needed on each tick
        if ( ScrollingText.debug ) ScrollingText.debug_txt.msg = "ScrollingText: tick=" + ttime;
        
        //handle scrolling for any active scroll text objects
        local obj_debug_info = "";
        for ( local i = 0; i < ScrollingText.objs.len(); i++ )
        {
            local obj = ScrollingText.objs[i];
            //wait for length of scroll object delay to start
            if ( ttime > obj.settings.delay )
            {
                //scroll if loop is infinite or if scroll count is less than the request loop count
                if ( obj.settings.loop < 0 || obj._count < obj.settings.loop )
                {
                    //do scrolling
                    ScrollingText.scroll( obj );
                }
            }
            if ( ScrollingText.debug ) obj_debug_info += "\nobj" + i + ": " + "\tt=" + obj.text.msg + "\ttl:" + obj._text.len() + "\tc=" + obj._count + "\tx=" + format("%.2f", obj.text.x) + "\ty=" + format("%.2f", obj.text.y) + "\tw=" + format("%.2f", obj.text.width) + "\th=" + format("%.2f", obj.text.height) + "\td:" + obj._dir;
        }
        if ( ScrollingText.debug ) ScrollingText.debug_txt.msg += obj_debug_info + "\n";
    },
    scroll = function( obj )
    {
        //scroll text object a certain direction
        switch ( obj._dir )
        {
            case "up":
                if ( obj.text.y > -obj.text.height )
                {
                    //scroll
                    obj.text.y -= obj.settings.speed_y;
                } else
                {
                    //loop
                    obj._count += 1;
                    if ( obj._type == ScrollType.VERTICAL_BOUNCE )
                    {
                        obj._dir = ( obj._dir == "up" ) ? "down" : "up";
                    } else
                    {
                        obj._dir = "up";
                        obj.text.y = obj.surface.height;
                    }
                }
                break;
            case "down":
                if ( obj.text.y < obj.text.height )
                {
                    //scroll
                    obj.text.y += obj.settings.speed_y;
                } else
                {
                    //loop
                    obj._count += 1;
                    if ( obj._type == ScrollType.VERTICAL_BOUNCE )
                    {
                        obj._dir = ( obj._dir == "up" ) ? "down" : "up";
                    } else
                    {
                        obj._dir = "down";
                        obj.text.y = -obj.text.height;
                    }
                }
                break;
            case "left":
                //horizontal scroll
                if ( obj.text.x > -obj.text.width )
                {
                    //scroll
                    obj.text.x -= obj.settings.speed_x;
                } else
                {
                    //loop
                    obj._count += 1;
                    if ( obj._type == ScrollType.HORIZONTAL_BOUNCE )
                    {
                        obj._dir = ( obj._dir == "left" ) ? "right" : "left";
                    } else
                    {
                        obj._dir = "left";
                        obj.text.x = obj.surface.width;
                    }
                }
                break;
            case "right":
                if ( obj.text.x < obj.surface.width )
                {
                    //scroll
                    obj.text.x += obj.settings.speed_x;
                } else
                {
                    //loop
                    obj._count += 1;
                    if ( obj._type == ScrollType.HORIZONTAL_BOUNCE )
                    {
                        obj._dir = ( obj._dir == "left" ) ? "right" : "left";
                    } else
                    {
                        obj._dir = "right";
                        obj.text.x = -obj.text.width;
                    }
                }
                break;
        }
    },
    actual_text = function(obj, var) {
        //converts magic tokens in the objects text msg value to the actual text value, so we can find the actual length of it
        //this is needed because the text object msg value only gives us pre-tokenized text in transition callback
        local actual = ScrollingText.replace( obj.text.msg, "\\[Title\\]", fe.game_info( Info.Title, var ) );
        actual = ScrollingText.replace( actual, "\\[Emulator\\]", fe.game_info( Info.Emulator, var ) );
        actual = ScrollingText.replace( actual, "\\[Category\\]", fe.game_info( Info.Category, var ) );
        actual = ScrollingText.replace( actual, "\\[Year\\]", fe.game_info( Info.Year, var ) );
        local playTime = fe.game_info( Info.PlayedTime, var );
        playTime = ( playTime.tointeger() / 60.0 );
        actual = ScrollingText.replace( actual, "\\[PlayedTime\\]",  format("%.1f", playTime) + " Minutes" );
        actual = ScrollingText.replace( actual, "\\[PlayedCount\\]", fe.game_info( Info.PlayedCount, var ) );
        actual = ScrollingText.replace( actual, "\\[Name\\]", fe.game_info( Info.Name, var ) );
        actual = ScrollingText.replace( actual, "\\[CloneOf\\]", fe.game_info( Info.CloneOf, var ) );
        actual = ScrollingText.replace( actual, "\\[Manufacturer\\]", fe.game_info( Info.Manufacturer, var ) );
        actual = ScrollingText.replace( actual, "\\[Players\\]", fe.game_info( Info.Players, var ) );
        actual = ScrollingText.replace( actual, "\\[Rotation\\]", fe.game_info( Info.Rotation, var ) );
        actual = ScrollingText.replace( actual, "\\[Control\\]", fe.game_info( Info.Control, var ) );
        actual = ScrollingText.replace( actual, "\\[Status\\]", fe.game_info( Info.Status, var ) );
        actual = ScrollingText.replace( actual, "\\[DisplayCount\\]", fe.game_info( Info.DisplayCount, var ) );
        actual = ScrollingText.replace( actual, "\\[DisplayType\\]", fe.game_info( Info.DisplayType, var ) );
        actual = ScrollingText.replace( actual, "\\[AltRomname\\]", fe.game_info( Info.AltRomname, var ) );
        actual = ScrollingText.replace( actual, "\\[AltTitle\\]", fe.game_info( Info.AltTitle, var ) );
        actual = ScrollingText.replace( actual, "\\[DisplayName\\]", fe.list.name );
        local current_filter = fe.filters[fe.list.filter_index];
        actual = ScrollingText.replace( actual, "\\[FilterName\\]", current_filter.name );
        //are these available?
        //actual = ScrollingText.replace( actual, "\\[ListSize\\]", fe.game_info( Info.ListSize, var ) );
        //actual = ScrollingText.replace( actual, "\\[ListEntry\\]", fe.game_info( Info.ListEntry, var ) );
        //actual = ScrollingText.replace( actual, "\\[SortName\\]", fe.game_info( Info.SortName, var ) );
        //actual = ScrollingText.replace( actual, "\\[SortValue\\]", fe.game_info( Info.SortValue, var ) );
        return actual;
    },
    replace = function(string, original, replacement)
    {
      //replace text in a string
      local expression = regexp(original);
      local result = "";
      local position = 0;
      local captures = expression.capture(string);
      while (captures != null)
      {
        foreach (i, capture in captures)
        {
          result += string.slice(position, capture.begin);
          result += replacement;
          position = capture.end;
        }
        captures = expression.capture(string, position);
      }
      result += string.slice(position);
      return result;
    },
    measure_width = function(text, font_width) {
        //estimate the width of the actual text content
        local length = text.len() * font_width.tofloat();
        if ( ScrollingText.debug ) print("measure_width: " + text.len() + ", " + font_width + ": " + length + "\n");
        return length;
    },
    add = function( text, x, y, w, h, scroll_type = ScrollType.HORIZONTAL_LEFT ) {
        //if not initialized yet, initialize it
        if ( !ScrollingText.init ) ScrollingText.initialize();
        
        //create a surface (the scrolling area) that the text object will be on
        local surface_obj = fe.add_surface( w, h );
            surface_obj.x = x;
            surface_obj.y = y;
        
        //create a background for our surface
        local bg_obj = surface_obj.add_image( fe.module_dir  + "scrollingtext/pixel.png", 0, 0, surface_obj.width, surface_obj.height );
        bg_obj.alpha = 0;
        if ( ScrollingText.debug )
        {
            bg_obj.alpha = 255;
            bg_obj.set_rgb(200, 200, 0);
        }
        
        //create a text object that will be manipulated in ticks callback
        local text_obj = surface_obj.add_text( text, 0, 0, surface_obj.width, surface_obj.height );
        if ( ScrollingText.debug )
        {
            text_obj.set_rgb( 0, 0, 0 );
            text_obj.set_bg_rgb( 0, 100, 100);
        }
        
        //create a scrolling text object
        local scroll_obj = {
            surface = surface_obj,
            surface_bg = bg_obj,
            text = text_obj,
            _text = text,
            _type = scroll_type,
            _count = 0,
            _dir = "left",
            set_bg_rgb = function( r, g, b, alpha = 255 ) {
                //wrapper function to set surface bg color / alpha
                this.surface_bg.alpha = alpha;
                this.surface_bg.set_rgb( r, g, b );
            },
            set_rgb = function( r, g, b, alpha = 255 ) {
                //wrapper function to set text color / alpha
                this.text.alpha = alpha;
                this.text.set_rgb( r, g, b );
            },
            set_pos = function( x, y, w = 0, h = 0 ) {
                w = ( w == 0 ) ? this.surface.width : w;
                h = ( h == 0 ) ? this.surface.height : h;
                this.surface.set_pos( x, y, w, h );
            },
            settings = {
                loop = -1,
                delay = 0,
                speed_x = 1.0,
                speed_y = 0.2,
                fixed_width = 0,
                transition_reset = true,
            }
        }
        ScrollingText.objs.push( scroll_obj );
        return scroll_obj;
    }
}
