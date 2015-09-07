/* Sprite Presets */

SpriteAnimations <- {
    "joystick_left": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 1, 0, 1 ],
        loop = true
    },
    "joystick_right": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 2, 0, 2 ],
        loop = true
    },
    "joystick_up": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 3, 0, 3 ],
        loop = true
    },
    "joystick_down": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 4, 0, 4 ],
        loop = true
    },
    "joystick_leftright": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 1, 0, 2, 0, 1, 0, 2 ],
        loop = true
    },
    "joystick_updown": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 3, 0, 4, 0, 3, 0, 4 ],
        loop = true
    },
    "joystick_updownleftright": {
        resource = SpriteAnimation.BASE_PATH + "images/joystick-move.png",
        when = When.Always,
        width = 128,
        frame = 0,
        time = 3000,
        order = [ 0, 3, 0, 4, 0, 1, 0, 2 ],
        loop = true
    },
    "button_red": {
        resource = SpriteAnimation.BASE_PATH + "images/button-press-red.png",
        when = When.Always,
        width = 128,
        frame = 0,
        order = [ 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 ],
        time = 1500,
        loop = true,
        delay = 500
    },
    "button_white": {
        resource = SpriteAnimation.BASE_PATH + "images/button-press-white.png",
        when = When.Always,
        width = 128,
        frame = 0,
        order = [ 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 ],
        time = 1500,
        loop = true,
        delay = 500
    },
    "pacland": {
        resource = SpriteAnimation.BASE_PATH + "images/pacland.gif",
        when = When.ToNewSelection,
        width = 32,
        height = 48,
        frame = 0,
        time = 1000,
        loop = 2,
        onStop = function( anim )
        {
            anim.frame(0);
        }
    }
}