/*
    A ParticleAnimation will create particles to enhance your layout.
    
    USAGE
    
        local config = {
            ...
        }
        animation.add( ParticleAnimation( object, config ) );
    
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
    
    ParticleAnimation Config values:
    
        resources = [ "1.png", "2.png" ]    // optional     an array of image resources that will be used
                                            //               - a default image is used if nothing is specified
        ppm = 60                            // optional     particles per minute
                                            //               - default is 60
        x = 480                             // optional     x location of the particle emitter
        y = 480                             // optional     y location of the particle emitter
                                            //               - default is screen center
        width = 1                           // optional     width of the particle emitter
        height = 1                          // optional     height of the particle emitter
        limit = 0                           // optional     limit of emitted particles
        movement = true                     // optional     if particles are affected by speed, accel, gravity, xOscillate    
        angle = [ 0, 0 ]                    // optional     min and max angle of particles emitted
        speed = [ 150, 150 ]                // optional     min and max speed of particles emitted over lifespan
        scale = [ 1.0, 1.0 ]                // optional     min and max scale of particles emitted over lifespan
        startScale = [ 1.0, 1.0 ]           // optional     min and max scale of particle at emission 
        rotate = [ 0, 0 ]                   // optional     mix and max rotation of particle over lifespan
        rotateToAngle = false               // optional     whether to rotate to angle?
        fade = 0                            // optional     length of time in ms to fade the particles alpha value
        gravity = 0                         // optional     gravity modification ( -75 to 75 )
        accel = 0                           // optional     add acceleration ( 0 to 20 )
        bound = [ 0, 0, 0, 0 ]              // optional     a bounding rectangle that particles will not leave
        xOscillate = [ 0, 0 ]               // optional     modify amp/freq of particle emission (
        lifespan = 5000                     // optional     lifespan of particle
        particlesontop = true               // optional     ???
        pointSwarm = [ 0, 0 ]               // optional     ???
        blendmode = "none"                  // optional     ???
        randomFrame = false                 // optional     ???
    
    Notes:
        The ParticleAnimation system is based on the HyperTheme particle system.
    
*/

class ParticleAnimation extends Animation
{
    BASE_PATH = fe.module_dir + "particles/";
    version = 1.5;
    build = 100;
    debug = false;
    resources = null;
    emitter = null;
    particles = null;
    timePerParticle = 0;
    count = 0;
    
    lastUpdate = 0;
    elapsed = 0;
    
    //used for debugging
    debug_emitter = null;
    debug_angle_min = null;
    debug_angle_max = null;
    debug_bounds = null;
    
    constructor( config = null )
    {
        base.constructor( config );
        if ("surface" in config == false ) config.surface <- fe;
        if ("resources" in config == false) config.resources <- [ BASE_PATH + "images/default.png" ];
        //emitter variables
        if ("ppm" in config == false) config.ppm <- 60;
        if ("x" in config == false) config.x <- fe.layout.width / 2;
        if ("y" in config == false) config.y <- fe.layout.height / 2;
        if ("width" in config == false) config.width <- 1;
        if ("height" in config == false) config.height <- 1;
        if ("limit" in config == false) config.limit <- 0;
        //particle variables
        if ("movement" in config == false) config.movement <- true;
        if ("angle" in config == false) config.angle <- [ 0, 0 ];
        if ("speed" in config == false) config.speed <- [ 150, 150 ];
        if ("scale" in config == false) config.scale <- [ 1.0, 1.0 ];       //scale over time
        if ("startScale" in config == false) config.startScale <- [ 1.0, 1.0 ];  //random scale
        if ("rotate" in config == false) config.rotate <- [ 0, 0 ];
        if ("rotateToAngle" in config == false) config.rotateToAngle <- false;
        if ("fade" in config == false) config.fade <- 0;
        if ("gravity" in config == false) config.gravity <- 0;
        if ("accel" in config == false) config.accel <- 0;
        if ("bound" in config == false) config.bound <- [ 0, 0, 0, 0 ] else config.bound <- [ config.x + config.bound[0], config.y + config.bound[1], config.x + config.bound[2], config.y + config.bound[3] ];
        if ("xOscillate" in config == false) config.xOscillate <- [ 0, 0 ];
        //todo
        if ("lifespan" in config == false) config.lifespan <- 5000;
        if ("particlesontop" in config == false) config.particlesontop <- true;
        if ("pointSwarm" in config == false) config.pointSwarm <- [ 0, 0 ];
        if ("blendmode" in config == false) config.blendmode <- "none";
        if ("randomFrame" in config == false) config.randomFrame <- false;
        
        //set limitations
        config.lifespan = minmax(config.lifespan, 500, 20000);
        config.angle[0] = minmax(config.angle[0], 0, 360);
        config.angle[1] = minmax(config.angle[1], 0, 360);
        config.speed[0] = minmax(config.speed[0], 0, 2000);
        config.speed[1] = minmax(config.speed[1], 0, 2000);
        config.rotate[0] = minmax(config.rotate[0], -50, 50);
        config.rotate[1] = minmax(config.rotate[1], -50, 50);
        config.scale[0] = minmax(config.scale[0], 0.1, 3.0);
        config.scale[1] = minmax(config.scale[1], 0.1, 3.0);
        config.startScale[0] = minmax(config.startScale[0], 0.1, 3.0);
        config.startScale[1] = minmax(config.startScale[1], 0.1, 3.0);
        config.fade = minmax(config.fade, 0, 10000);
        config.accel = minmax(config.accel, 0, 20);
        config.gravity = minmax(config.gravity, -75, 75);
        config.xOscillate[0] = minmax(config.xOscillate[0], 0, 50);
        config.xOscillate[1] = minmax(config.xOscillate[1], 0, 1000);
        
        //create emitter
        emitter = {
            ppm = config.ppm,
            width = config.width,
            height = config.height
            x = config.x,
            y = config.y,
            limit = config.limit
        }
        
        //create resources
        resources = [];
        foreach( r in config.resources )
        {
            local img = config.surface.add_image( r, -1, -1, 1, 1 );
            img.x = -img.texture_width;
            img.y = -img.texture_height;
            img.width = img.texture_width;
            img.height = img.texture_height;
            resources.append( img );
        }
        
        //setup particles
        //particles = [];
        //temporarily put a limit so we can fill the array ahead of time
        timePerParticle = (60 / config.ppm.tofloat()) * 1000;
        local MAX_PARTICLES = 1000;
        particles = array(MAX_PARTICLES);
        for (local i = 0; i < MAX_PARTICLES; i++) {
            local resource = randomResource();
            particles[i] = Particle(0, resource, emitter, config);
            particles[i].visible(false);
        }
        
        //used for debugging
        if (debug) setupDebug(config);
        
    }
    
    function setupDebug(config) {
        debug_emitter = config.surface.add_image( BASE_PATH + "pixel.png", -1, -1, 1, 1);
        //debug_emitter = config.surface.add_clone(debug_bounds);
        debug_emitter.set_rgb(255, 100, 0);
        debug_emitter.x = emitter.x - 3;
        debug_emitter.y = emitter.y - 3;
        debug_emitter.width = emitter.width;
        debug_emitter.height = emitter.height;
        debug_emitter.alpha = 125;
        
        debug_angle_min = config.surface.add_clone(debug_emitter);
        debug_angle_min.set_rgb(255, 255, 0);
        //debug_angle_min.x = emitter.x;
        //debug_angle_min.y = emitter.y;
        debug_angle_min.x = ( emitter.width != 0 ) ? emitter.x + ( emitter.width / 2 ) : 0;
        debug_angle_min.y = ( emitter.height !=0 ) ? emitter.y + ( emitter.height / 2 ) : 0;
        debug_angle_min.width = 100;
        debug_angle_min.height = 2;
        debug_angle_min.rotation = config.angle[0];
        debug_angle_min.alpha = 255;

        debug_angle_max = config.surface.add_clone(debug_angle_min);
        debug_angle_max.set_rgb(255, 0, 0);
        debug_angle_max.rotation = config.angle[1];
        
        debug_bounds = config.surface.add_image( BASE_PATH + "pixel.png", -1, -1, 1, 1);
        debug_bounds.set_rgb(255, 100, 0);
        debug_bounds.x = config.bound[0];
        debug_bounds.y = config.bound[1];
        debug_bounds.width = config.bound[2] - config.bound[0];
        debug_bounds.height = config.bound[3] - config.bound[1];
        debug_bounds.alpha = 200;
        
    }
    
    function random(minNum, maxNum) {
        return floor(((rand() % 1000 ) / 1000.0) * (maxNum - (minNum - 1)) + minNum);
    }
    function randomf(minNum, maxNum) {
        return (((rand() % 1000 ) / 1000.0) * (maxNum - minNum) + minNum).tofloat();
    }
    
    function randomResource() {
        return resources[random(0, resources.len() - 1)];
    }

    //if a value is less then min it will be min and if greater than max it will be max
    function minmax(value, min, max) {
        if (value < min) value = min;
        if (value > max) value = max;
        return value;
    }

    function angle(angle, radius, originX, originY) {
        return [ (radius * cos(angle.tofloat() * PI / 180)).tofloat() + originX,
                 (radius * sin(angle.tofloat() * PI / 180)).tofloat() + originY
               ];
    }

    function onStart() {}
    function onStop() {}
    
    function onUpdate()
    {
        elapsed += time - lastUpdate;
        lastUpdate = time;
        
        //check if we setup a new particle
        //::print( "time: " + time + " elapsed: " + elapsed + "\n" );
        if ( particles.len() == 0 || elapsed >= timePerParticle )
        {
            //::print( "  new particle\n" );
            if ( count == particles.len() - 1 ) count = 0;
            particles[count].visible(true);
            particles[count].alive = true;
            particles[count].createdAt = time;
            count += 1;
            elapsed = elapsed - timePerParticle;
        }

        local msg = "";
        //update existing particles
        for (local i = 0; i < particles.len(); i++) {
            if (particles[i].alive) {
                //update
                particles[i].update(time);
                //kill dead ones
                if (particles[i].isDead()) {
                    particles[i].visible(false);
                    //particles.remove(i);
                    //fe.obj[#].remove
                }
                //give us some debug info
                if (i > particles.len() - 4) {
                    msg += "p" + i + " " + particles[i].toString();
                }
            }
        }
    }
    
    function tostring() { return "ParticleAnimation"; }
}

class Particle {
    alive = false;
    createdAt = 0;
    resource = null;
    x = 0;
    y = 0;
    w = 0;
    h = 0;
    startx = 0;
    starty = 0;
    movement = false;
    speed = null;
    angle = 0;
    scale = 1.0;
    startScale = [ 1, 1 ];
    rotate = [ 0, 0 ];
    rotateToAngle = false;
    fade = 0;
    accel = 0;
    gravity = 0;
    bound = null;
    xOscillate = null;

    //not fully implmented
    lifespan = 0;
    lifetime = 0;
    
    //other variables
    anglePoint = null;      //one-time store an angle point to the radius calculated before doing updates
    currentScale = 1.0;     //store the current scale
    currentRotation = 0;    //store the current rotation    
    currentFade = 0;        //store the current fade alpha
    currentAccel = 0;       //store the current acceleration
    currentGravity = 0;     //store the gravity
    currentSpeed = 0;       //store the current speed
    constructor(createdAt, resource, emitter, config) {
        this.createdAt = createdAt;
        this.resource = config.surface.add_clone(resource);

        this.x = this.startx = ParticleAnimation.random(emitter.x, emitter.x + emitter.width);
        this.y = this.starty = ParticleAnimation.random(emitter.y, emitter.y + emitter.height);
        this.w = resource.width;
        this.h = resource.height;
        
        this.movement = config.movement;
        this.lifespan = this.lifetime = config.lifespan;
        this.speed = ParticleAnimation.random(config.speed[0], config.speed[1]);
        this.angle = ParticleAnimation.random(config.angle[0], config.angle[1]);
        anglePoint = ParticleAnimation.angle(angle, 300, startx, starty);
        this.scale = config.scale;
        this.startScale = ParticleAnimation.randomf(config.startScale[0], config.startScale[1]);
        this.rotate = ParticleAnimation.random(config.rotate[0], config.rotate[1]);
        this.rotateToAngle = config.rotateToAngle;
        this.fade = config.fade;
        this.gravity = config.gravity;
        this.accel = config.accel;
        this.bound = config.bound;
        this.xOscillate = config.xOscillate;
    }
    
    function isDead() { if (lifespan <= 0) return true; return false; }
    
    function update(ttime) {
        ttime  = ttime - createdAt;
        lifespan = lifetime - (ttime - createdAt);
        
        //if (collides()) ExtendedDebugger.notice("collision!");
        
        //the ttime/ numbers below are adjustments to attempt to match the speed of HyperTheme
        if (movement) {
            //gravity
            if (gravity != 0) {
                local gBase = 9.78;
                //local gVariation = (gravity / 75.0);
                local gVariation = gravity.tofloat();
                local gAccel = (ttime / 2000.0) * (gBase + gVariation);
                currentGravity = pow(gAccel, 2);
                if (gVariation < 0) currentGravity = -currentGravity;
            }
            
            //speed and acceleration
            currentAccel = (ttime.tofloat() / 1000.0) * accel;
            currentSpeed = speed * (1 + currentAccel);

            local dist = ((ttime.tofloat() / 1200.0) * currentSpeed);
            //local dist = ((ttime.tofloat() / 1000.0) * speed);
            local ang = [ (anglePoint[0] - startx) / 300.0, (anglePoint[1] - starty) / 300.0 ];
            resource.x = startx + dist * ang[0];
            resource.y = starty + dist * ang[1] + currentGravity;
            
            //xOscillate
            if (xOscillate[0] > 0 && xOscillate[1] > 0) {
                local amp = xOscillate[0].tofloat();
                local freq = xOscillate[1].tofloat();
                //resource.x += sin((ttime.tofloat() / freq)) * amp;
                resource.x += sin((ttime.tofloat() / 7000.0)) * 500;
            }
        } else {
            resource.x = startx;
            resource.y = starty;
        }

        //fade
        if (fade > 0 && currentFade >= 0) {
            currentFade = 255 - (ttime / fade.tofloat()) * 255;
            if (currentFade < 0) currentFade = 0;
            resource.alpha = currentFade;
        }
        
        //scale
        if (scale[0] != 1 || scale[1] != 1) {
            //scale (scale over time)
            //change * (time / duration) + start;
            currentScale = (scale[1] - scale[0]) * (ttime / lifetime.tofloat()) + scale[0];
        } else {
            //startScale (random scale)
            currentScale = startScale;
        }

        resource.width = w * currentScale;
        resource.height = h * currentScale;

        //rotate
        if (rotateToAngle) {
            resource.rotation = angle;
        } else {
            currentRotation = (rotate * (ttime.tofloat() / 1000.0)) * 10;
            resource.rotation = currentRotation;
        }
        //center on point
        //how to center on scale and rotation??
        resource.x -= resource.width / 2;
        resource.y -= resource.height / 2;
        
        //old formulas
        //resource.x = (ttime.tofloat() / abs(speed - 2050)).tofloat() * (anglePoint[0] - startx) + startx;
        //resource.y = (ttime.tofloat() / abs(speed - 2050)).tofloat() * (anglePoint[1] - starty) + starty;
        //resource.x = ParticleAnimation.calculate("in", "linear", ttime, startx, angle[0], 2000);
        //resource.y = ParticleAnimation.calculate("in", "linear", ttime, starty, angle[1], 2000);
    }
    
    function collides() {
        if (abs(x - bound[0]) * 2 < resource.width + bound[2] && abs(y - bound[1]) * 2 < resource.height + bound[3]) {
            return true;
        }
        //with rotation
        /*
        local pWidth = sqrt(resource.width * resource.width + resource.height * resource.height) * cos(angle);
        local bWidth = sqrt(bound[2] * bound[2] + bound[3] * bound[3]);
        if (abs(x - bound[0]) < (abs(pWidth + bWidth) / 2) && (abs(y - bound[1]) < (abs(resource.height + bound[3]) / 2))) {
            //collide
            return true;
        }
        */
        return false;
    }
    
    function setAlpha(a) { resource.alpha = ParticleAnimation.minmax(a, 0, 255); }
    function setColor(r, g, b) { resource.set_rgb(ParticleAnimation.minmax(r, 0, 255), ParticleAnimation.minmax(g, 0, 255), ParticleAnimation.minmax(b, 0, 255)); }
    function visible(v) { resource.visible = v; }
    function toString() { return ": " + x + "," + y + " a=" + angle + " sp: " + currentSpeed + " sca: " + currentScale + " rot: " + currentRotation + " fa: " + currentFade + " gr: " + currentGravity + " ac: " + currentAccel + "\n"; }
}

//load particle presets
fe.load_module("animate/particles/presets");
