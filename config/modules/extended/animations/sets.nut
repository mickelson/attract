AnimationSet["left_center_up"] <-  [
    {
        which = "translate",
        kind = "loop",
        when = When.ToNewSelection,
        duration = 500,
        from = "offleft",
        to = "center",
        easing = "out",
        tween = "back"
    },
    {
        which = "translate",
        kind = "loop",
        when = When.FromOldSelection,
        duration = 500,
        from = "current",
        to = "top",
        easing = "out",
        tween = "bounce"
    }
];

AnimationSet["fade_in_out"] <-  [
    { 
        which = "property",
        when = When.ToNewSelection,
        wait = true,
        duration = 250,
        property = "alpha",
        from = 255,
        to = 0,
        easing = "out",
        tween = "quad"
    },
    { 
        which = "property",
        when = When.FromOldSelection,
        delay = 1000,
        duration = 500,
        property = "alpha",
        from = 0,
        to = 255,
        easing = "out",
        tween = "quad"
    }
];

AnimationSet["hover"] <-  [
        {
            which = "property",
            when = When.Always,
            wait = false,
            property = "x",
            kind = "yoyo",
            from = "left",
            duration = 8000,
            from = "left",
            to = "center",
            easing = "out",
            tween = "quad"
        },
        {
            which = "property",
            when = When.Always,
            wait = false,
            property = "y",
            kind = "yoyo",
            duration = 10000,
            from = "top",
            to = "bottom",
            easing = "out",
            tween = "quad",
        }
];
