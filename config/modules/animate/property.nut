/*
    A PropertyAnimation will animate an objects property over a given time. Properties can be any object
    property with a number value: (x, y, width, height, etc..)
    
    Additional pseudo properties are added for convenience:
        scale:      1.0
        position:   { x = 50, y = 50 }
        color:      { red = 255, green = 255, blue = 255 }

    USAGE
    
        local config = {
            ...
        }
        animation.add( PropertyAnimation( object, config ) );
    
    Default Config values:
    
        when = When.ToNewSelection, // optional     when to start animation
                                    //               - default is When.Immediate
        time = 500                  // optional     length of the animation in ms
                                    //               - default is 1000
        delay = 0                   // optional     time to wait before starting animation
        wait = false                // optional     whether this is a waiting transition animation
        loop = false                // optional     whether to loop the animation (at end, start from beginning)
                                    //               - integer|true|false (def)
        pulse = false,              // optional     whether to pulse animation (play in reverse at end)
                                    //               - integer|true|false (def)
        restart = true,             // optional     whether the animation should restart if it is already running and should run again
                                    //               - true (def)|false
        tween = Tween.Linear        // optional     tween to use for animation
        easing = Easing.Out         // optional     easing to use for animation
                                    //               - default is Linear / Out
    PropertyAnimation Config values:
        property = "scale",         // required     any property whose value is a number
        start = value,              // optional     value to start the animation at
                                    //               - depending on property, a single number value or table
                                    //               - i.e. scale would be 1.0, position would be { x = 0, y = 0 }
                                    //               - default is the current property value
        end = value,                // required     value to end the animation at
                                    //               - depending on property, a single number or table
                                    //               - i.e. scale would be 1.0, position would be { x = 0, y = 0 }
        center = auto               // optional     center point of rotation/scale
                                    //               - by default, rotation/scale occurs from object center point
        
    NOTES:
        
    Property values can also use strings with operators (+,-,/,* and %) for the 'start' and 'end' values,
    relative to the current value of the property
        Using an operator will add/subtr/mult/div from/to the current value:
            ex. "+50", "-50", "*50", "/50"
                this even works with "table" properties (position, color)
                start = { x = "-50", y = 100 } //for a position config
                start = { red = "-10", green = 255, blue = 255 }    //for a color config
        Adding a percentage sign will calculate a percent based on the current value:
            ex. "50%" would be 50% of the current value
                "+50%" would be equal to "150%", or 50% larger than the current value
    
    Optional start and end values:
        By default, if not provided, start and end values will be the current property value.
        Carefully pay attention to what values you are (or are not) providing.
        This can make things a little confusing, especially when using operator values for only the start or end,
        but can also be very useful.
        
    When using position, rotation and scale, you can use the 'center' config option to specify the point to rotate/scale objects.
    Default is center of the object, meaning scale or rotations would keep the object in the same location.

*/
class PropertyAnimation extends Animation
{
    version = 1.5;
    build = 100;
    object = null;
    origin = null;
    ostart = null;
    oend = null;
    constructor( object, config = null )
    {
        base.constructor( config );
        this.object = object;

        if ( "center" in config == false ) config.center <- "auto";
        
        //store the original start/end values, as they might be modified later
        if ( "start" in config == true) ostart = config.start;
        if ( "end" in config == true) oend = config.end;
        
        //store the original object values as a reference
        origin = { x = object.x, y = object.y, width = object.width, height = object.height, mx = object.x + ( object.width / 2 ), my = object.y + (object.height / 2) }
    }
    
    function onStart()
    {
        //set the initial start values
        //if start or end values are not specified, use the current objects value each time
        //if start or end values ARE specified, we evaluate them for strings with operators
        switch( config.property )
        {
            case "position":
                if ( ostart == null ) config.start <- { x = object.x, y = object.y }; else config.start <- { x = evaluate(ostart.x, object.x), y = evaluate(ostart.y, object.y) };
                if ( oend == null ) config.end <- { x = object.x, y = object.y }; else config.end <- { x = evaluate(oend.x, object.x), y = evaluate(oend.y, object.y) };
                break;
            case "color":
                if ( ostart == null ) config.start <- { red = object.red, green = object.green, blue = object.blue }; else config.start <- { red = evaluate(ostart.red, object.red), green = evaluate(ostart.green, object.green), blue = evaluate(ostart.blue, object.blue) };
                if ( oend == null ) config.end <- { red = object.red, green = object.green, blue = object.blue }; else config.end <- { red = evaluate(oend.red, object.red), green = evaluate(oend.green, object.green), blue = evaluate(oend.blue, object.blue) };
                break;
            case "scale":
                if ( ostart == null ) config.start <- 1.0; else config.start <- evaluate(ostart, getScale());
                if ( oend == null ) config.end <- 1.0; else config.end <- evaluate(oend, getScale());
                break;
            default:
                if ( ostart == null ) config.start <- object[config.property]; else config.start <- evaluate(ostart, object[config.property]);
                if ( oend == null ) config.end <- object[config.property]; else config.end <- evaluate(oend, object[config.property]);
                break;
        }
    }
    
    function onUpdate()
    {
        local value = null;
        // calculate the current value based on the time and duration
        // for position, scale and rotation, we use the calculated values to perform a transform_translate()
        switch (config.property)
        {
            case "position":
                value = {
                    x = calculate( config.start.x, config.end.x, time, config.time, config.tween, config.easing ),
                    y = calculate( config.start.y, config.end.y, time, config.time, config.tween, config.easing )
                }
                transform_translate(value.x, value.y, getScale(), object.rotation, "position");
                break;
            case "x":
                value = calculate( config.start, config.end, time, config.time, config.tween, config.easing ),
                transform_translate(value, origin.y, getScale(), object.rotation, "x");
                break;
            case "y":
                value = calculate( config.start, config.end, time, config.time, config.tween, config.easing ),
                transform_translate(origin.x, value, getScale(), object.rotation, "y");
                break;
            case "scale":
                value = calculate( config.start, config.end, time, config.time, config.tween, config.easing );
                transform_translate(origin.x, origin.y, value, object.rotation, "scale");
                break;
            case "rotation":
                value = calculate( config.start, config.end, time, config.time, config.tween, config.easing );
                transform_translate(origin.x, origin.y, getScale(), value, "rotation");
                break;
            case "color":
                value = {
                    red = calculate( config.start.red, config.end.red, time, config.time, config.tween, config.easing ),
                    green = calculate( config.start.green, config.end.green, time, config.time, config.tween, config.easing ),
                    blue = calculate( config.start.blue, config.end.blue, time, config.time, config.tween, config.easing )
                }
                object.set_rgb(value.red, value.green, value.blue);
                break;
            default:
                value = calculate( config.start, config.end, time, config.time, config.tween, config.easing );
                object[config.property] = value;
                break;
        }
    }
    
    function onReverse() {
        //for reverse we just swap out the start and end values
        if ( reversed )
        {
            config.start = oend;
            config.end = ostart;
        } else
        {
            config.start = ostart;
            config.end = oend;
        }
    }
    
    function onCancel() {}
    function onRestart() {
        // if ( !reversed ) onStart();
    }
    function onStop() {}
    
    function getScale()
    {
        if ( object.width <= 0 || origin.width <= 0 )
        {
            return 1.0;
        }
        return object.width / origin.width;
    }
    
    //updates x/y/w/h location for transformation, rotation and scale from original object
    function transform_translate(x, y, scale, rotation, from = "")
    { 
        local newWidth = origin.width * scale;
         local newHeight = origin.height * scale;
        
        local transX = origin.x;
        local transY = origin.y;
        
        local scaleMidX = transX + newWidth / 2;
        local scaleMidY = transY + newHeight / 2;
        
        local radians = rotation * PI / 180.0;
        //move x and y based on center of scaled, rotated object
        local newX = ( transX - scaleMidX ) * cos(radians) - ( transY - scaleMidY ) * sin(radians) + scaleMidX;
        local newY = ( transX - scaleMidX ) * sin(radians) + ( transY - scaleMidY ) * cos(radians) + scaleMidY;
        
        object.x = newX + ( origin.mx - scaleMidX ) + ( x - origin.x );
        object.y = newY + ( origin.my - scaleMidY ) + ( y - origin.y );
        object.width = newWidth;
        object.height = newHeight;
        object.rotation = rotation;
        
        //::print( "from: " + from + " loc: " + object.x + "," + object.y + " scale: " + scale + " rotation: " + rotation + "\n");
        
        
        // ::print( from + " scale: " + scale + " rotation: " + rotation + "\n");
        // local newWidth = origin.width * scale;
        // local newHeight = origin.height * scale;
        
        // local midX = origin.x + origin.width / 2;
        // local midY = origin.y + origin.height / 2;
        
        // local scaleMidX = origin.x + newWidth / 2;
        // local scaleMidY = origin.y + newHeight / 2;
        
        // local radians = rotation * PI / 180.0;
        // //move x and y based on center of scaled, rotated object
        // local newX = ( origin.x - scaleMidX ) * cos(radians) - ( origin.y - scaleMidY ) * sin(radians) + scaleMidX;
        // local newY = ( origin.x - scaleMidX ) * sin(radians) + ( origin.y - scaleMidY ) * cos(radians) + scaleMidY;
        
        // object.x = newX + ( midX - scaleMidX );
        // object.y = newY + ( midY - scaleMidY );
        // object.width = newWidth;
        // object.height = newHeight;
        // object.rotation = rotation;
    }
        
    function tostring() { return "PropertyAnimation"; }
}

//load presets
fe.load_module("animate/property/presets");