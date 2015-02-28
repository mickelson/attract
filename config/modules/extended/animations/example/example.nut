//push the animation name that users will use to the Animation table
Animation["example"] <- function(c = {} ) {
    return ExampleAnimation(c);
}

const PCOUNT = 256;

class ExampleAnimation extends ExtendedAnimation {
    particles = null;
    
    constructor(config) {
        base.constructor(config);
        particles = array(PCOUNT);
        local resources = [];
        local d = EXTOBJ_DIR + "animations/example/";

        local names=["invader.png","invader2.png","invader3.png"];
        foreach ( n in names )
            resources.append(fe.add_image( d+n,-32,-32,32,32 ) );

        for (local i = 0; i < PCOUNT; i++) {
            particles[i] = ExampleParticle(i, resources[random(0,2)], 0, 0, fe.layout.width, fe.layout.height);
        }
    }

    function getType() { return "ExampleAnimation"; }
    
    function random(minNum, maxNum) {
        return floor(((rand() % 1000 ) / 1000.0) * (maxNum - (minNum - 1)) + minNum);
    }

    function start(obj) {
        foreach (p in particles) {
            p.reset();
            p.visible(true);
        }
    }
    
    function frame(obj, ttime) {
        foreach (p in particles) {
            p.set(  calculate(config.easing, config.tween, ttime, p.start[0], p.end[0], config.duration),
                    calculate(config.easing, config.tween, ttime, p.start[1], p.end[1], config.duration)
             );
        }
    }
    
    function stop(obj) {
        //we don't need to do anything on stop
    }
}

class ExampleParticle {
    id = 0;
    object = null;
    minX = 0;
    minY = 0;
    maxX = 0;
    maxY = 0;
    start = null;
    end = null;
    forwardAnim = true;
    constructor(id, resource, minX, minY, maxX, maxY) {
        object = fe.add_clone(resource);
        this.minX = minX;
        this.minY = minY;
        this.maxX = maxX;
        this.maxY = maxY;
        reset();
    }
    
    function random(minNum, maxNum) {
        return floor(((rand() % 1000 ) / 1000.0) * (maxNum - minNum) + minNum);
    }
    
    function reset() {
        forwardAnim = !forwardAnim;
        if (start == null) {
            start = [   random(minX, maxX),
                        random(minY, maxY) ];
            object.alpha = random(25, 255);
            object.set_rgb(random(0,255), random(0,255), random(0,255));
            object.width = object.height = random(24, 36);
        } else {
            start = end;
        }
        end = [   random(minX, maxX),
                    random(minY, maxY) ];
        set(start[0], start[1]);
    }
    
    function setColor(r, g, b) {
        object.set_rgb(r, g, b);
    }
    
    function set(x, y) {
        object.x = x;
        object.y = y;
    }
    
    function visible(v) { object.visible = v; }
}
