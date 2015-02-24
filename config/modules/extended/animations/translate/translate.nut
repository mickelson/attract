//push the animation name that users will use to the Animation table
Animation["translate"] <- function(c = {} ) {
    return TranslateAnimation(c);
}

class TranslateAnimation extends ExtendedAnimation {
    constructor(config) {
        base.constructor(config);
    }
    function getType() { return "TranslateAnimation"; }
    function start(obj) {
        //set defaults
        local defaults = {
            "from": "left",
            "to": "center"
        }
        if ("from" in config == false) config.from <- defaults["from"];
        if ("to" in config == false) config.to <- defaults["to"];
        //replace string value positions in from and to with correct positions
        if (typeof config.from == "string") config.start <- POSITIONS[config.from](obj) else config.start <- config.from;
        if (typeof config.to == "string") config.end <- POSITIONS[config.to](obj) else config.end <- config.to;
    }
    function frame(obj, ttime) {
        local point;
        if (config.reverse) {
            if (config.tween == "quadbezier") {
                local arc = 1.25;
                local controlpoint = [ (config.end[0] + config.end[1]) / 2 * arc, (config.start[0] + config.start[1]) / 2 * arc ]; 
                local bezier = quadbezier(config.end[0], config.end[1], controlpoint[0], controlpoint[1], config.start[0], config.start[1], t)
                point = [ bezier[0], bezier[1] ];
            } else {
                point = [   calculate(config.easing, config.tween, ttime, config.end[0], config.start[0], config.duration),
                            calculate(config.easing, config.tween, ttime, config.end[1], config.start[1], config.duration)
                        ];
            }
        } else {
            if (config.tween == "quadbezier") {
                local arc = 1.25;
                local controlpoint = [ (config.start[0] + config.start[1]) / 2 * arc, (config.end[0] + config.end[1]) / 2 * arc ]; 
                local bezier = quadbezier(config.start[0], config.start[1], controlpoint[0], controlpoint[1], config.end[0], config.end[1], t);
                point = [ bezier[0], bezier[1] ];
            } else {
                point = [   calculate(config.easing, config.tween, ttime, config.start[0], config.end[0], config.duration),
                            calculate(config.easing, config.tween, ttime, config.start[1], config.end[1], config.duration)
                        ];
            }
        }
        //ExtendedDebugger.notice(ttime + "," + config.start[0] + "x" + config.start[1] + " - " + config.end[0] + "x" + config.end[1] + "," + config.duration + " - " + point[0] + "x" + point[1]);
        obj.setPosition( point );
    }
}