//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");

OBJECTS.tutorial1.msg = "11. ParticleAnimation: Presets";
OBJECTS.tutorial2.msg = "Some builtin presets for Particles can be used";

//Some PRESETS are provided for you to use as well
animation.add( ParticleAnimation( Particles.bubbles1 ) );
//animation.add( ParticleAnimation( Particles.snow ) );
//animation.add( ParticleAnimation( Particles.sparkle ) );