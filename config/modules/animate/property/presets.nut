/* PRESETS */
PropertyAnimations <- {
    "hide": { property = "alpha", start = 255, end = 0, when = When.ToNewSelection, wait = 250, time = 250 },
    "show": { property = "alpha", start = 0, end = 255, when = When.FromOldSelection, time = 2000 },
    "enlarge": { property = "scale", start = 1.0, end = 2.0, when = When.ToNewSelection, time = 1500, tween = Tween.Quad },
    "enlarge_fun": { property = "scale", start = 1.0, end = 2.0, when = When.ToNewSelection, time = 1500, tween = Tween.Bounce },
    "shrink": { property = "scale", start = 2.0, end = 1.0, when = When.ToNewSelection, time = 1500, tween = Tween.Bounce },
    "slide_left": { property = "x", end = "-25", when = When.OnPrevious, time = 1500, tween = Tween.Quad },
    "slide_right": { property = "x", end = "+25", when = When.OnNext, time = 1500, tween = Tween.Quad },
    "rise": { property = "position", end = { x = 0, y = "-25" }, when = When.ToNewSelection, tween = Tween.Elastic, time = 1200, delay = 750 },
    "fall": { property = "position", end = { x = 0, y = "+25" }, when = When.ToNewSelection, tween = Tween.Elastic, time = 1200, delay = 750 },
    "rotate_left_10": { property = "rotation", start = 0, end = -10, when = When.ToNewSelection, time = 1500 },
    "rotate_right_10": { property = "rotation", start = 0, end = 10, when = When.ToNewSelection, time = 1500 },
    "rotate_left_45": { property = "rotation", start = 0, end = -45, when = When.ToNewSelection, time = 1500 },
    "rotate_right_45": { property = "rotation", start = 0, end = 45, when = When.ToNewSelection, time = 1500 },
    "rotate_left_90": { property = "rotation", start = 0, end = -90, when = When.ToNewSelection, time = 1500 },
    "rotate_right_90": { property = "rotation", start = 0, end = 90, when = When.ToNewSelection, time = 1500 },
}