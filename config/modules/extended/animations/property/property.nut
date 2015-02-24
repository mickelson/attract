//push the animation name that users will use to the Animation table
Animation["property"] <- function(c = {} ) {
    return PropertyAnimation(c);
}

class PropertyAnimation extends ExtendedAnimation {
    constructor(config) {
        base.constructor(config);
        //set defaults
        if ("property" in config == false) config.property <- "alpha";
    }
    function getType() { return "PropertyAnimation"; }
    function start(obj) {
        local defaults = {
            "alpha": [ 0, 255 ],
            "x": [ "offleft", "top" ],
            "y": [ "offtop", "top" ],
            "scale": [ 1.0, 2.0 ],
            "skew_x": [ 0, 10 ],
            "skew_y": [ 0, 10 ],
            "pinch_x": [ 0, 10 ],
            "pinch_y": [ 0, 10 ],
            "width": [ obj.getWidth(), obj.getWidth() + 50 ],
            "height": [ obj.getHeight(), obj.getHeight() + 50 ],
            "rotation": [ 0, 90 ],
        }
        if ("from" in config == false) config.from <- defaults[config.property][0];
        if ("to" in config == false) config.to <- defaults[config.property][1];
        switch (config.property) {
            case "x":
            case "y":
                local point = 0;
                if (config.property == "y") point = 1;
                if (typeof config.from == "string") config.start <- POSITIONS[config.from](obj)[point] else config.start <- config.from;
                if (typeof config.to == "string") config.end <- POSITIONS[config.to](obj)[point] else config.end <- config.to;
                break;
            default:
                config.start <- config.from;
                config.end <- config.to;
                break;
        }
    }
    function frame(obj, ttime) {
        local value;
        if (config.reverse) value = calculate(config.easing, config.tween, ttime, config.end, config.start, config.duration) else value = calculate(config.easing, config.tween, ttime, config.start, config.end, config.duration);
        switch (config.property) {
            case "x":
                obj.setX(value);
                break;
            case "y":
                obj.setY(value);
                break;
            case "scale":
                obj.setScale(value);
                break;
            case "skew_x":
                obj.setSkewX(value);
                break;
            case "skew_y":
                obj.setSkewY(value);
                break;
            case "pinch_x":
                obj.setPinchX(value);
                break;
            case "pinch_y":
                obj.setPinchY(value);
                break;
            case "width":
                obj.setWidth(value);
                break;
            case "height":
                obj.setWidth(value);
                break;
            case "rotation":
                obj.setRotation(value);
                break;
            case "alpha":
                obj.setAlpha(value);
                break;
        }
    }
}
