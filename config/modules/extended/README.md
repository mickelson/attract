#ExtendedObjects and Animate library

The extended objects library extends the capabilities of the existing Attract-Mode objects.

**ExtendedObjects**
ExtendedObjects is a module you can include with your Attract-Mode layout to provide more functionality. It is also designed for expandability, allowing developers to create their own unique objects that can be reused in ExtendedObjects.

**Animate**
Animate is a module that relies on ExtendedObjects to make it easy to add animations to objects in your Attract-Mode layout. It is also designed for expandability, allowing developers to create their own custom animations and animation configurations.

##ExtendedObjects Usage
```Squirrel
    //load required file
    fe.load_module("extended\extended.nut");
    //add your objects (note the addition of an id before the standard AttractMode parameters)
    ExtendedObjects.add_artwork("mainScreenshot", "snap", 0, 0, fe.layout.width, fe.layout.height);
    //the id allows you to later access your objects by the id you provided
    //note all object variables are now methods - set_shadow = true is now setShadow(true)
    ExtendedObjects.get("mainScreenshot").setShadow(true);
```
For more on using ExtendedObjects, check out the [Tutorial Layouts](https://github.com/liquid8d/attract-extra/tree/master/layouts).

##Animate Usage
----------------
```Squirrel
    //load required files
    fe.load_module("extended\extended.nut");
    fe.load_module("extended\animate.nut");
    //add your objects (note the addition of an id before the standard parameters)
    ExtendedObjects.add_image("logo", "logo.png", 0, 0, fe.layout.width, fe.layout.height);
    ExtendedObjects.add_artwork("game", "snap", 0, 0, fe.layout.width, fe.layout.height);
    //add your animations
    //here we create an animation config that will be passed onto the object describing which animation, where to animate from and where to animate to
    local moveLogo = {
        which = "translate",
        from = "offtopright",
        to = "top"
    }
    ExtendedObjects.get("logo").animate(moveLogo);
    //you can provide many options for the animation in the config table, options are shown below:
    local offscreen = {
        which = "translate",
        duration = 1500,
        when = When.FromOldSelection,
        easing = "out",
        tween = "back",
        from = "offscreenbottom",
        to = "bottom"
    }
    ExtendedObjects.get("game").animate(offscreen);
```

###.animate() parameters
    cfg:                table (surround with { } ) that contains your animation config options

###.animate() config variables
    OPTION              DEFAULT                         DESCRIPTION
    which               translate                       type of animation: "translate" or "property" (or others if you include a file from extended\animations\..)
    when                When.FromOldSelection           when to run animation, a transition type (When.TYPE) or When.Always
    wait                true                            wait until the animation is finished before the next transition starts
    restart             true                            if an animation is still running and transition state occurs again, restart it
    kind                transition                      one of transition (start to finish), loop (repeat start to finish) or yoyo (forward, then backward)
    repeat              1                               number of times to repeat animation
    delay:              0                               delay before the animation starts in ms
    duration:           1000                            length of animation in ms
    easing              out                             easing animation type to use: in, out, inout, outin
    tween               quad                            tween animation to use:
                                                          You can use one of the following: (See: http:easings.net/# for more info)
                                                          back, bounce, circle, cubic, elastic
                                                          expo, linear, quad, quart, quint, sine
                                                          quadbezier (an arc - WIP)
                                                          
    reverse             false                           perform animation in reverse (true|false)

###Property config variables (which = "property")
    property            alpha                           the property to animate
    from                                                animation starts at this value
    to:                                                 animation ends at this value
    Default 'from' and 'to' differ depending on property
                      alpha: 0 to 255
                      x: offleft to start
                      y: offtop to start
                      width: object width to object width + 50
                      height: object height to object height + 50
                      skew_x: 0 to 10
                      skew_y: 0 to 10
                      pinch_x: 0 to 10
                      pinch_y: 0 to 10
                      rotate: 0 to 90

###Translate config variables (which = "translate")
    from:               offscreenbottom                 animation starts from this position - an array [ x, y ] or position "center"
    to:                 center                          animation goes to this position - an array [ x, y ] or position "bottom"

###Animation Sets
You can group multiple animations together in a "set". A set is an array of tables, or configs. To use it:

object.animate_set(set);

You can use some existing presets:
    fade_in_out         fades out (alpha) on Transition.ToNewSelection, fades in on Transition.FromOldSelection
    hover               simulate a floating object

Or you can create your own:
```Squirrel
local myAnimationSet = [
    {
        which = "translate",
        duration = 1500,
        when = When.FromOldSelection,
        easing = "out",
        tween = "back",
        from = "offscreenbottom",
        to = "bottom"
    },
    {
        which = "property",
        duration = 1500,
        property = "alpha",
        from = 0,
        to = 255
    }
];
```

###Positions
A nice feature of using ExtendedObjects is you can use positions to easily place objects at certain locations:
```
object.setPosition("center");
```
You can see a full list of positions here:
[positions.png](https://raw.githubusercontent.com/liquid8d/attract-extra/master/extras/positions.png)

These positions can also be used for animations 'from' or 'to' variables in your animation config:
```
    local anim = {
        which = "translate",
        from = "offright",
        to = "right",
        ...
    }
```

**object positions**
```
    start|last|current
```
**screen positions**
```
    top|left|right|bottom|center
    topleft|topright|bottomleft|bottomright
```
**offscreen positions**
```
    offtop|offleft|offright|offbottom
    offtopleft|offtopright|offbottomleft|offbottomright
    offtopleftx|offtoprightx|offbottomleftx|offbottomrightx
    offtoplefty|offtoprighty|offbottomlefty|offbottomrighty
```

###A Few Notes:
* positions are primarily for translate animations, but do work for x and y property animations
* positions for x and y - remember you are only setting x or y - so if you go to 'bottomright', it's only going to go along the x or y axis
* from = "current" to = wherever would be a one-time animation since on the next animation the from location would be the same as where its already at
* from = "last" might be tricky to use, but if one animation finishes before the other starts, it might be useful
* certain 'easing' and 'tween' options might be odd in some situations

##Developers
*** WIP ***
Another nice feature of ExtendedObjects is it makes it pretty simple for coders to create new objects or animations.

###Creating Objects
You can look at some of the WIP objects for examples in extended\objects\.

To create a new object that can be used with ExtendedObjects:
*create a new folder & file in: layouts\extended\objects\myobject\myobject.nut
*create a class extending ExtendedObject
```Squirrel
    class MyObject extends ExtendedObject {
        constructor(id, x, y, w, h) {
            base.constructor(id, x, y);
            //initialize your objects
            //NOTE: current you must set at least one object, working on this
            object = fe.add_text("", 0, 0, 0, 0);
            //add any callbacks you are interested in using
            ExtendedObjects.add_callback(this, "onTick");
            ExtendedObjects.add_callback(this, "onTransition");
        }
        function onTick(params) {
            //Note all params for callbacks are past in through a 'params' object
            local ttime = params.ttime;
            //do your work here
        }
        function onTransition(params) {
            local ttype = params.ttype;
            local var = params.var;
            local ttime = params.ttime;
            //do your work here
            
            //You must return false if you are not doing continuous work
            return false;
        }
    }
```
*add a hook function so users can add your object:
```Squirrel
    ExtendedObjects.add_myobject <- function(id, x, y, w, h) {
        return ExtendedObjects.add(MyObject(id, x, y, w, h));
    }
```
*In your layout.nut file, add your includes and object:
```Squirrel
    fe.load_module("extended\extended.nut");
    fe.load_module("extended\animate.nut");
    fe.load_module("extended\objects\myobject\myobject.nut");
    ExtendedObjects.add_myobject("coolObject", 0, 0, 100, 100);
```

###Creating Animations
***WIP***
You can look at some of the WIP animations for examples in extended\anims\.

To create a new animation that can be used with ExtendedObjects:
*create a new folder & file in: layouts\extended\anims\myanim\myanim.nut
*create a class extending ExtendedAnimation:
```Squirrel
    //push the animation name that users will use to the Animation table
    Animation["myanim"] <- function(c = {} ) {
        return MyAnim(c);
    }
    class MyAnim extends ExtendedAnimation {
        constructor(config) {
            base.constructor(config);
            //add any properties and their defaults to the config that you might want users to change
            if ("my_property" in config == false) config.my_property <- "default_value";
            //you can modify existing values if you want as well
            config.easing = "out";
            config.tween = "sine";
        }
        //return a friendly name for your object type
        function getType() { return "MyAnim"; }
        function start(obj) {
            base.start(obj);
            //your animation is starting, initialize it
            //obj is an ExtendedObject the animation will run on
        }
        function frame(obj, ttime) {
            base.frame(obj);
            //run your animated frame
            //protip: you can use the calculate(easing, tween, ttime, duration, start, end) to get automated easings from point to point
        }
        function stop(obj) {
            base.stop(obj);
            //finish up
        }
    }
```
*In your layout.nut file, add your includes and object:
```Squirrel
    fe.load_module("extended\extended.nut");
    fe.load_module("extended\animate.nut");
    fe.load_module("extended\animations\myanim\myanim.nut");
    local obj = ExtendedObjects.add_image("img", "frame.png", 0, 0, 100, 100);
    local cfg = {
                    which = "my_anim",
                    when = ...
                    ...
                    my_property = ""
                }
    obj.animate(cfg);
```

### Developing Further
***WIP***
ExtendedObjects and Animate can be extended even further than objects and animations. Any class you create can hook into ExtendedObjects or Animations via callbacks:

*Create a nut file: my.nut
*Create a class
```Squirrel
    class MyThing {
        constructor() {
            //hook into ExtendedObjects or Animate callbacks
            ExtendedObjects.add_callback(this, "onObjectAdded");
            ExtendedObjects.add_callback(this, "onTick");
            ExtendedObjects.add_callback(this, "onTransition");
            ExtendedObjects.add_callback(this, "onAnimationStart");
            ExtendedObjects.add_callback(this, "onAnimationFrame");
            ExtendedObjects.add_callback(this, "onAnimationStop");
            //add your own callbacks for other interested listeners
            local params = {
                param1 = "a",
                param2 = "b"
            }
            ExtendedObjects.run_callback("onMyThingInitialized", params);
        }
        //if the callback functions exist, they will be executed
    }
```
* Other interested classes can now listen for your callbacks using ExtendedObjects.add_callback("onMyThingInitialized");


##Recently Added (1.4)
* convert to module
* add_surface implemented
* add_clone implemented 
* layer updates
* new tutorial layouts
* additions of new object methods for 1.3/1.4
    * Image
        * swap = swap()
            * should a shadow remain on the original object? i think so
        * video_flags = getVideoFlags()/setVideoFlags()
        * video_playing = getVideoPlaying()/setVideoPlaying()
        * video_duration = getVideoDuration()
        * video_time = getVideoTime()
        * file_name = getFilename()/setFilename()
        * set_rgb() = setColor() - already exists
        * set_pos() = setPosition()/setSize() already exist
    * Listbox
        * format_string = getFormatString()/setFormatString()
        * set_rgb() = setColor() - already exists
        * set_bg_rgb() = getBGColor()/setBGColor() already exist
        * set_sel_rgb() = getSelectionColor()/setSelectionColor() already exist
        * set_selbg_rgb() = getSelectionBGColor()/setSelectionBGColor() already exist
        * set_pos() = setPosition()/setSize() already exist
    * Text
        * set_rgb() = setColor() - already exists
        * set_bg_rgb() = getBGColor()/setBGColor() already exist
        * set_pos() = setPosition()/setSize() already exist

##TODO
This is my active todo list (bugs and features):
* Why are callbacks not running (in Debug, callbacks are no longer working)
* callbacks not quite implemented properly (if the same environment asks for multiple callbacks, it might call them more than once.. or not at all?)
* Particles
    * create particle objects immediately, don't wait to draw or there will be layer/surface issues
    * particle bug (wigs out - suddenly snow goes up or default comes from left of screen instead of center)
* Animations
    * allow for animations at different object events (When.OnObjectVisible/When.OnObjectHidden/When.OnObjectSelected?)
    * check on waiting and non-waiting transition animations now using clock()
    * start/current position not working?
* Can we implement the overload methods? Looks like ExtendedObjects needs to be a class
* flipping with subimg doesn't seem to be working for me
* play with using surfaces as "screens"

###Issues
This is a list of known issues:
* Core
    * fe.overlay not implemented - not sure what we will do with this yet
    * fe.signal not implemented - nor sure what we will do with this yet
    * add_shader not implemented - not sure what we will do with this yet
    * add_sound not implemented - not sure what we will do with this yet
    * does not verify availability of ExtendedObjects and Animate library
    * does not validate user entered config variables
    * possible variable/config naming conflicts?
* Objects
    * objects that extend ExtendedObject must currently add an empty object
* Animations
    * still not always smooth (not starting from beginning)
    * are we converted to using clock for tick and transitions?
    * from/to = "current" won't work on a delayed animation if the position has changed (because it gets the current coordinates immediately instead of waiting until the delay is done)
    * dont start animation until after delay?
        * current won't work with chained animations, since current is set before the first animation finishes
* Particles


###Enhancements
This is a list of enhancements I am considering adding to the library:
* should the module be separate modules (objects, layers, positions, animations)
* surface enhancements - users create named layers that they can add objects to
* transform - scale + rotate from center
    setCenterAlign option for objects? If enabled, modify X/Y/W/H with -width / 2 and -height / 2
* add objects the same way we add animations, move objects into the objects folder (add_object)(
* need a way to distinguish and run object animations vs. non-object animations (not attach animations to objects)
* make POSITIONS a more user friendly method
* move position functions into a method attached to objects instead of POSITIONS?
* method chaining config creator
    use AnimationConfig class instead of table to allow for proper method chaining
* improve debugger
* work to make vertical+horizontal layouts work properly
    adjust animation speed for vertical layouts (should slow them down based on the aspect)
* changeFrom changeTo? Allow us to do value + or - amounts
* animated sprite sets with subimg?
* improve method and property naming - base it on other animation libraries (flash or android)
* add_clone method - shadows uses clones, but separate images/artwork do not
* some freakin' cool new objects (wheel, randomwheel, etc)
* color - from rgb to rgb
* allow positions when adding objects
* multiple transitions in animation config
* provide config values for animations like back, expo, elastic for p, a or s
* reorder objects on the draw list
* new objects (marquee wheel, etc)
    Modified Orbit with options (# of slots, horizontal/vertical, spacing)
* quad bezier improvements (control point, arc option)
* animation paths - multiple points animations
* screen transitions - move all objects on screen (ex. everything at top of screen slides up, everything at bottom slides down)
* work with shaders, sound, plugin_command
* utilities library? (user friendly string handling for game_info)
* animation presets
    Arc Grow, Blur, Bounce, Bounce Around 3D, Bounce Random, Chase Ease, Elastic, Elastic Bounce, Fade, Flag, Flip, Grow, Grow Blur Grow Bounce, Grow Center Shrink, Grow X, Grow Y, Pendulum Pixelate, Pixelate Zoom out, Rain Float, Scroll, Stripes, Stripes 2 Strobe, Sweep Left, Sweep Right
    Fade, Fly In, Float In, Split, Wipe, Shape, Wheel, Random Bars, Grow & Turn, Zoom, Swivel, Bounce, Pulse, Color Pulse, Teeter, Spin, Grow/Shrink,  Desaturate, Darken, Lighten, Transparency, Object Color,   Cut, Fade, Push, Wipe, Split, Reveal, Random bars, Shape, Uncover, Flash Fall Over, Drape, Curtains, Wind, Prestige, Fracture, Crush, Peel off Page Curl
    Resting animations: Hover, Pulse, Rock, Spin
* animation chains - chain multiple animations together (without needing multiple and delays)
* onDemand animations?
* reorder objects on draw list - sort?
