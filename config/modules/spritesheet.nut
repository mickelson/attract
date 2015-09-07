///////////////////////////////////////////////////
//
// Attract-Mode Frontend - "spritesheet" module
//
//  ******************************************************************************************
//  NOTE: THIS MODULE IS OUTDATED. I RECOMMEND USING A SPRITEANIMATION WITH THE ANIMATE MODULE.
//        THIS MAY HAVE SOME FEATURES THAT ARE NOT YET SUPPORTED, SO IT WILL STICK AROUND FOR NOW.
//  ******************************************************************************************
//
// SpriteSheet allows you to create animations from an image with multiple "sprites"
//
// Usage:
//  //create an image object with your spritesheet
//  local joy = fe.add_image("joystick-move.png");
//  //create a SpriteSheet instance with your object and sprite size
//  local sprite = SpriteSheet(joy, 128, 128);
//      //settings
//      sprite.orientation = "vertical";    //orientation of your sprite frames, default is horizontal
//      sprite.repeat = "yoyo";             //a number or either loop or yoyo, default is loop
//      sprite.spf = 1;                     //seconds per frame, default is 1
//      sprite.order = [ 3, 0, 1, 3 ];      //optional array with a custom frame index order
//
//      //control
//      sprite.frame(0);    //set to a specific frame
//      sprite.reset();     //reset the animation
//      sprite.stop();      //stop the animation
//      sprite.start();     //start the animation
//      
//      //info
//      local last = sprite.last_frame();  //will tell you what the last frame index is
//      local next = sprite.next_frame();  //will tell you what the next frame index will be
//      local prev = sprite.prev_frame();  //will tell you what the next frame index will be
//      local played = sprite.playCount();    //will tell you the number of times the animation has played (reset on reset)
///////////////////////////////////////////////////
const SPRITESHEET_VERSION=1;

class SpriteSheet
{
    //TODO
    //single row/column only for now - implement row/columns as frames based on texture size
    //played does not increase for yoyo
    //bug - doing last frame again after stop?
    //spf or fps?
    
    //settings you care about
    orientation = "horizontal";     //either horizontal or vertical spritesheet
    width = 0;                      //sprite height
    height = 0;                     //sprite width
    spf = 1;                        //seconds per frame
    order = null;                   //an array with a custom frame order to use from the spritesheet
    repeat = "loop";                //a number or either loop or yoyo
    
    //internal stuff
    mObj = null;                    //object to animate
    mOffset = 0;                    //current texture offset, or array index for a frame array
    mTimer = 0;                     //timer to watch elapsed time
    mRunning = false;               //is animation running?
    mReverse = false;               //whether we are running animation in reverse
    mPlayed = 0;                    //number of times animation has played
    
    constructor( obj, w, h = null )
    {
        mObj = obj;
        width = w;
        height = ( h == null ) ? width : h;
        //set initial sprite size
        mObj.subimg_width = width;
        mObj.subimg_height = height;
        fe.add_ticks_callback( this, "on_tick" );
    }
    
    function start()
    {
        mRunning = true;
    }
    
    function stop()
    {
        mRunning = false;
    }
    
    function reset()
    {
        //reset to first frame
        mPlayed = 0;
        frame( 0 ); 
    }
    
    function playCount()
    {
        return mPlayed;
    }
    
    function on_tick( ttime )
    {
        if ( mObj != null && mRunning )
        {
            local elapsed = ttime - mTimer;
            if ( elapsed > spf * 1000 )
            {
                mTimer = ttime - ( elapsed - ( spf * 1000 ) );
                //check if we need to reverse the animation for yoyo repeat
                if ( repeat == "yoyo" ) mReverse = (mReverse && prev_frame() == last_frame() || !mReverse && next_frame() == 0) ? mReverse = !mReverse : mReverse;
                //show the next frame
                local playFrame = ( mReverse ) ? prev_frame() : next_frame();
                if ( mReverse && playFrame == last_frame() || !mReverse && playFrame == 0) mPlayed += 1;
                if ( typeof repeat == "integer" && mPlayed > repeat) stop();
                frame( playFrame );
                //print( "frame: " + mTimer + " offset: " + mOffset + " reverse: " + mReverse + " played: " + mPlayed + "\n" );
            }
        }
    }
    
    //shows a specific sprite frame
    function frame(which)
    {
        mOffset = which;
        switch ( orientation ) {
            case "horizontal":
                mObj.subimg_x = ( order == null ) ? mOffset * width : order[mOffset] * width;
                break;
            case "vertical":
                mObj.subimg_y = ( order == null ) ? mOffset * height : order[mOffset] * height;
                break;
        }
    }
    
    function last_frame()
    {
        if ( order == null )
        {
            switch ( orientation ) {
                case "horizontal":
                    return (mObj.texture_width - width) / width;
                case "vertical":
                    return (mObj.texture_height - height) / height;
            }
        } else
        {
            return order.len() - 1;
        }
    }
    
    //finds out which is the previous frame offset based on settings
    function prev_frame()
    {
        if ( order == null )
        {
            //iterate each sprite frame until we reach the beginning
            switch ( orientation ) {
                case "horizontal":
                    if ( mOffset > 0 ) return mOffset - 1; else return (mObj.texture_width - width) / width;
                case "vertical":
                    if ( mOffset > 0 ) return mOffset - 1; else return (mObj.texture_height - height) / height;
            }
        } else
        {
            //get the previous sprite frame in a custom array, or the last if we reach the beginning
            if ( mOffset > 0 ) return mOffset - 1; else return order.len() - 1;
        }
    }
    
    //finds out which is the next frame offset based on settings
    function next_frame()
    {
        if ( order == null )
        {
            //iterate each sprite frame until we reach the end
            switch ( orientation ) {
                case "horizontal":
                    if ( mOffset * width < mObj.texture_width - width ) return mOffset + 1; else return 0;
                case "vertical":
                    if ( mOffset * height < mObj.texture_height - height ) return mOffset + 1; else return 0;
            }
        } else
        {
            //get the next sprite frame in a custom array, or the first if we reach the beginning
            if ( mOffset < order.len() - 1 ) return mOffset + 1; else return 0;
        }
    }

}
