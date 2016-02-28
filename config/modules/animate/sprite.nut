/*
    A SpriteAnimation simplifies animating an image spritesheet using subimg
    
    USAGE
    
        local config = {
            ...
        }
        animation.add( SpriteAnimation( object, config ) );
    
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
        restart = true,             // optional     whether the animation should restart from the beginning if it is already running
                                    //               - true (def)|false
        tween = Tween.Linear        // optional     tween to use for animation
        easing = Easing.Out         // optional     easing to use for animation
                                    //               - default is Linear / Out

    SpriteAnimation Config values:
    
        order = [ 0, 1, 2 ]         // optional     an array of frame indexes to specify the order they play in
                                    //               - no order specified plays first frame to last frame
        width = 128                 // optional     width of your sprite
                                    //               - default is 128
        height = 128                // optional     height of your sprite
                                    //               - default is same as width
        orientation = "horizontal"  // optional     orientation of your sprite frames in the image file
                                    //               - default is horizontal
        resource = "filename.png"   // optional     use this resource as the spritesheet
        mask = true                 // optional     use fix_masked_image() on your sprite sheet
        
    Notes:
    
    
*/    
class SpriteAnimation extends Animation
{
    BASE_PATH = fe.module_dir + "sprite/";
    version = 1.5;
    build = 100;
    mOffset = 0;                    //current texture mOffset, or array index for a frame array
    mFrameCount = 0;                //number of frames in the texture
    mFrameTime = 0;                 // time per frame
    mPlayed = 0;                    //number of times animation has played
    started = 0;
    
    constructor ( object, config = null )
    {
        base.constructor( config );
        
        //default config options
        if ( "width" in config == false ) config.width <- 128;
        if ( "height" in config == false ) config.height <- config.width;
        if ( "frame" in config == false ) config.frame = 0;
        if ( "order" in config == false ) config.order <- null;
        if ( "orientation" in config == false ) config.orientation <- "horizontal";
        if ( "resource" in config == true ) object.file_name = config.resource;
        if ( "mask" in config == false ) config.mask <- true;
        
        //initialize object and sprite size
        this.object = object;
        
        try
        {
            local f = file( object.file_name, "r" );
            if ( config.mask ) object.fix_masked_image();
        } catch ( e )
        {
            print("animate.nut: sprite.nut: Error reading file " + object.file_name + ". Make sure the image is valid\n");
        }
        
        object.subimg_width = config.width;
        object.subimg_height = config.height;
        frame(config.frame);
        
    }
    
    function onStart() {
        //??WHY WON'T IT RESTART LIKE PROPERTY ANIMS????
        //::print("START WAS CALLED\n");
        started = 0;
        object.subimg_width = config.width;
        object.subimg_height = config.height;
        frame(config.frame);
        
        //determine frame count
        if ( config.order != null )
        {
            //if the frame order is provided in the config, use it
            mFrameCount = config.order.len();
        } else
        {
            //if no order specified, base it on the orientation of the spritesheet
            switch ( config.orientation ) {
                case "vertical":
                    mFrameCount = object.texture_height / config.height;
                case "horizontal":
                default:
                    mFrameCount = object.texture_width / config.width;
            }
        }

        if ( mFrameCount )
            mFrameTime = config.time / mFrameCount;
            
    }
    
    function onUpdate()
    {
        
        //check if we need to reverse the animation for yoyo repeat
        //if ( config.repeat == "yoyo" ) reversed = (reversed && prev_frame() == last_frame() || !reversed && next_frame() == 0) ? reversed = !reversed : reversed;
        local current = AnimationCore.time();
        local elapsed = current - started;
        if ( elapsed >= mFrameTime )
        {
            started = current;
            //show the next frame
            local playFrame = ( reversed ) ? prev_frame() : next_frame();
            if ( reversed && playFrame == last_frame() || !reversed && playFrame == 0) mPlayed += 1;
            frame( playFrame );
            if ( playFrame == last_frame() ) stop();
            //print("playing: " + config.order[playFrame] + "\n");
        }
        //print ( "spf: " + mFrameTime + " time: " + time + " elapsed: " + elapsed + "\n" );
    }
    
    function onReverse()
    {
        //reverse order
        //print("reversing\n");
        //reversed = (reversed && prev_frame() == last_frame() || !reversed && next_frame() == 0) ? reversed = !reversed : reversed;
    }
    
    function onStop()
    {
        mPlayed = 0;
    }
    
    //shows a specific sprite frame
    function frame(which)
    {
        mOffset = which;
        switch ( config.orientation ) {
            case "vertical":
                object.subimg_y = ( config.order == null ) ? mOffset * config.height : config.order[mOffset] * config.height;
                break;
            case "horizontal":
            default:
                object.subimg_x = ( config.order == null ) ? mOffset * config.width : config.order[mOffset] * config.width;
                break;
        }
    }
    
    function last_frame()
    {
        if ( config.order == null )
        {
            switch ( config.orientation ) {
                case "vertical":
                    return (object.texture_height - config.height) / config.height;
                case "horizontal":
                default:
                    return (object.texture_width - config.width) / config.width;
            }
        } else
        {
            return config.order.len() - 1;
        }
    }
    
    function prev_frame()
    {
        if ( config.order == null )
        {
            //iterate each sprite frame until we reach the beginning
            switch ( config.orientation ) {
                case "vertical":
                    if ( mOffset > 0 ) return mOffset - 1; else return (object.texture_height - config.height) / config.height;
                case "horizontal":
                default:
                    if ( mOffset > 0 ) return mOffset - 1; else return (object.texture_width - config.width) / config.width;
            }
        } else
        {
            //get the previous sprite frame in a custom array, or the last if we reach the beginning
            if ( mOffset > 0 ) return mOffset - 1; else return config.order.len() - 1;
        }
    }
    
    function next_frame()
    {
        if ( config.order == null )
        {
            //iterate each sprite frame until we reach the end
            switch ( config.orientation ) {
                case "vertical":
                    if ( mOffset * config.height < object.texture_height - config.height ) return mOffset + 1; else return 0;
                case "horizontal":
                default:
                    if ( mOffset * config.width < object.texture_width - config.width ) return mOffset + 1; else return 0;
            }
        } else
        {
            //get the next sprite frame in a custom array, or the first if we reach the beginning
            if ( mOffset < config.order.len() - 1 ) return mOffset + 1; else return 0;
        }
    }

    function tostring() { return "SpriteAnimation"; }
}

//load presets
fe.load_module("animate/sprite/presets");
