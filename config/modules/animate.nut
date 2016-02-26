fe.load_module("file");

::AnimateVersion <- 1.1;
DEBUG_ANIMATION <- false;

/*
    Animate module allows you to use and create custom animations in the frontend via the AnimationCore and Animation classes.
    
    AnimationCore is stored in the root squirrel table 'animation'. Depending on the animation used, this may animate existing
    objects or run a custom animation that creates objects, such as particle effects.
    
    The following animation classes are currently included:
        PropertyAnimation
        SpriteAnimation
        ParticleAnimation

    USAGE:
    
        Add the animate module to your layout:
        fe.add_module("animate");
        
        Animation classes typically accept an object and a config table, or just a config table.
        To create an animation, we create an instance of an animation class, and add it using animation.add():
        
        animation.add( AnimationClass( obj, cfg ) ); // for animations that apply to objects
        animation.add( AnimationClass( cfg ) ); //for animations that create their own objects

        PropertyAnimation example:
            Animate any property of an object whose value is a number.

            ex.
            local myText = fe.add_text("My Text", 0, 0, 100, 30);
            local animConfig = {
                property = "alpha",
                start = 255,
                end = 100,
                pulse = true
            }
            animation.add( PropertyAnimation( myText, animConfig ) );
    
    ANIMATION CONFIGS:
    
        Configs are flexible and options vary depending on the Animation class you are using. The base options are:
    
        //handled by Animation base class
        when = When.ToNewSelection          // when the animation starts (can use Transition, but other When options are available)
        time = 500                          // length of animation in ms
        delay = 0                           // delay before animation starts in ms
        wait = false                        // whether this is a waiting animation during transitions (complete before continuing)
        loop = false                        // whether to loop the animation (can also be a number)
        pulse = false                       // whether to pulse the animation (can also be a number), use onReverse() to handle reversing your animation - by default, it will reverse start/end values
        tween = Tween.Linear                // type of tween to use during animation (use calculate to determine tween/easing value)
        easing = Easing.Out                 // type of easing to use during animation (use calculate to determine tween/easing value)
    
        Other Animation classes may have specify their own config values that may be required or optional

    ADVANCED USERS:
        
        While the animation class handles the animation itself, you can do your own operations by including
        custom functions to perform at various stages of the animation:
        
        local animConfig = {
            onInit = function( anim ) {
                ::print("my anim initialized\n");
            },
            onStart = function( anim ) {
                ::print("my anim started\n");
            },
            onUpdate = function( anim ) {
                ::print("my anim updated\n");
            },
            onRestart = function( anim ) {
                ::print("my anim restarted\n");
            },
            onStop = function( anim ) {
                ::print("my anim stopped\n");
            },
            when = function( anim ) {
                //this can be used to evaluate when an animation should start, which is checked in the tick callback
                return true;
            },
        }
        
        This can be helpful simply for debugging or if you want something to occur at some point duration the animation. The 
        animation instance is passed to the function, so you can get any information or config values at any point.

    DEVELOPMENT:
        
        The Animation class is a base class which can be extended by developers to create new animations. It handles things such as
        timing, starting, updating and stopping animations.
    
        See the included animation classes in the modules/animate folder for examples.

        ** If you create any custom animations, consider submitting them to me to be included with the animate module
        for others to use. **
        
        A typical animation class:
        
        class MyAnimation extends Animation
        {
            constructor( object, config = null )
            {
                //if you will do anything with an object, include it in the constructor and set it
                this.object = object;
                //config is a required parameter
                base.constructor( config );
                //do initialization stuff here
            }
            
            //do init when animation is added
            function onInit() { }
            //do start stuff
            function onStart() { }
            //do restart stuff
            function onRestart() { }
            //do update stuff
            function onUpdate() { }
            //do stop stuff
            function onStop() { }
            //give your animation a name
            function tostring() { return "MyAnimation"; }
        }
        
        Animations are automatically started and stopped based on the 'when' and 'time' option specified by the user.
        If you want greater control, the following functions are available to you, and can be called on your animation:
            init(), start(), restart(), reverse(), update(), stop()

        Some helper functions are available to use:
            calculate(start, end, time, duration, tween = "linear", easing = "out")
                Typically used in your onUpdate function to figure out a value for your animation.
                If you use this function to calculate your current value on a time based animation, the values are calculated
                using Penner's easing equations, which allows the user to specify tween and easing animation types for
                our animation.
            AnimationCore.time() or ::animation.time()
                the current clock time, in ms
            
        Because your animation extends the base class, they should not overwrite the base options, unless there is a reason for it.
        For consistency, you should make use of them whenever possible. Make sure to document any differences so the user understands
        how to create an animation config.
        
        //optional, but should be used when possible
        start = startValue                  // value when starting the animation
        end = endValue                      // value when ending the animation
        
        The following variables are available to you, and in use by the base class (so should not be overwritten):
            config: the config provided to the animation
            running: if the animation is running (true|false)
            time: the running time of the animation
            startTime: the clock time the animation started
            endTime: the clock time the animation will end (unless it is cancelled)
            reversed: if the animation is in a pulse state (reversed)
            played: number of times the animation ran
            transition.ttime: if started in transition state, the transition ttime, else null
            transition.ttype: if started in transition state, the transition ttype, else null
            transition.var: if started in transition state, the transition var, else null
            
        
    TODO LIST
        
        better presets for property, particles, sprites
        fix pulse / loop / restart (delays and repeating)
        animation sets: be apart of animation core, or at discretion of developer?
        quad bezier arcs: function available, how to implement?
        
        fix pos/scale/rot transform_translate - must be in specific order right now
        fix scale and rotation when image/artwork doesn't specify width (unscaled) - will need to get texture_width and texture_height in the transition callback
        fix explosion of particle effects after launching a game? (unconfirmed)
        
        evaluate function improvements:
            currently "-50" for end, even with a start value set continously removes 50
            if start is provided, maybe make end relative to start
            if end was provided, maybe make start relative to end?
            or allow for last = "s+10" or "e-10%"? for modifying based on start or end values instead of current?

        how to handle transition info: mainly for waiting transitions, but need start variables for others
        state saving: how will we stored modified values, if the screensaver activates? currently modified values get reset
        custom when functions: improvements for calling them in transition/tick
        bounds: set bounds instead of a start/end value for position?
        Other animation classes?
            KeyFrameAnimation
            Interpolation?
            Color?
        property.nut
            do start/last/current work for values right now?
            entrance and exit presets
            center positioning: add proper centering with rotate, scale, skew and pinch
            properties before delay: should we set properties to start value before the delay occurs?
            multiple properties at once ( may rely on animation sets )
        particles.nut
            ?
        sprite.nut
            respect restart option - restart sprite animation from beginning if when transition is reached again
*/

//options for 'when' an animation should start
When <- {
    StartLayout = Transition.StartLayout,
    EndLayout = Transition.EndLayout,
    ToNewSelection = Transition.ToNewSelection,
    FromOldSelection = Transition.FromOldSelection,
    ToGame = Transition.ToGame,
    FromGame = Transition.FromGame,
    ToNewList = Transition.ToNewList,
    Always = 100,
    StartFrontend = function( anim ) { if ( anim.transition.ttype == Transition.StartLayout && anim.transition.var == FromTo.Frontend ) return true; return false; },
    EndFrontend = function( anim ) { if ( anim.transition.ttype == Transition.EndLayout && anim.transition.var == FromTo.Frontend ) return true; return false; },
    StartScreenSaver = function( anim ) { if ( anim.transition.ttype == Transition.EndLayout && anim.transition.var == FromTo.ScreenSaver ) return true; return false; },
    EndScreenSaver = function( anim ) { if ( anim.transition.ttype == Transition.StartLayout && anim.transition.var == FromTo.ScreenSaver ) return true; return false; },
    OnPrevious = function( anim ) { if ( anim.transition.ttype == Transition.FromOldSelection && anim.transition.var == 1 ) return true; return false; },
    OnNext = function( anim ) { if ( anim.transition.ttype == Transition.FromOldSelection && anim.transition.var == -1 ) return true; return false; },
    OnPageUp = function( anim ) { if ( anim.transition.ttype == Transition.FromOldSelection && anim.transition.var == -fe.layout.page_size ) return true; return false; },
    OnPageDown = function( anim ) { if ( anim.transition.ttype == Transition.FromOldSelection && anim.transition.var == fe.layout.page_size ) return true; return false; },
}

//aliases to tweens that are available for animations
Tween <- {
    Linear = "linear",
    Cubic = "cubic",
    Quad = "quad",
    Quart = "quart",
    Quint = "quint",
    Sine = "sine",
    Expo = "expo",
    Circle = "circle",
    Elastic = "elastic",
    Back = "back",
    Bounce = "bounce"
}

//aliases to easings that are available for animations
Easing <- {
    Out = "out",
    In = "in",
    OutIn = "outin",
    InOut = "inout"
}

//helper for positioning
Screen <- {
    center = { x = fe.layout.width / 2, y = fe.layout.height / 2 },
    percent_width = function( p ) { return (p.tofloat() / 100) * fe.layout.width; },
    percent_height = function( p ) { return (p.tofloat() / 100) * fe.layout.height; }, 
}

const OFFSCREEN = 20;

LOCATIONS <- {
    top = function(o) { return o.y }
    left = function(o) { return o.x }
    right = function(o) { return o.x + o.width }
    bottom = function(o) { return o.y + o.height }
}

POSITIONS <- {
    center = function(o) { return { x = Screen.center.x - (o.width / 2), y = Screen.center.y - (o.height / 2) } },
    start = function(o) { return { x = o.config.start_x, y = o.config.start_y } },
    last = function(o) { if ("last" in o.config) return { x = o.config.last.x, y = o.config.last.y }; return start(o); },
    current = function(o) { return { x = o.x, y = o.y } },
    top = function(o) { return { x = Screen.center.x - (o.width / 2), y = 0 } },
    left = function(o) { return { x = 0, y = Screen.center.y - (o.height / 2) } },
    bottom = function(o) { return { x = Screen.center.x - (o.width / 2), y = fe.layout.height - o.height } },
    right = function(o) { return { x = fe.layout.width - o.width, y = Screen.center.y - (o.height / 2) } },
    topleft = function(o) { return { x = 0, y = 0 } },
    topright = function(o) { return { x = fe.layout.width - o.width, y = 0 } },
    bottomleft = function(o) { return { x = 0, y = fe.layout.height - o.height } },
    bottomright = function(o) { return { x = fe.layout.width - o.width, y = fe.layout.height - o.height } },
    midtop = function(o) { return { x = Screen.center.x - (o.width / 2), y = (Screen.center.y / 2) - (o.height / 2) } },
    midbottom = function(o) { return { x = Screen.center.x - (o.width / 2), y = Screen.center.y + (Screen.center.y / 2) - (o.height / 2) } },
    midleft = function(o) { return { x = (Screen.center.x / 2) - (o.width / 2), y = Screen.center.y - (o.height / 2) } },
    midright = function(o) { return { x = Screen.center.x + (Screen.center.x / 2) - (o.width / 2), y = Screen.center.y - (o.height / 2) } },
    midtopleft = function(o) { return { x = (Screen.center.x / 2) - (o.width / 2), y = (Screen.center.y / 2) - (o.height / 2) } },
    midbottomleft = function(o) { return { x = (Screen.center.x / 2) - (o.width / 2), y = Screen.center.y + (Screen.center.y / 2) - (o.height / 2) } },
    midtopright = function(o) { return { x = Screen.center.x + (Screen.center.x / 2) - (o.width / 2), y = (Screen.center.y / 2) - (o.height / 2) } },
    midbottomright = function(o) { return { x = Screen.center.x + (Screen.center.x / 2) - (o.width / 2), y = Screen.center.y + (Screen.center.y / 2) - (o.height / 2) } },
    offmidtopleftx = function(o) { return { x = -o.width - OFFSCREEN, y = (Screen.center.y / 2) - (o.height / 2) } },
    offmidtoplefty = function(o) { return { x = (Screen.center.x / 2) - (o.width / 2), y = -o.height - OFFSCREEN } },
    offmidbottomleftx = function(o) { return { x = -o.width - OFFSCREEN, y = Screen.center.y + (Screen.center.y / 2) - (o.height / 2) } },
    offmidbottomlefty = function(o) { return { x = (Screen.center.x / 2) - (o.width / 2), y = fe.layout.height + OFFSCREEN } },
    offmidtoprightx = function(o) { return { x = fe.layout.width + OFFSCREEN, y = (Screen.center.y / 2) - (o.height / 2) } },
    offmidtoprighty = function(o) { return { x = Screen.center.x + (Screen.center.x / 2) - (o.width / 2), y = -o.height - OFFSCREEN } },
    offmidbottomrightx = function(o) { return { x = fe.layout.width + OFFSCREEN, y = Screen.center.y + (Screen.center.y / 2) - (o.height / 2) } },
    offmidbottomrighty = function(o) { return { x = Screen.center.x + (Screen.center.x / 2) - (o.width / 2), y = fe.layout.height + OFFSCREEN } },
    offtop = function(o) { return { x = Screen.center.x - (o.width / 2), y = -o.height - OFFSCREEN } },
    offbottom = function(o) { return { x = Screen.center.x - (o.width / 2), y = fe.layout.height + o.height + OFFSCREEN } },
    offleft = function(o) { return { x = - o.width - OFFSCREEN, y = Screen.center.y - (o.height / 2) } },
    offright = function(o) { return { x = fe.layout.width + o.width + OFFSCREEN, y = Screen.center.y - (o.height / 2) } },
    offtopleftx = function(o) { return { x = - o.width - OFFSCREEN, y = 0 } },
    offtoplefty = function(o) { return { x = 0, y = - o.height - OFFSCREEN } },
    offtopleft = function(o) { return { x = - o.width - OFFSCREEN, y = - o.height - OFFSCREEN } },
    offtoprightx = function(o) { return { x = fe.layout.width + OFFSCREEN, y = 0 } },
    offtoprighty = function(o) { return { x = 0, y = - o.height - OFFSCREEN } },
    offtopright = function(o) { return { x = fe.layout.width + OFFSCREEN, y = - o.height - OFFSCREEN } },
    offbottomleftx = function(o) { return { x = - o.width - OFFSCREEN, y = 0 } },
    offbottomlefty = function(o) { return { x = 0, y = fe.layout.height + OFFSCREEN } },
    offbottomleft = function(o) { return { x = - o.width - OFFSCREEN, y = fe.layout.height + OFFSCREEN } },
    offbottomrightx = function(o) { return { x = fe.layout.width + OFFSCREEN, y = 0 } },
    offbottomrighty = function(o) { return { x = 0, y = fe.layout.height + OFFSCREEN } },
    offbottomright = function(o) { return { x = fe.layout.width + OFFSCREEN, y = fe.layout.height + OFFSCREEN } }
}

// The core animation class which stores animation and provides some
//  helper functions
class AnimationCore
{
    animations = [];
    
    //allows users to add a new animation
    function add( animation )
    {
        animations.append( animation );
        animation.init();
    }
    
    //return the system time in ms
    function time()
    {
        return fe.layout.time.tofloat();
    }
    
    //handle transitions for all animations
    function transition_callback(ttype, var, ttime)
    {
        
        //we'll return true if there is any wait animations running
        local busy = false;
        foreach ( anim in animations )
        {
            //update or start animations
            if ( anim.running )
            {
                
                if ( "when" in anim.config && anim.config.when == ttype && ttime == 0 && anim.config.restart )
                {
                    //TODO: will this run while waiting?
                    //if this same animation for this transition is already running, restart it
                    anim.saveTransition( ttype, var, ttime );
                    anim.start();
                } else
                {
                    //update animation
                    if ( anim.config.wait ) anim.transition.ttime = ttime;
                    anim.update();
                }
                if ( anim.config.wait ) busy = true;
            } else
            {
                if ( "when" in anim.config && anim.config.when == ttype && ttime == 0 )
                {
                    //start animation for the current transition
                    anim.saveTransition( ttype, var, ttime );
                    anim.start();
                    if ( anim.config.wait ) busy = true;
                } else if ( "when" in anim.config && typeof( anim.config.when ) == "function" )
                {
                    //run custom when function
                    anim.saveTransition( ttype, var, ttime );
                    local shouldStart = anim.config.when( anim );
                    if ( shouldStart ) anim.start();
                    //prevent restarts, removes saved transition values
                    anim.emptyTransition();
                }
            }
        }
        if ( busy )
        {
            print("warning: transition waiting not recommended (use animation delays instead)\n");
            return true;
        }
        return false;
    }

    //handle ticks for all animations
    function ticks_callback(ttime)
    {
        foreach ( anim in animations )
        {
            if ( anim.running )
            {
                //update animation if running
                anim.update();
            } else
            {
                //check if we need to start an animation
                if ( "when" in anim.config && typeof( anim.config.when ) == "function" )
                {
                    //run custom when function
                    local shouldStart = anim.config.when( anim );
                    //&& ::animation.time() < anim.endTime
                    if ( shouldStart ) anim.start();
                }
            }
        }
    }

    //  Penner's Easing equations
    //  http://www.robertpenner.com/easing/
    //  these functions allow us to do neat animations from point a to point b
    //  t = current time
    //  b = beginning value
    //  c = change in value
    //  d = duration
    penner = {
        "linear": function (t, b, c, d) { return c * t / d + b; }
        "in": {
            "cubic": function (t, b, c, d) { return c * pow(t / d, 3) + b; },
            "quad": function (t, b, c, d) { return c * pow(t / d, 2) + b; },
            "quart": function (t, b, c, d) { return c * pow(t / d, 4) + b; },
            "quint": function (t, b, c, d) { return c * pow(t / d, 5) + b; },
            "sine": function (t, b, c, d) { return -c * cos(t / d * (PI / 2)) + c + b; },
            "expo": function (t, b, c, d) { if ( t == 0) return b; return c * pow(2, 10 * ( t / d - 1)) + b; },
            "circle": function (t, b, c, d) { return -c * (sqrt(1 - pow(t / d, 2)) - 1) + b; },
            "elastic": function (t, b, c, d, a = null, p = null) { if (t == 0) return b; t /= d; if (t == 1) return b + c; if (p == null) p = d * 0.37; local s; if (a == null || a < abs(c)) { a = c; s = p / 4; } else { s = p / (PI * 2) * asin(c / a); } t = t - 1; return -(a * pow(2, 10 * t) * sin((t * d - s) * (PI * 2) / p)) + b; },
            "back": function (t, b, c, d, s = null) { if (s == null) s = 1.70158; t /= d; return c * t * t * ((s + 1) * t - s) + b; },
            "bounce": function (t, b, c, d) { return c - AnimationCore.penner["out"]["bounce"](d - t, 0, c, d) + b; }
        },
        "out": {
            "cubic": function (t, b, c, d) { t /= d; t -= 1; return c * (pow(t, 3) + 1) + b; },
            "quad": function (t, b, c, d) { t /= d; return -c * t * (t - 2) + b; },
            "quart": function (t, b, c, d) { t /= d; t -= 1; return -c * (pow(t, 4) - 1) + b; },
            "quint": function (t, b, c, d) { t /= d; t -= 1; return c * (pow(t, 5) + 1) + b; },
            "sine": function (t, b, c, d) { return c * sin(t / d * (PI / 2)) + b; },
            "expo": function (t, b, c, d) { if (t == d) return b + c; return c * (-pow(2, -10 * (t / d)) + 1) + b;},
            "circle": function (t, b, c, d) { t /= d; t -= 1; return (c * sqrt(1 - pow(t, 2)) + b); },
            "elastic": function (t, b, c, d, a = null, p = null) { if (t == 0) return b; t = t / d; if (t == 1) return b + c; if (p == null) p = d * 0.37; local s; if (a == null || a < abs(c)) { a = c; s = p / 4; } else { s = p / (PI * 2) * asin(c / a); } return (a * pow(2, -10 * t) * sin((t * d - s) * (PI * 2) / p) + c + b).tofloat(); },
            "back": function (t, b, c, d, s = null) { if (s == null) s = 1.70158; t /= d; t -= 1; return c * (t * t * ((s + 1) * t + s) + 1) + b; },
            "bounce": function (t, b, c, d) { t /= d; if (t < 1 / 2.75) { return c * (7.5625 * t * t) + b; } else if ( t < 2 / 2.75) { return c * (7.5625 * (t -= (1.5/2.75)) * t + 0.75) + b; } else if ( t < 2.5 / 2.75 ) { return c * (7.5625 * (t -= (2.25/2.75)) * t + 0.9375) + b; } else { return c * (7.5625 * (t -= (2.625/2.75)) * t + 0.984375) + b;} }
        },
        "inout": {
            "cubic": function (t, b, c, d) { t /= d; t *= 2; if (t < 1) return c / 2 * t * t * t + b; t = t - 2; return c / 2 * (t * t * t + 2) + b; },
            "quad": function (t, b, c, d) { t /= d; t *= 2; if (t < 1) return c / 2 * pow(t, 2) + b; return -c / 2 * ((t - 1) * (t - 3) - 1) + b; },
            "quart": function (t, b, c, d) {  t /= d; t *= 2; if (t < 1) return c / 2 * pow(t, 4) + b; t = t - 2; return -c / 2 * (pow(t, 4) - 2) + b; },
            "quint": function (t, b, c, d) { t /= d; t *= 2; if (t < 1) return c / 2 * pow(t, 5) + b; t = t - 2; return c / 2 * (pow(t, 5) + 2) + b; },
            "sine": function (t, b, c, d) { return -c / 2 * (cos(PI * t / d) - 1) + b; },
            "expo": function (t, b, c, d) { if (t == 0) return b; if (t == d) return b + c; t /= d; t *= 2; if (t < 1) return c / 2 * pow(2, 10 * (t - 1)) + b; t = t - 1; return c / 2 * (-pow(2, -10 * t) + 2) + b; },
            "circle": function (t, b, c, d) { t /= d; t *= 2; if (t < 1) return -c / 2 * (sqrt(1 - t * t) - 1) + b; t = t - 2; return c / 2 * (sqrt(1 - t * t) + 1) + b; },
            "elastic": function (t, b, c, d, a = null, p = null) { if (t == 0) return b; t /= d; t *= 2; if (t == 2) return b + c; if (p == null) p = d * (0.3 * 1.5); local s; if (a == null || a < abs(c)) { a = c;  s = p / 4; } else { s = p / (PI * 2) * asin(c / a); } if (t < 1) return -0.5 * (a * pow(2, 10 * (t - 1)) * sin((t * d - s) * (PI * 2) / p)) + b; return a * pow(2, -10 * (t - 1)) * sin((t * d - s) * (PI * 2) / p) * 0.5 + c + b; },
            "back": function (t, b, c, d, s = null) { if (s == null) s = 1.70158; s = s * 1.525; t /= d; t *= 2; if (t < 1) return c / 2 * (t * t * ((s + 1) * t - s)) + b; t = t - 2; return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b; },
            "bounce": function (t, b, c, d) { if (t < d / 2) return AnimationCore.penner["in"]["bounce"](t * 2, 0, c, d) * 0.5 + b; return AnimationCore.penner["out"]["bounce"](t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b; }
        }
    }
        
    //just for debugging purposes, so we know which 'when' we are talking about
    function when(w) {
        switch (w) {
            case 0:
                return "StartLayout";
            case 1:
                return "EndLayout";
            case 2:
                return "ToNewSelection";
            case 3:
                return "FromOldSelection";
            case 4:
                return "ToGame";
            case 5:
                return "FromGame";
            case 6:
                return "ToNewList";
            case 7:
                return "EndNavigation";
            case 100:
                return "OnDemand";
            case 101:
                return "Always";
        }
    }
}

// The base animation class, which can be extended
class Animation
{
    object = null;  //object animation will run on, if any
    config = null;  //config table
    
    running = false;    //is the animation running?
    startTime = 0;      //start time (system clock)    
    endTime = 0;        //expected end time, per config values for time and delay
    time = 0;           //current animation time, in ms
    
    reversed = false;   //is the animation reversed?
    played = 0;         //number of times the animation has played
    
    transition = null;  //current transition values
    
    constructor( userConfig = null )
    {
        if ( DEBUG_ANIMATION ) print( "Loaded animation\n" );
        config = ( userConfig != null ) ? userConfig : {}
        
        //default animation config options
        if ("when" in config == false) config.when <- When.Always;
        if ( "delay" in config == false ) config.delay <- 0;
        if ( "time" in config == false ) config.time <- 500;
        if ( "loop" in config == false ) config.loop <- false;
        if ( "pulse" in config == false ) config.pulse <- false;
        if ( "restart" in config == false ) config.restart <- true;
        if ( "tween" in config == false ) config.tween <- Tween.Linear;
        if ( "easing" in config == false ) config.easing <- Easing.Out;
        if ( "wait" in config == false ) config.wait <- false;
        
        transition = { ttype = null, var = null, ttime = -1 }
    }
    
    //called to initialize the animation (once the animation has been added to the animation table)
    function init()
    {
        if ( DEBUG_ANIMATION ) print( "init animation\n" );
        //call the animations onInit functions
        onInit();
        if ( "onInit" in config == true ) config.onInit(this);
        
        //start always animations
        if ( config.when == When.Always ) start();
    }
    
    //called to start the animation
    function start()
    {
        if ( DEBUG_ANIMATION ) print( "start animation\n" );
        if ( running )
        {
            //run the animations onRestart functions before we reset everything
            onRestart();
            if ( "onRestart" in config == true ) config.onRestart(this);
        }
        
        time = 0;
        startTime = AnimationCore.time();
        endTime = startTime + config.delay + config.time;
        
        if ( !running )
        {
            running = true;
            //run the animations onStart functions
            onStart();
            if ( "onStart" in config == true ) config.onStart(this);
        }
    }
    
    //just an alias to start
    function restart() { 
        if ( DEBUG_ANIMATION ) print( "restart animation\n" );
        start();
    }
    
    //called to update the animation
    function update( cancelled = false )
    {
        if ( DEBUG_ANIMATION ) print( "update animation\n" );
        //update time after delay has occured
        local delayedStart = AnimationCore.time() - startTime - config.delay;
        if (delayedStart >= 0)
        {
            //if a cancel was requested, sets time to the end
            if ( !cancelled ) time = delayedStart else time = endTime;
        }
        
        //limit time to duration of animation
        if ( config.when != When.Always && time > config.time ) time = config.time;

        //run the animations onUpdate functions
        if ( delayedStart >= 0 )
        {
            onUpdate();
            if ( "onUpdate" in config == true ) config.onUpdate(this);
        }
        
        if ( cancelled )
        {
            //run the animations onCancel functions
            onCancel();
            if ( "onCancel" in config == true ) config.onCancel(this);
        }
        
        //stop the animation when we reach the end
        if ( config.when != When.Always && time >= config.time ) stop();
    }
    
    //called to cancel the animation
    function cancel() { 
        if ( DEBUG_ANIMATION ) print( "cancel animation\n" );
        update(true);
    }
    
    //called to stop the animation
    function stop()
    {
        if ( DEBUG_ANIMATION ) print( "stop animation\n" );
        played++;
        
        if ( ( typeof(config.loop) == "integer" && played < config.loop ) || ( typeof(config.pulse) == "integer" && played < config.pulse ) )
        {
            //TODO update startTime/endTime or allow start/restart to do it
            //if loop or pulse are a number and we haven't reached it, restart the animation
            if ( config.pulse ) reverse();
            restart();
        } else if ( !config.wait && ( config.loop == true || config.pulse == true ) )
        {
            //if this isn't a waiting transition, continously loop/pulse
            if ( config.pulse ) reverse();
            restart();
        } else
        {
            //stop the animation
            running = false;
            played = 0;
            //run the animations onStop functions
            onStop();
            if ( "onStop" in config == true ) config.onStop(this);
        }
    }
    
    function reverse()
    {
        if ( DEBUG_ANIMATION ) print( "reverse animation\n" );
        reversed = !reversed;
        //run the animations onReverse functions
        if ( reversed )
        {
            onReverse();
            if ( "onReverse" in config == true ) config.onReverse(this);
        }
    }

    //stores the current transition information, removed once transition is complete
    function saveTransition(ttype, var, ttime)
    {
        transition = { ttype = ttype, var = var, ttime = ttime }
    }
    
    //removes the last transition information
    function emptyTransition()
    {
        //print("transition emptied\n");
        transition = { ttype = null, var = null, ttime = -1 }
    }
    
    //these will be implemented in extended classes
    function onInit() {}
    function onStart() {}
    function onUpdate() {}
    function onCancel() {}
    function onRestart() {}
    function onStop() {}
    function onReverse() {}

    //use Penner functions to calculate the current value based on a start/end value and a current time/duration
    function calculate(start, end, time, duration, tween = "linear", easing = "out" )
    {
        //don't go past duration
        if ( time > duration ) time = duration;
        local change = end - start;
        local newValue = null;
        if (easing == null || tween == null || tween == "linear")
        {
            //default to a linear value
            newValue = AnimationCore.penner["linear"](time, start, change, duration);
        } else
        {
            //get the new value with the proper tween and easing
            switch(easing) {
                case "outin":
                    //outin uses in and out functions based of the first or second half of the animation
                    if (time < duration / 2) {
                        newValue = AnimationCore.penner["out"][tween](time * 2, start, change / 2, duration);
                    } else {
                        newValue = AnimationCore.penner["in"][tween]((time * 2) - duration, start + change / 2, change / 2, duration);
                    }
                    break;
                default:
                    newValue = AnimationCore.penner[easing][tween](time, start, change, duration);
                    break;
            }
        }
        return newValue;
    }

    //evaluate for relative string values (+int, -int, /int, *int, int%)
    //relTo is what the operators are relative to. i.e. +10 might relative to object.x
    function evaluate(val, relTo)
    {
        if ( typeof(val) == "string" )
        {
            local first = val.slice(0, 1);
            local last = val.slice(val.len() - 1, val.len());
            local rest = ( last == "%"  ) ? val.slice(0, val.len() - 1) : val.slice(1, val.len());
            switch ( first )
            {
                case "+":
                    if ( last == "%" ) return relTo + ( relTo * ( rest.tofloat() / 100 ) );
                    return relTo + rest.tofloat();
                case "-":
                    if ( last == "%" ) return relTo - ( relTo * ( rest.tofloat() / 100 ) );
                    return relTo - rest.tofloat();
                case "/":
                    if ( last == "%" ) return relTo / ( relTo * ( rest.tofloat() / 100 ) );
                    return relTo / rest.tofloat();
                case "*":
                    if ( last == "%" ) return relTo * ( relTo * ( rest.tofloat() / 100 ) );
                    return relTo * rest.tofloat();
                default:
                    if ( last == "%" ) return relTo * ( rest.tofloat() / 100 );
            }
            return val.tofloat();
        }
        return val;
    }
    
    //provide a way to do arcs
    function quadbezier(p1x, p1y, cx, cy, p2x, p2y, t) {
        local c1x = p1x + (cx - p1x) * t;
        local c1y = p1y + (cy - p1y) * t;
        local c2x = cx + (p2x - cx) * t;
        local c2y = cy + (p2y - cy) * t;
        local tx = c1x + (c2x - c1x) * t;
        local ty = c1y + (c2y - c1y) * t;
        return { x = tx, y = ty };
    }
        
    function tostring() { return "Animation"; }
}

//we hook the AnimationCore into the animation table for fe scripting
animation <- AnimationCore();
fe.add_transition_callback(animation, "transition_callback" );
fe.add_ticks_callback(animation, "ticks_callback" );

//load any animations in the animations folder
local path = fe.module_dir + "animate/";
local dir = DirectoryListing( path, false );
foreach ( f in dir.results )
{
    try
    {
        if ( f.slice( f.len() - 4 ) == ".nut" )
        {
            if ( DEBUG_ANIMATION ) print( "Loading animation: " + f + "\n" );
            fe.load_module( "animate/" + f );
        }
    }catch ( e )
    {
        print( "animate.nut: Error loading animation: " + f );
    }
}


