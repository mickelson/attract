ExtendedObjects.add_orbit <- function(id, a, x, y, w, h) {
    return ExtendedObjects.add(Orbit(id, a, x, y, w, h));
}

const MWIDTH = 280;
const MHEIGHT = 170;
const SPIN_MS = 2000;
function get_y(x) { return ( 200 + sqrt( pow( 270, 2 ) - pow( x - 400, 2 ) ) ); }
function set_bright(x, o) { o.set_rgb( x, x, x ); }

class Orbit extends ExtendedObject {
    marquees = [];
    last_move = 0;
    shader = null;
    constructor(id, art, x, y, w, h) {
        base.constructor(id, x, y);
        //workaround
        object = fe.add_text("", x, y, w, h);
        if ("artwork" in config == false) config.artwork <- art;
        if ("enable_bloom" in config == false) config.enable_bloom <- false;
        
        shader = fe.add_shader(Shader.Empty);
        if (config.enable_bloom) {
            shader = fe.add_shader(Shader.Fragment, "extended/objects/orbit/bloom_shader.frag");
            shader.set_texture_param("bgl_RenderedTexture");
        }

        //io, xl, xm, xr, sl, sm, sr)
        //  x left, mid, right   scale left, mid, right
        //index offset, x?, x?, x?, s?, s?, s?
        local xm = w / (MWIDTH / 2);
        local xl = x;
        local xr = w - (MWIDTH / 2);
        local sm = 1.0;
        marquees.append(Marquee(config.artwork, -2, 200, 150, 145, 0.7, 0.4, 0.1 )); 
        marquees.append(Marquee(config.artwork, -1, 400, 200, 150, 1.0, 0.7, 0.4 ));
        marquees.append(Marquee(config.artwork,  2, 655, 650, 600, 0.1, 0.4, 0.7 ));
        /*
        marquees.append(Marquee(config.artwork, -2, 200, 150, 145, 0.7, 0.4, 0.1 )); 
        marquees.append(Marquee(config.artwork, -1, 400, 200, 150, 1.0, 0.7, 0.4 ));
        marquees.append(Marquee(config.artwork,  2, 655, 650, 600, 0.1, 0.4, 0.7 ));
        */
        // Delayed creation of these two so that they are drawn over top of the others
        marquees.append(Marquee(config.artwork, 1, 650, 600, 400, 0.4, 0.7, 1.0 ) );
        // This is the marquee for the current selection
        marquees.append(Marquee(config.artwork, 0, 600, 400, 200, 0.7, 1.0, 0.7 ) );
        marquees[2].ob.shader = shader;

        ExtendedObjects.add_callback(this, "onTick");
        ExtendedObjects.add_callback(this, "onTransition");
    }
       
    function onTick(params) {
        /*
        local ttime = params.ttime;
        local block = ttime / 30000;
        if ( block % 2 )
            set_bright( ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
        else
            set_bright( 255 - ( ( ttime % 30000 ) / 30000.0 ) * 255, frame );
            */
    }
    
    function onTransition(params) {
        local ttype = params.ttype;
        local var = params.var;
        local ttime = params.ttime;
        switch ( ttype ) {
            case Transition.ToNewSelection:
                if ( ttime < SPIN_MS )
                {
                    marquees[2].ob.shader = shader;
                    local moves = abs( var );
                    local jump_adjust = 0;
                    if ( moves > marquees.len() )
                    {
                        jump_adjust = moves - marquees.len();
                        moves = marquees.len();
                    }

                    local move_duration = SPIN_MS / moves;
                    local curr_move = ttime / move_duration;

                    local change_index=false;
                    if ( curr_move > last_move )
                    {
                        last_move=curr_move;
                        change_index=true;
                    }

                    local progress = ( ttime % move_duration ).tofloat() / move_duration;

                    if ( var < 0 )
                    {

                        if ( change_index )
                        {
                            // marquees[marquees.len()-1].ob will get swapped through to the leftmost position
                            marquees[marquees.len()-1].ob.index_offset = marquees[0].base_io - curr_move - jump_adjust;
                            for ( local i=marquees.len()-1; i>0; i-=1 )
                            {
                                marquees[i].swap_art( marquees[i-1] );
                                marquees[i].reset();
                            }
                        }

                        foreach ( m in marquees )
                            m.move_left( progress );
                    }
                    else
                    {
                        if ( change_index )
                        {
                            // marquees[0].ob will get swapped through to the rightmost position
                            marquees[0].ob.index_offset = marquees[marquees.len()-1].base_io + curr_move + jump_adjust;
                            for ( local i=0; i<marquees.len()-1; i+=1 )
                            {
                                marquees[i].swap_art( marquees[i+1] );
                                marquees[i].reset();
                            }
                        }
                        foreach ( m in marquees )
                            m.move_right( progress );
                    }
                    return true;
                }

                foreach ( m in marquees )
                {
                    m.ob = m.orig_ob;
                    m.reset();
                    m.ob.index_offset = m.base_io;
                }
                marquees[2].ob.shader = yes_shader;
                last_move=0;
                break;

            case Transition.StartLayout:
            case Transition.FromGame:
                if ( ttime < 255 )
                {
                    foreach (o in fe.obj)
                        o.alpha = ttime;

                    return true;
                }
                else
                {
                    foreach (o in fe.obj)
                        o.alpha = 255;
                }
                break;

            case Transition.EndLayout:
            case Transition.ToGame:
                if ( ttime < 255 )
                {
                    foreach (o in fe.obj)
                        o.alpha = 255 - ttime;

                    return true;
                }
                break;
        }
        return false;
    }
}

class Marquee {
	ob=null; 
	orig_ob=null;
	base_io=0;
	xl=0; xm=0; xr=0; 
	sl=0.0; sm=0.0; sr=0.0;

	constructor( art, pio, pxl, pxm, pxr, psl, psm, psr ) {
		xl=pxl; xm=pxm; xr=pxr; sl=psl; sm=psm; sr=psr;
		orig_ob = ob = fe.add_artwork( art );
		ob.preserve_aspect_ratio=true;
		ob.movie_enabled = false;
		ob.index_offset = base_io = pio;
		reset();
	}

	function move_left( p ) {
		local scale = ( sm - ( sm - sl ) * p );
		local nx = xm - ( xm - xl ) * p;

		ob.width = MWIDTH * scale;
		ob.height = MHEIGHT * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function move_right( p ) {
		local scale = ( sm - ( sm - sr ) * p );
		local nx = xm + ( xr - xm ) * p;

		ob.width = MWIDTH * scale;
		ob.height = MHEIGHT * scale;
		ob.x = nx - ob.width / 2;
		ob.y = get_y( nx ) - ob.height / 2;
		set_bright( scale * 255, ob );
	}

	function reset() {
		ob.width = MWIDTH * sm;
		ob.height = MHEIGHT * sm;
		ob.x = xm - ob.width / 2;
		ob.y = get_y( xm ) - ob.height / 2;
		set_bright( sm * 255, ob );
	}
	function swap_art( o ) {
		local temp = o.ob;
		o.ob = ob;
		ob = temp;
	}
}