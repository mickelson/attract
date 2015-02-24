ExtendedObjects.add_warp <- function(id, x, y, w, h) {
    return ExtendedObjects.add(Warp(id, x, y, w, h));
}
const n = 256;

class Warp extends ExtendedObject {
    e_speed = 4;
    x = null;
    y = null;
    z = null;
    star_ratio = 256;
    star_colour_ratio = null;
    star = array(n);
    mouse_x = null;
    mouse_y = null;
    base_speed = null;
    star_speed = null;
    resource = fe.add_image("extended/objects/warp/dot.png", -1, -1, 1, 1);
    constructor(id, x, y, w, h) {
        base.constructor(id, x, y);
        //hack
        object = fe.add_text("", x, y, w, h);
        init(x, y, w, h);
        ExtendedObjects.add_callback(this, "onTick");
        ExtendedObjects.add_callback(this, "onTransition");
    }
    
    function init(x, y, w, h) {
        this.x = x + (w / 2);
        this.y = y + (h / 2);
        this.z = (w + h) / 2;
        mouse_x = this.x;
        mouse_y = this.y;
        star_colour_ratio = 1.0/z;
        base_speed = e_speed.tofloat(); 
        if ( abs( base_speed ) > 25 ) // speed limits...
            base_speed=25.0;
            star_speed = base_speed;    
        for(local i=0;i<n;i++)
        {
            star[i]=array(7);
            star[i][0]=my_rand()*getWidth()*2-this.x*2;
            star[i][1]=my_rand()*getHeight()*2-this.y*2;
            star[i][2]=my_round( my_rand()*z );
            star[i][3]=0;
            star[i][4]=0;
            star[i][5]=fe.add_clone( resource );
            star[i][6]=fe.add_clone( resource );

            // make 10% of the stars a bit bigger
            if ( rand() % 10 == 0 )
            {
                star[i][5].width = star[i][5].height=2;
                star[i][6].width = star[i][6].height=2;
            }
        }
    }
    
    function onTick(params) {
        anim();
    }

    function onTransition(params) {
        anim();
        return false;
    }
    
    function my_round( num )
    {
        return ( num + 0.5 ).tointeger();
    }

    function my_rand()
    {
        return ( rand() % 1000 ) / 1000.0;
    }

    function my_set_colour( star, val )
    {
        local temp = val * 255;
        if ( temp > 255 )
            temp = 255;

        star.set_rgb( temp, temp, temp );
    }

    function anim()
    {
        for(local i=0;i<n;i++)
        {
            local test=true;
            local star_x_save=star[i][3];
            local star_y_save=star[i][4];

            star[i][0]+=mouse_x>>4;

            if(star[i][0] > x <<1)
            {
                star[i][0]-=(getX() + getWidth()).tointeger()<<1;
                test=false;
            }
            if(star[i][0] < -x<<1)
            {
                star[i][0]+=(getX() + getWidth()).tointeger()<<1;
                test=false;
            }

            star[i][1]+=mouse_y>>4;

            if(star[i][1] > y<<1)
            {
                star[i][1]-=(getX() + getHeight()).tointeger()<<1;
                test=false;
            }
            if(star[i][1] < -y<<1)
            {
                star[i][1]+=(getX() + getHeight()).tointeger()<<1;
                test=false;
            }

            star[i][2]-=star_speed;

            if(star[i][2] > z)
            {
                star[i][2]-=z;
                test=false;
            }
            if(star[i][2] < 0)
            {
                star[i][2]+=z;
                test=false;
            }

            star[i][3]=x+(star[i][0]/star[i][2])*star_ratio;
            star[i][4]=y+(star[i][1]/star[i][2])*star_ratio;

            local temp = (1-star_colour_ratio*star[i][2])*2;
            my_set_colour( star[i][5], temp );

            if(star_x_save>0
                &&star_x_save<getWidth()
                &&star_y_save>0
                &&star_y_save<getHeight()
                            &&test)
            {
                star[i][6].visible = true;

                my_set_colour( star[i][6], temp );

                star[i][5].x = star[i][3];
                star[i][5].y = star[i][4];

                star[i][6].x = star_x_save;
                star[i][6].y = star_y_save;
            }
            else
            {
                star[i][5].x = star[i][3];
                star[i][5].y = star[i][4];
                star[i][6].visible = false;
            }
        }
    }

}
