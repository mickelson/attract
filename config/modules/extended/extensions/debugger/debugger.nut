ExtendedObjects["debugger"] <- function () {
    return ExtendedDebugger();
}

class ExtendedDebugger {
    outline_objects = [];
    text_objects = [];
    config = {
        "max_lines": 3,
        "align": Align.Left,
        "alpha": 255,
        "bg": [40, 40, 40],
        "bg_alpha": 150,
        "rgb": [255, 255, 0],
        "charsize": 14,
        "word_wrap": false
    }
    constructor() {
        local d = FeConfigDirectory + "modules/extended/extensions/debugger/";
        foreach(o in ExtendedObjects.objects) {
            //we're creating text objects for each object that exists in the layout
            local txt = ExtendedText("debug_" + o.id, o.toString(), o.getX(), o.getY() + o.getHeight() - 25, o.getWidth(), 25, Layer.Foreground);
            setDefaults(txt);
            text_objects.append(txt);
            local outline = ExtendedImage("debug_outline_" + o.id, d + "outline.png", o.getX(), o.getY(), o.getWidth(), o.getHeight(), Layer.Foreground);
            outline_objects.append(outline);
        }
        
        local notice = ExtendedText("debug_notice", "debug_notice", 0, 0, fe.layout.width, 20, Layer.Foreground);
        ExtendedObjects.add_callback(this, "onObjectAdded");
        ExtendedObjects.add_callback(this, "onTransition");
        ExtendedObjects.add_callback(this, "onTick");
        ExtendedObjects.add_callback(this, "onAnimationFrame");
        ExtendedObjects.add_callback(this, "onAnimationStart");
        ExtendedObjects.add_callback(this, "onAnimationStop");
        text_objects.append(notice);
        setDefaults(notice);
    }
    
    function onObjectAdded(params) {
        ExtendedDebugger.notice("object added: " + params.object.id);
    }
    function onAnimationFrame(params) {
        ExtendedDebugger.notice("animating: " + params.object.id);        
    }
    
    function onAnimationStart(params) {
        ExtendedDebugger.notice("animation started: " + params.object.id);
    }
    
    function onAnimationStop(params) {
        ExtendedDebugger.notice("animation stopped: " + params.object.id);
    }
    
    function onTick(params) {
        ExtendedDebugger.notice(params.ttime);
        updateObjects();
    }
    
    function updateObjects() {
        foreach(o in ExtendedObjects.objects) {
            local debugText = get_text(o.id);
            if (debugText != null) {
                debugText.setPosition( [ o.getX(), o.getY() + o.getHeight() - 25 ]);
                debugText.setText(o.toString());
            }
            local debugOutline = get_outline(o.id);
            if (debugOutline != null) {
                debugOutline.setPosition( [ o.getX(), o.getY() ]);
            }
        }
    }
    
    function onTransition(params) {
        local ttype = params.ttype;
        local var = params.var;
        local ttime = params.ttime;
        //ExtendedDebugger.notice("Transition: " + Animate.getWhen(ttype));
        updateObjects();
        if (ttype == Transition.StartLayout) {
            local msg = "Found " + ExtendedObjects.objects.len() + " objects: ";
            foreach(o in ExtendedObjects.objects) {
                msg += o.id + "-";
            }
            notice(msg);
        }
        return false;
    }
    
    function setDefaults(obj) {
        obj.setAlign(config.align);
        obj.setAlpha(config.alpha);
        obj.setColor(config.rgb[0], config.rgb[1], config.rgb[2]);
        obj.setBGColor(config.bg[0], config.bg[1], config.bg[2]);
        obj.setBGAlpha(config.bg_alpha);
        obj.setCharSize(config.charsize);
        obj.setWordWrap(config.word_wrap);
        obj.setShadow(false);
    }
    
    function notice(text) {
        local notice = get_text("notice");
        if (notice != null) {
            notice.setText(text + "\n");
        }
    }
    
    function get_text(id) { foreach (o in text_objects) { if ("debug_" + id == o.id) return o; } return null; }
    function get_outline(id) { foreach (o in outline_objects) { if ("debug_outline_" + id == o.id) return o; } return null; }
    function setVisible(v) { foreach(o in text_objects) o.setVisible(v); foreach(o in outline_objects) o.setVisible(v); }
}
