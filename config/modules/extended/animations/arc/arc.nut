//push the animation name that users will use to the Animation table
Animation["arc"] <- function(c = {} ) {
    return ArcAnimation(c);
}
class ArcAnimation extends ExtendedAnimation {
    controlX = 0;
    controlY = 0;
    
    constructor(config) {
        base.constructor(config);
        if ("size" in config == false) config.size <- 200;
    }
    
    function getType() { return "ArcAnimation"; }
    
    function start(obj) {
        //find out the control point for quadbezier
        controlX = config.to[0] - config.from[0] / 2;
    }
    
    function frame(obj, ttime) {
        local t = (ttime.tofloat() / config.duration).tofloat();
        local bezier = quadbezier( config.from[0], config.from[1], controlX, controlY, config.to[0], config.to[1], t );
        local point = [   calculate(config.easing, config.tween, ttime, config.from[0], config.to[0], config.duration),
                            calculate(config.easing, config.tween, ttime, config.from[1], config.to[1], config.duration)
                        ];
        obj.setPosition( bezier );
        //obj.setPosition( [ point[0], arcY ] );
        /*
        controlY = (config.to[1] - config.from[1] / 2) - config.size;
        local point = [ calculate( config.easing, config.tween, ttime, bezier[0], bezier[1], config.duration ),
                        calculate( config.easing, config.tween, ttime, bezier[0], bezier[1], config.duration )
                      ];
        */
    }
    
    function quadbezier(p1x, p1y, cx, cy, p2x, p2y, t) {
        local c1x = p1x + (cx - p1x) * t;
        local c1y = p1y + (cy - p1y) * t;
        local c2x = cx + (p2x - cx) * t;
        local c2y = cy + (p2y - cy) * t;
        local tx = c1x + (c2x - c1x) * t;
        local ty = c1y + (c2y - c1y) * t;
        return [ tx, ty ];
    }
}