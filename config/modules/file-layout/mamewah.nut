/////////////////////////////////////////////////////////
//
// Mamewah
// http://mamewah.mameworld.info/
//
// Layout Docs: http://wiki.arcadecontrols.com/wiki/Mamewah_Skins
// 
// .lay TXT
//
// TODO
// 
// Layout Compatibility
// background file_name path removal
// some things don't align just right - may not be much i can do about that using a standard listbox
// listbox text overlaps
// listbox rows - does mamewah measure font height to determine rows?
// font issues
// fade?
// translate missing art to AM missing art?
// z-order?
// horizontal-vertical?
// 
// Other Compatibility
// config for mapping artwork # to artwork
// options/message/screensaver forms?
// intro-exit movies
// sound effects
//
// Not Supported
// .wmv?
// 
/////////////////////////////////////////////////////////

class MamewahLayout extends LayoutFile
{
        
    function file_type()
    {
        return "txt";
    }
    
    function parse( content )
    {
        local layout = AMLayout();
        
        local form_map = {
            main = 0,
            //options = 293,
            //message = 352,
            //screensaver = 398
        }

        //map the mamewah layout fixed line numbers 
        local main_display_object_map = {
            "mainlogo": 7,
            "gamelistindicator": 20,
            "emulator_name": 33,
            "game_list": 46,
            "game_selected": 59,
            "artwork1": 72,
            "artwork2": 85,
            "artwork3": 98,
            "artwork4": 111,
            "artwork5": 124,
            "artwork6": 137,
            "artwork7": 150,
            "artwork8": 163,
            "artwork9": 176,
            "artwork10": 189,
            "game_description": 202,
            "romname": 215,
            "year_manufacturer": 228,
            "screen_type": 241,
            "controller_type": 254,
            "driver_status": 267,
            "catver_category": 280,
        }
        
        local options_display_object_map = {
            "options_heading": 300,
            "options_list": 313,
            "current_setting_heading": 326,
            "current_setting_value": 339
        }
        
        local message_display_object_map = {
            "heading": 359,
            "message": 372,
            "prompt": 385,
        }
        
        local screensaver_display_object_map = {
            "artwork1": 405,
            "artwork2": 418,
            "artwork3": 431,
            "artwork4": 444,
            "artwork5": 457,
            "artwork6": 470,
            "artwork7": 483,
            "artwork8": 496,
            "artwork9": 509,
            "artwork10": 522,
            "message": 535,
            "game_description": 548,
            "mp3_name": 561
        }
        
        local obj = {}
        foreach ( name, val in form_map )
        {
            local start = val;
            local attr_1 = split( content.lines[start], ";" );
            local attr_2 = split( content.lines[start + 1], ";" );
            local attr_3 = split( content.lines[start + 2], ";" );
            
            layout.attr["width"] = ( attr_1.len() > 0 ) ? attr_1[0].tointeger() : 640;
            layout.attr["height"] = ( attr_2.len() > 0 ) ? attr_2[0].tointeger() : 480;
            
            //mamewah specific settings we may/may not need
            layout.attr["mamewah"] <- {
                colordepth = ( attr_1.len() > 1 ) ? attr_1[1] : 0,
                refresh = ( attr_2.len() > 2 ) ? attr_2[2] : 0,
                fadespeed = ( attr_2.len() > 1 ) ? attr_2[1] : 0,
                color = intToRGB(lstrip(content.lines[start + 2]).tointeger()),
                filename = content.lines[start + 3],
                transparent = ( lstrip(content.lines[start + 4]).tointeger() == 0 ) ? true : false,
                selected_bg_color = intToRGB(lstrip(content.lines[start + 5]).tointeger()),
                selected_color = intToRGB(lstrip(content.lines[start + 6]).tointeger())
            }
            
            foreach ( name, val in main_display_object_map )
            {
                local start = val;
                local amtype = "unknown";
                switch ( name )
                {
                    case "gamelistindicator":
                    case "emulator_name":
                    case "game_selected":
                    case "game_description":
                    case "romname":
                    case "year_manufacturer":
                    case "screen_type":
                    case "controller_type":
                    case "driver_status":
                    case "catver_category":
                        //text
                        amtype = "text";
                        break;
                    case "mainlogo":
                        //image
                        amtype = "image";
                        break;
                    case "game_list":
                        //listbox
                        amtype = "listbox";
                        break;
                }
                
                local obj = {
                    amtype = amtype,
                    enabled = ( content.lines[start] == "True" ) ? true : false,
                    transparent = ( lstrip(content.lines[start + 1]).tointeger() == 0 ) ? true : false,
                    color = intToRGB(lstrip(content.lines[start + 2]).tointeger()),
                    font_color = intToRGB(lstrip(content.lines[start + 3]).tointeger()),
                    font = content.lines[start + 4],
                    font_bold = ( content.lines[start + 5] == "True" ) ? true : false,
                    font_italic = ( content.lines[start + 6] == "True" ) ? true : false,
                    font_size = content.lines[start + 7].tointeger(),
                    text_align = content.lines[start + 8].tointeger(),
                    x = content.lines[start + 9].tointeger(),
                    y = content.lines[start + 10].tointeger(),
                    width = content.lines[start + 11],
                    height = content.lines[start + 12]
                }
                
                if ( amtype == "text" ) obj.msg <- name;
                if ( name.slice(0, 7) == "artwork" )
                {
                    obj.amtype = "artwork";
                    obj.file_name <- name;
                }
                
                layout.objects.push( obj );
            }
            
            /*
            // POSSIBLY ADD OPTIONS-MESSAGE-SCREENSAVER STUFF LATER?
            foreach ( name, val in options_display_object_map )
            foreach ( name, val in message_display_object_map )
            foreach ( name, val in screensaver_display_object_map )
            {
                local start = val;
                values.options.display_objects[name] <- {
                    enabled = ( layout.lines[start] == "True" ) ? true : false,
                    transparent = ( layout.lines[start + 1].tointeger() == 0 ) ? true : false,
                    color = intToRGB(layout.lines[start + 2].tointeger()),
                    font_color = intToRGB(layout.lines[start + 3].tointeger()),
                    font = layout.lines[start + 4],
                    font_bold = ( layout.lines[start + 5] == "True" ) ? true : false,
                    font_italic = ( layout.lines[start + 6] == "True" ) ? true : false,
                    font_size = layout.lines[start + 7].tointeger(),
                    text_align = layout.lines[start + 8].tointeger(),
                    x = layout.lines[start + 9],
                    y = layout.lines[start + 10],
                    width = layout.lines[start + 11],
                    height = layout.lines[start + 12]
                }
            }
            */            
        }
        return layout;
    }
    
    function translate( layout )
    {
        fe.layout.width = layout.attr["width"];
        fe.layout.height = layout.attr["height"];
        local textmap = {
            "gamelistindicator": "[DisplayName] [FilterName]",
            "emulator_name": "[Emulator]",
            "game_selected": "Game [ListEntry] of [ListSize] selected",
            "game_description": "[Title]",
            "romname": "[Name] (Clone of [CloneOf])",
            "year_manufacturer": "[Year] [Manufacturer]",
            "screen_type": "[Rotation] [DisplayType]",
            "controller_type": "[Control]",
            "driver_status": "Status [Status]",
            "catver_category": "[Category]"
        }
        local imagemap = {
            "artwork1": "snap",
            "artwork2": "marquee",
            "artwork3": "box",
            "artwork4": "",
            "artwork5": "",
            "artwork6": "",
            "artwork7": "",
            "artwork8": "",
            "artwork9": "",
            "artwork10": "",
        }
        foreach( object in layout.objects )
        {
            switch( object.amtype )
            {
                case "text":
                    object.msg = textmap[object.msg];
                    break;
                case "artwork":
                    object.file_name = imagemap[object.file_name];
                    break;
            }
            foreach ( attr, value in object )
            {
                if ( attr == "width" || attr == "height" )
                {
                    value = replace( value, "fe.layout.width", fe.layout.width );
                    value = replace( value, "fe.layout.height", fe.layout.height );
                    value = value.tointeger();
                    object[attr] = value;
                }
            }
        }
        return layout;
    }
    
    function create( layout )
    {

        local newobject = null;
        
        //background
        //background color
        newobject = LAYERS[0].add_image( FeConfigDirectory  + "/modules/file-layout/pixel.png", 0, 0, fe.layout.width, fe.layout.height );
        newobject.set_rgb( layout.attr.mamewah.color.r, layout.attr.mamewah.color.g, layout.attr.mamewah.color.b );
        if ( layout.attr.mamewah.filename != "" )
        {
            //image
            newobject = LAYERS[0].add_image( find_file(layout.attr.mamewah.filename), 0, 0, fe.layout.width, fe.layout.height );
        }
        
        
        for ( local i = 0; i < layout.objects.len(); i++ )
        {
            local object = layout.objects[i];
            if ( object.enabled )
            {
                switch( object.amtype )
                {
                    case "text":
                        if ( DEBUG_FILEFORMAT ) print( "added text: " + object.msg + "\n" );
                        newobject = fe.add_text( object.msg, object.x, object.y, object.width, object.height );
                        newobject.set_rgb( object.font_color.r, object.font_color.g, object.font_color.b );
                        if ( !object.transparent ) newobject.set_bg_rgb( object.color.r, object.color.g, object.color.b );
                        newobject.font = ( object.font == "System" ) ? "" : object.font;
                        newobject.style = ( object.font_bold ) ? Style.Bold : Style.Regular;
                        newobject.charsize = object.font_size;
                        if ( object.text_align == 0 )
                        {
                            newobject.align = Align.Left;
                        } else if ( object.text_align == 1 )
                        {
                            newobject.align = Align.Right;
                        } else if ( object.text_align == 2 )
                        {
                            newobject.align = Align.Centre;
                        }
                        break;
                    case "image":
                        //logo
                        if ( DEBUG_FILEFORMAT ) print( "added image: logo.gif\n" );
                        newobject = LAYERS[1].add_image( path + "logo.gif", object.x, object.y, 0, 0 );
                        newobject.preserve_aspect_ratio = true;
                        break;
                    case "artwork":
                        if ( DEBUG_FILEFORMAT ) print( "added artwork: " + object.file_name + "\n" );
                        newobject = LAYERS[2].add_artwork( object.file_name, object.x, object.y, object.width, object.height );
                        //newobject.preserve_aspect_ratio = true;
                        break;
                    case "listbox":
                        if ( DEBUG_FILEFORMAT ) print( "added listbox\n" );
                        newobject = fe.add_listbox( object.x, object.y, object.width, object.height );
                        newobject.set_rgb( object.font_color.r, object.font_color.g, object.font_color.b );
                        newobject.set_bg_rgb( object.color.r, object.color.g, object.color.b );
                        newobject.bg_alpha = ( object.transparent ) ? 0 : 255;
                        newobject.set_selbg_rgb( layout.attr.mamewah.selected_bg_color.r, layout.attr.mamewah.selected_bg_color.g, layout.attr.mamewah.selected_bg_color.b );
                        newobject.selbg_alpha = ( layout.attr["mamewah"].transparent ) ? 0 : 255;
                        newobject.set_sel_rgb( layout.attr.mamewah.selected_color.r, layout.attr.mamewah.selected_color.g, layout.attr.mamewah.selected_color.b );
                        newobject.font = object.font;
                        switch ( object.text_align )
                        {
                            case 0:
                                newobject.align = Align.Left;
                                break;
                            case 1:
                                newobject.align = Align.Right;
                                break;
                            case 2:
                                newobject.align = Align.Centre;
                                break;
                        }
                        newobject.style = ( object.font_bold ) ? Style.Bold : Style.Regular;
                        newobject.charsize = object.font_size;
                        break;
                }
            }
        }
        
    }
    
}
