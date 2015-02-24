ExtendedObjects.add_wheel <- function(id, s, a, x, y, w, h) {
    return ExtendedObjects.add(Wheel(id, s, a, x, y, w, h));
}

class Wheel extends ExtendedObject {
    objects = null;
    constructor(id, slots, artwork, x, y, w, h) {
        base.constructor(id, x, y);
        objects = {};
        
        //workaround
        object = fe.add_text("", x, y, w, h);
        
        ExtendedObjects.add_callback(this, "onAnimationStop");

        local aspect = (ScreenWidth / ScreenHeight);
        config.slots <- slots;
        config.selected <- 4;
        config.artwork <- artwork;
        config.spacing <- 5;
        config.orientation <- "horizontal";
        config.offset <- 0;
        if (config.orientation == "vertical") {
            config.slotHeight <- (h - (slots - 1 * config.spacing))/slots;
            config.slotWidth <- config.slotHeight * aspect;
        } else {
            config.slotWidth <- (w - (slots - 1 * config.spacing))/slots;
            config.slotHeight <- config.slotWidth / aspect;
        }
        local index = 0;
        for (local i = 0; i < config.slots; i++) {
            index = (i + 1) - config.selected;
            local obj = null;
            if (config.orientation == "vertical") {
                //obj = fe.add_artwork(config.artwork, x, y + (i * config.slotHeight) + (i * config.spacing), config.slotWidth, config.slotHeight);
                obj.config.offset = (i * config.slotHeight) + (i * config.spacing);
                obj = ExtendedObjects.add_artwork("wheel" + i, config.artwork, x, y + obj.config.offset, config.slotWidth, config.slotHeight);
            } else {
                //obj = fe.add_artwork(config.artwork, x + (i * config.slotWidth) + (i * config.spacing), y, config.slotWidth, config.slotHeight);
                config.offset = (i * config.slotWidth) + (i * config.spacing);
                obj = ExtendedObjects.add_artwork("wheel" + i, config.artwork, x + config.offset, y, config.slotWidth, config.slotHeight);
                if (i == 0 || i == config.slots - 1) {
                    //first and last objects
                    obj.add_animation({ when = Transition.ToNewSelection, property = "alpha", from = 255, to = 100 } );
                    obj.add_animation({ when = Transition.FromOldSelection, property = "alpha", from = 100, to = 255 } );
                }
            }
            obj.setPreserveAspectRatio(true);
            obj.setIndexOffset(index);
            //obj.preserve_aspect_ratio = true;
            //obj.index_offset = index;
            
            objects["wheel" + i] <- obj;
        }
    }
    
    function setX(x) {
        foreach (o in objects) {
            o.setX(x + o.config.offset);
        }
    }
    function setY(y) {
        foreach (o in objects) {
            o.setY(y + o.config.offset);
        }
    }
    
    function onAnimationStop(params) {
        //update index reshow first and last?
        //local obj = params.object;
        //if (obj.getId() == "wheel0" && params.animation.config.when == Transition.ToNewSelection) obj.setPosition([ 50, 250 ]);
        //if (obj.getId() == "wheel0" && params.animation.config.when == Transition.ToNewSelection) obj.setPosition([ 50, 450 ]);
    }
}