/////////////////////////////////////////////////////////
//
// Attract-Mode
// http://attractmode.org/
//
// This class will create a layout from an XML file in the Attract Mode XML format.
//
// AttractMode XML is defined as follows:
// <layout width="" height="">
//     <include type="module">fade</include>
//     <image file_name="bg.png" x="0" y="0" width="0" height="0" />
//     <listbox x="0" y="0" width="0" height="0" />
//     <text msg="[Title]" x="0" y="0" width="0" height="0" />
//     <artwork file_name="snap" x="0" y="0" width="0" height="0" />
// </layout>
// 
// The <include> tag will load a module, script or plugin as defined by the 'type' attribute
// The <listbox/> tag can use any attributes defined in the Fe.ListBox class
// The <image/> tag can use any attributes defined in the Fe.Image class
// The <text/> tag can use any attributes defined in the Fe.Text class
// The <artwork/> tag can use any attributes defined in the Fe.Image (Artwork specific) class
// 
// TODO
//   layout animations (ParticleAnimation), SpriteAnimations not ready yet
//   flag variables not yet implemented (style, align, etc.)
//      flags like so?    ( fe.layout.base_rotation + fe.layout.toggle_rotation ) % 4
//   dynamic variables (like fe.list.name ) - need to update text in a transition callback instead of just translating (or have the translate function run in this callback?
//   surfaces - with objects as children
//   shaders
//   clone
//   <include> for scripts, plugins - make sure we can access layout + objects from included files
//   callbacks via XML? <transition when="Transition.ToNewSelection" .. />
/////////////////////////////////////////////////////////

class AttractModeLayout extends LayoutFile
{
    
    function file_type()
    {
        return "xml";
    }
    
    //parse content from xml, setting defaults when values don't exist
    function parse( content )
    {
        local layout = AMLayout();
        
        //add the layout attributes;
        layout.attr["width"] = ( "width" in content.attr ) ? content.attr["width"].tointeger() : 640;
        layout.attr["height"] = ( "height" in content.attr ) ? content.attr["height"].tointeger() : 480;
        
        foreach( child in content.children )
        {
            
            if ( child.tag == "include" )
            {
                local type = child.attr["type"];
                local name = child.text;
                switch( type )
                {
                    case "module":
                        layout.modules.push( name );
                        break;
                    case "script":
                        layout.scripts.push( name );
                        break;
                    case "xml":
                        //chained xml includes
                        break;
                }
            } else if ( child.tag == "text" || child.tag == "image" || child.tag == "artwork" || child.tag == "surface" || child.tag == "listbox" )
            {
                local obj = {}
                
                //properties for all supported tags
                obj.amtype <- child.tag,
                obj.x <- ( "x" in child.attr ) ? child.attr["x"].tointeger() : 0;
                obj.y <- ( "y" in child.attr ) ? child.attr["y"].tointeger() : 0;
                obj.width <- ("width" in child.attr ) ? child.attr["width"] : 0;
                obj.height <- ("height" in child.attr ) ? child.attr["height"] : 0;
                obj.visible <- ( "visible" in child.attr ) ? child.attr["visible"] : true;
                obj.rotation <- ( "rotation" in child.attr ) ? child.attr["rotation"].tointeger() : 0;
                obj.red <- ( "red" in child.attr ) ? child.attr["red"].tointeger() : 255;
                obj.green <- ( "green" in child.attr ) ? child.attr["green"].tointeger() : 255;
                obj.blue <- ( "blue" in child.attr ) ? child.attr["blue"].tointeger() : 255;
                obj.alpha <- ( "alpha" in child.attr ) ? child.attr["alpha"].tointeger() : 255;
                obj.bg_red <- ( "bg_red" in child.attr ) ? child.attr["bg_red"].tointeger() : 0;
                obj.bg_green <- ( "bg_green" in child.attr ) ? child.attr["bg_green"].tointeger() : 0;
                obj.bg_blue <- ( "bg_blue" in child.attr ) ? child.attr["bg_blue"].tointeger() : 0;
                obj.bg_alpha <- ( "bg_alpha" in child.attr ) ? child.attr["bg_alpha"].tointeger() : 0;
                obj.filter_offset <- ( "filter_offset" in child.attr ) ? child.attr["filter_offset"].tointeger() : 0;
                obj.shader <- ( "shader" in child.attr ) ? child.attr["shader"] : null;
                
                //properties for text objects
                if ( child.tag == "text" )
                {
                    obj.msg <- ( "msg" in child.attr ) ? child.attr["msg"] : "";
                    obj.word_wrap <- ( "word_wrap" in child.attr ) ? child.attr["word_wrap"] : false;
                }
                
                //properties for all image objects
                if ( child.tag == "image" || child.tag == "artwork" || child.tag == "surface" )
                {
                    obj.skew_x <- ( "skew_x" in child.attr ) ? child.attr["skew_x"].tointeger() : 0;
                    obj.skew_y <- ( "skew_y" in child.attr ) ? child.attr["skew_y"].tointeger() : 0;
                    obj.pinch_x <- ( "pinch_x" in child.attr ) ? child.attr["pinch_x"].tointeger() : 0;
                    obj.pinch_y <- ( "pinch_y" in child.attr ) ? child.attr["pinch_y"].tointeger() : 0;
                    obj.subimg_x <- ( "subimg_x" in child.attr ) ? child.attr["subimg_x"].tointeger() : 0;
                    obj.subimg_y <- ( "subimg_y" in child.attr ) ? child.attr["subimg_y"].tointeger() : 0;
                    obj.subimg_width <- ("subimg_width" in child.attr ) ? child.attr["subimg_width"].tointeger() : 0;
                    obj.subimg_height <- ("subimg_height" in child.attr ) ? child.attr["subimg_height"].tointeger() : 0;
                    obj.preserve_aspect_ratio <- ( "preserve_aspect_ratio" in child.attr ) ? child.attr["preserve_aspect_ratio"] : false;
                    obj.trigger <- ( "trigger" in child.attr ) ? child.attr["trigger"] : "Transition.ToNewSelection";
                }
               
                //properties for image and artwork objects
                if ( child.tag == "image" || child.tag == "artwork" )
                {
                    obj.file_name <- ( "file_name" in child.attr ) ? child.attr["file_name"] : "";
                    obj.video_flags <- ( "video_flags" in child.attr ) ? child.attr["video_flags"] : "";
                    obj.video_playing <- ( "video_playing" in child.attr ) ? child.attr["video_playing"] : true;
                }
                
                //properties for listbox objects
                if ( child.tag == "listbox" )
                {
                    obj.sel_red <- ( "sel_red" in child.attr ) ? child.attr["sel_red"].tointeger() : 255;
                    obj.sel_green <- ( "sel_green" in child.attr ) ? child.attr["sel_green"].tointeger() : 255;
                    obj.sel_blue <- ( "sel_blue" in child.attr ) ? child.attr["sel_blue"].tointeger() : 0;
                    obj.sel_alpha <- ( "sel_alpha" in child.attr ) ? child.attr["sel_alpha"].tointeger() : 255;
                    obj.selbg_red <- ( "selbg_red" in child.attr ) ? child.attr["selbg_red"].tointeger() : 0;
                    obj.selbg_green <- ( "selbg_green" in child.attr ) ? child.attr["selbg_green"].tointeger() : 0;
                    obj.selbg_blue <- ( "selbg_blue" in child.attr ) ? child.attr["selbg_blue"].tointeger() : 255;
                    obj.selbg_alpha <- ( "selbg_alpha" in child.attr ) ? child.attr["selbg_alpha"].tointeger() : 255;
                    obj.rows <- ( "rows" in child.attr ) ? child.attr["rows"].tointeger() : 11;
                    obj.sel_style <- ( "sel_style" in child.attr ) ? child.attr["sel_style"] : "Style.Regular";
                    obj.format_string <- ( "format_string" in child.attr ) ? child.attr["format_string"] : "";
                }
                
                //properties for text and listbox objects
                if ( child.tag == "text" || child.tag == "listbox" )
                {
                    obj.charsize <- ( "charsize" in child.attr ) ? child.attr["charsize"].tointeger() : -1;
                    obj.style <- ( "style" in child.attr ) ? child.attr["style"] : "Style.Regular";
                    obj.align <- ( "align" in child.attr ) ? child.attr["align"] : "Align.Centre";
                    obj.font <- ( "font" in child.attr ) ? child.attr["font"] : "arial";
                }
                
                //properties for text, image and artwork objects
                if ( child.tag == "text" || child.tag == "image" || child.tag == "artwork" )
                {
                    obj.index_offset <- ( "index_offset" in child.attr ) ? child.attr["index_offset"].tointeger() : 0;
                }
                
                //store animation details
                if ( child.tag == "animate" ) layout.animations.push( child.attr );
                
                obj.animations <- [];
                foreach( subchild in child.children )
                {
                    if ( subchild.tag == "animate" ) obj.animations.push( subchild.attr );
                }
                
                //add the new object to our layout objects array
                layout.objects.push( obj );
            }
            
        }

        return layout;
    }
    
    
    //use to translate string values to numbers and dynamic text strings to a fixed value or AM variable
    function translate( layout )
    {
        fe.layout.width = layout.attr["width"].tointeger();
        fe.layout.height = layout.attr["height"].tointeger();
        fe.layout.font = layout.attr["font"];
        
        local translate_map = [
            //Constants
            [ "FeVersion", FeVersion ],
            [ "FeVersionNum", FeVersionNum ],
            [ "FeConfigDirectory", FeConfigDirectory ],
            [ "ScreenWidth", ScreenWidth ],
            [ "ScreenHeight", ScreenHeight ],
            [ "ScreenSaverActive", ScreenSaverActive ],
            [ "OS", OS ],
            [ "ShadersAvailable", ShadersAvailable ],
            //fe.layout
            [ "fe.layout.width", fe.layout.width ],
            [ "fe.layout.height", fe.layout.height ],
            [ "fe.layout.font", fe.layout.font ],
            [ "fe.layout.base_rotation", fe.layout.base_rotation ],
            [ "fe.layout.toggle_rotation", fe.layout.toggle_rotation ],
            [ "fe.layout.page_size", fe.layout.page_size ],
            //fe.list
            [ "fe.list.name", fe.list.name ],
            [ "fe.list.filter_index", fe.list.filter_index ],
            [ "fe.list.index", fe.list.index  ],
            [ "fe.overlay.is_up", fe.overlay.is_up ],
            //fe.filters
            //fe.monitors
            //Transitions
            [ "Transition.StartLayout", Transition.StartLayout ],
            [ "Transition.EndLayout", Transition.EndLayout ],
            [ "Transition.ToNewSelection", Transition.ToNewSelection ],
            [ "Transition.FromOldSelection", Transition.FromOldSelection ],
            [ "Transition.ToGame", Transition.ToGame ],
            [ "Transition.FromGame", Transition.FromGame ],
            [ "Transition.ToNewList", Transition.ToNewList ],
            [ "Transition.EndNavigation", Transition.EndNavigation ],
            //fe.game_info variables (would need transition callback)
        ];
        
        foreach( object in layout.objects )
        {
            //translate object attribute values from translate_map
            foreach( attr, value in object )
            {
                if ( typeof( value ) == "string" )
                {
                    foreach( item in translate_map )
                    {
                        if ( value == item[0] ) object[attr] = item[1];
                    }
                    try
                    {
                        object[attr] = object[attr].tointeger();
                    } catch( e ) { }
                }
            }
            
            //translate object animation attribute values from translate_map
            foreach( anim in object.animations )
            {
                foreach( attr, value in anim )
                {
                    if ( typeof( value ) == "string" )
                    {
                        foreach( item in translate_map )
                        {
                            if ( value == item[0] ) anim[attr] = item[1];
                        }
                        try
                        {
                            anim[attr] = anim[attr].tointeger();
                        } catch( e ) { }
                    }
                }
            }
        }
        
        //translate layout animation attribute values from translate_map
        foreach( anim in layout.animations )
        {
            print("translating layout animation\n");
            foreach( attr, value in anim )
            {
                if ( typeof( value ) == "string" )
                {
                    foreach( item in translate_map )
                    {
                        if ( value == item[0] )
                        {
                            anim[attr] = item[1];
                            print("translated object anim value: " + value + "\n" );
                        }
                    }
                    try
                    {
                        anim[attr] = anim[attr].tointeger();
                    } catch( e ) { }
                }
            }
        }
        
        return layout;
    }
        
    //This function creates the AM objects, based on the XML
    function create( layout )
    {
        
        local newobject = null;
                
        for ( local i = 0; i < layout.objects.len(); i++ )
        {
            local object = layout.objects[i];
            switch( object.amtype )
            {
                case "text":
                    if ( DEBUG_FILEFORMAT ) print( "added text: " + object.msg + "\n" );
                    newobject = fe.add_text( object.msg, object.x, object.y, object.width, object.height );
                    newobject.visible = object.visible;
                    newobject.rotation = object.rotation;
                    newobject.set_rgb( object.red, object.green, object.blue );
                    newobject.alpha = object.alpha;
                    newobject.index_offset = object.index_offset;
                    newobject.filter_offset = object.filter_offset;
                    newobject.set_bg_rgb( object.bg_red, object.bg_green, object.bg_blue );
                    newobject.bg_alpha = object.bg_alpha;
                    newobject.charsize = object.charsize;
                    //newobject.style = object.style;
                    //newobject.align = object.align;
                    newobject.word_wrap = object.word_wrap;
                    newobject.font = object.font;
                    if ( object.shader != null) newobject.shader = object.shader;
                    break;
                case "image":
                case "artwork":
                case "surface":
                    if ( object.amtype == "image" )
                    {
                        if ( DEBUG_FILEFORMAT ) print( "added image: " + path + object.file_name + "\n" );
                        newobject = fe.add_image( path + "/" + object.file_name, object.x, object.y, object.width, object.height );
                    } else if ( object.amtype == "artwork" )
                    {
                        //artwork only
                        if ( DEBUG_FILEFORMAT ) print( "added artwork: " + object.file_name + "\n" );
                        newobject = fe.add_artwork( object.file_name, object.x, object.y, object.width, object.height );
                        newobject.index_offset = object.index_offset;
                        newobject.filter_offset = object.filter_offset;
                    } else
                    {
                        if ( DEBUG_FILEFORMAT ) print( "added surface: " + object.width + "x" + object.height + "\n" );
                        newobject = fe.add_surface(object.x, object.y, object.width, object.height);
                    }
                    newobject.visible = object.visible;
                    newobject.rotation = object.rotation;
                    newobject.set_rgb( object.red, object.green, object.blue );
                    newobject.alpha = object.alpha;
                    if ( object.shader != null) newobject.shader = object.shader;
                    newobject.skew_x = object.skew_x;
                    newobject.skew_y = object.skew_y;
                    newobject.pinch_x = object.pinch_x;
                    newobject.pinch_y = object.pinch_y;
                    newobject.subimg_x = object.subimg_x;
                    newobject.subimg_y = object.subimg_y;
                    if ( object.subimg_width != 0 ) newobject.subimg_width = object.subimg_width;
                    if ( object.subimg_height != 0 ) newobject.subimg_height = object.subimg_height;
                    //newobject.video_flags = object.video_flags;
                    if ( object.amtype != "surface" ) newobject.video_playing = object.video_playing;
                    newobject.preserve_aspect_ratio = object.preserve_aspect_ratio;
                    break;
                case "listbox":
                    if ( DEBUG_FILEFORMAT ) print( "added listbox\n" );
                    newobject = fe.add_listbox( object.x, object.y, object.width, object.height );
                    newobject.visible = object.visible;
                    newobject.rotation = object.rotation;
                    newobject.set_rgb( object.red, object.green, object.blue );
                    newobject.alpha = object.alpha;
                    newobject.filter_offset = object.filter_offset;
                    newobject.set_bg_rgb( object.bg_red, object.bg_green, object.bg_blue );
                    newobject.set_sel_rgb( object.sel_red, object.sel_green, object.sel_blue );
                    newobject.set_selbg_rgb( object.selbg_red, object.selbg_green, object.selbg_blue );
                    newobject.bg_alpha = object.bg_alpha;
                    newobject.sel_alpha = object.sel_alpha;
                    newobject.selbg_alpha = object.selbg_alpha;
                    newobject.charsize = object.charsize;
                    //newobject.style = object.style;
                    //newobject.align = object.align;
                    newobject.font = object.font;
                    newobject.rows = object.rows;
                    //newobject.sel_style = object.sel_style;
                    newobject.format_string = object.format_string;
                    if ( object.shader != null) newobject.shader = object.shader;
                    break;
                default:
                    if ( DEBUG_FILEFORMAT ) print( "unsupported object: " + object.amtype + "\n" );
            }
            
            //add object animations
            foreach( config in object.animations )
            {
                print( "added animation to " + object.amtype + " object\n" );
                animate( config, newobject );
            }
            
        }
        
        //add generic layout animations
        foreach( config in layout.animations )
        {
            animate( config );
        }
        
    }
}
