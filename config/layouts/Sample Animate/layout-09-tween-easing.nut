//NOTE: A shared resource .nut file is used for these examples
// A typical animated layout will specify fe.load_module("animate"); at the top
// All objects used in these examples are stored in an OBJECTS table, which is created in the shared .nut file
fe.do_nut("resources/shared.nut");
    
OBJECTS.logo.visible = true;

OBJECTS.tutorial1.msg = "09. Tween and Easing Animations";
OBJECTS.tutorial2.msg = "Animations can have different tween effects: Tween.Linear, Tween.Bounce, Tween.Elastic, Tween.Back, Tween.Quad, Tween.Expo\nAnimations can have different easings: Easing.Out, Easing.In, Easing.OutIn and Easing.InOut\nCombining them creates a unique animation effect for on-screen and off-screen animations";

OBJECTS.logo.x = 400;
OBJECTS.logo.y = 150;

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Linear, time = 3000 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = OBJECTS.logo.y, end = -180, tween = Tween.Back, easing = Easing.Out, time = 3000, delay = 3500 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Bounce, time = 3000, delay = 7000 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = OBJECTS.logo.y, end = -180, tween = Tween.Circle, easing = Easing.Out, time = 3000, delay = 10500 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Elastic, time = 3000, delay = 14000 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = OBJECTS.logo.y, end = -180, tween = Tween.Expo, easing = Easing.OutIn, time = 3000, delay = 17500 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "y", start = -180, end = OBJECTS.logo.y, tween = Tween.Sine, easing = Easing.InOut, time = 3000, delay = 21000 } ) );

animation.add( PropertyAnimation( OBJECTS.logo, { property = "alpha", start = 255, end = 50, time = 5000, delay = 25500 } ) );
