//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.tutorial1.msg = "10. ParticleAnimation: Custom Config";
OBJECTS.tutorial2.msg = "ParticleAnimations use an emitter to emit particles and use a customizable config\nThe emitter is visible here showing the x/y/width/height (orange box) where the particles emit and the min and max angles (red/yellow lines) they emit to\nYou can adjust the emitter location and angles, particle images (resources), size and speed at start or over time, gravity and lifespan - more coming!";

OBJECTS.snap.visible = false;

///////////////////
//  Particle Sample
///////////////////

//enable debug to see the emitter
ParticleAnimation.debug <- true;

local particle_cfg = {
    resources = [ "resources/mario.png", "resources/link.png" ],    //file resources - comma separated
    ppm = 30,               //how many particles per minute will be created
    x = 50,                  //the x location of the particle emitter
    y = 100,                  //the y location of the particle emitter
    width = 100,           //the width of the particle emitter
    height = 360,             //the height of the particle emitter
    speed = [ 100, 250 ],   //randomizes the speed of the resource when it is first created
    angle = [ 0, 15 ],     //the angle where particles will be emitted (min, max)
    startScale = [ 1, 1 ],  //randomizes the scale of the resource when it is first created
    gravity = 0,            //gravity attached to the particle
    fade = 0,           //fade the alpha over time
    lifespan = 10000,       //how long the resource will 'live'
    scale = [ 0.5, 0.5 ],     //scale the resource over its lifespan
    //accel = 3.5,                  * not yet working
    //bound = [ 0, 0, 3000, 650 ],  * not yet working
    //rotate = [ 3, 3 ],            * not yet working
    //rotateToAngle = false,        * not yet working
    //xOscillate = [ 10, 250 ]      * not yet working
}

animation.add( ParticleAnimation( particle_cfg ) );
