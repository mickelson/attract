// Layout by cools / Arcade Otaku
// For use in a 640x240 resolution
// We use this strange resolution to enhance the front rendering quality in 15KHz
//
// Layout User Options
class UserConfig {
 </label="Background Image",help="Choose snap/video snap, title, flyer, user image (bg.jpg in layout folder) or no background",options="video,snap,title,flyer,user,none"/>bg_image="video";
 </label="Show Wheel Logo?",help="Enable the wheel logo?",options="Yes,No"/>wheel_logo="Yes";
 </label="Show Category?",help="Enable the category text?",options="Yes,No"/>category_text="Yes";
}
local my_config=fe.get_config();

// Colour cycle code from LukeNukem
cycleVTable <-{
 "cnListHeight":0,"swListHeight":0,"wkListHeight":0,
 "cnListRed":0,"swListRed":0,"wkListRed":0,
 "cnListGreen":0,"swListGreen":0,"wkListGreen":0,
 "cnListBlue":0,"swListBlue":0,"wkListBlue":0,
}
function cycleValue(ttime,counter,switcher,workValue,minV,maxV,BY,speed){
 if (cycleVTable[counter] == 0)
  cycleVTable[counter]=ttime;
////////////// 1000=1 second //////////////
 if (ttime-cycleVTable[counter]>speed){
  if (cycleVTable[switcher]==0){
   cycleVTable[workValue]+=BY;
   if (cycleVTable[workValue]>=maxV)
    cycleVTable[switcher]=1;
  }
  else if (cycleVTable[switcher]==1){
   cycleVTable[workValue]-=BY;
   if (cycleVTable[workValue]<=minV)
    cycleVTable[switcher]=0;
  }    
 cycleVTable[counter]=0;
 }
 return cycleVTable[workValue];	
}

// Gives us a nice high random number for the RGB levels
function brightrand() {
	return 255-(rand()/512);
}

// Layout Constants
fe.layout.orient=RotateScreen.Right;
fe.layout.width=fe.layout.width/2;
local flw=fe.layout.width;
local flh=fe.layout.height;
local rot=(fe.layout.base_rotation+fe.layout.toggle_rotation)%4;
switch (rot) {
 case RotateScreen.Right:
 case RotateScreen.Left:
  fe.layout.width=flh;
  fe.layout.height=flw;
  flw=fe.layout.width;
  flh=fe.layout.height;
  break;
}

// Background
if (my_config["bg_image"]=="video") {
 local bg=fe.add_artwork("snap",0-(flw*0.1),0-(flh*0.1),flw*1.2,flh*1.2);
 bg.preserve_aspect_ratio=false;
 bg.set_rgb(128,128,128);
}
else if (my_config["bg_image"]=="snap") {
 local bg=fe.add_artwork("snap",0-(flw*0.1),0-(flh*0.1),flw*1.2,flh*1.2);
 bg.video_flags=Vid.ImagesOnly;
 bg.preserve_aspect_ratio=false;
 bg.set_rgb(128,128,128);
}
else if (my_config["bg_image"]=="title") {
 local bg=fe.add_artwork("title",0-(flw*0.1),0-(flh*0.1),flw*1.2,flh*1.2);
 bg.preserve_aspect_ratio=false;
 bg.set_rgb(128,128,128);
}
else if (my_config["bg_image"]=="flyer") {
 local bg=fe.add_artwork("flyer", 0, 0, flw, flh);
 bg.preserve_aspect_ratio=false;
 bg.set_rgb(128,128,128);
}
else if (my_config["bg_image"]=="user") {
 local bg=fe.add_image("bg.jpg",0,0,flw,flh);
 bg.preserve_aspect_ratio=false;
 bg.set_rgb(128,128,128);
}

// List
local lbx=0;                    // Listbox X position
local lby=flh*0.4;              // Listbox Y position
local itn=9;                    // Listbox entries
if (my_config["wheel_logo"]=="No"){
 lby=flh*0.17;                  // Listbox Y position without logo
 itn=13;                        // Listbox entries without logo
} 
local lbw=flw;                  // Listbox width
local lbh=(flh-lby-(flh*0.08)); // Listbox height
local ith=lbh/itn;              // Listbox entry height
lby+=(ith*(itn/2));             // Listbox draw point - do not change
local lbr=255;                  // Listbox red
local lbg=255;                  // Listbox green
local lbb=128;                  // Listbox blue
local lbf=0.75;                 // Listbox colour fade. Lower values=less fade.

for (local i=(itn/2)*-1;i<=(itn/2);i+=1){
 if (i==0){
 }
 else { 
  local unselgame=fe.add_text("[Title]",lbx,lby+(ith*i),lbw,ith);
  unselgame.index_offset=i;
  if (i<0) {unselgame.alpha=255-((i*(255/(itn/2))*-1)*lbf);}
  else {unselgame.alpha=255-(i*(255/(itn/2))*lbf);} 
 }
}

// Texts
local fltsh=fe.add_text("<  [FilterName]  >",1,(flh*0.06)+1,flw,flh*0.05);
fltsh.set_rgb(0,0,0);
local filtername=fe.add_text(fltsh.msg,fltsh.x-1,fltsh.y-1,fltsh.width,fltsh.height);
local listtitle=fe.add_text("[ListSize] [DisplayName]",0,flh*0.11,flw,flh*0.05);
local slgsh=fe.add_text("[Title]",lbx+1,lby+1,lbw,ith);
slgsh.set_rgb(0,0,0);
local selgame=fe.add_text(slgsh.msg,slgsh.x-1,slgsh.y-1,slgsh.width,slgsh.height);
local freeplayleft=fe.add_text("Free Play",0,0,flw,flh*0.05);
freeplayleft.align=Align.Left;
local freeplayright=fe.add_text("Free Play",0,0,flw,flh*0.05);
freeplayright.align=Align.Right;
local inssh=fe.add_text("Instructions",1,(flh*0.92)+1,flw,flh*0.04);
inssh.set_rgb(0,0,0);
local instructions=fe.add_text(inssh.msg,inssh.x-1,inssh.y-1,inssh.width,inssh.height);
local help_text=fe.add_text("Play Game [BUTTON1]  -  Random [START]  -  Quit Game [START1+2]",0,flh*0.95,flw,flh*0.04);
if (my_config["category_text"]=="Yes"){
 local category=fe.add_text("[Category]",0-(flw*0.01),flh*0.95,flh*0.85,flh*0.05);
 category.rotation=-90;
 category.align=Align.Left;
 category.alpha=64;
}
local copyright=fe.add_text("©[Year] [Manufacturer]",flw-(flh*0.06),flh*0.95,flh*0.85,flh*0.05);
copyright.rotation=-90;
copyright.align=Align.Left;
copyright.alpha=64;

// Preview image
if (my_config["wheel_logo"]=="Yes"){
 local wheelshadow=fe.add_artwork("wheel",(flw*0.1)+2,(flh*0.18)+2,flw*0.80,flh*0.22);
 wheelshadow.preserve_aspect_ratio=true;
 wheelshadow.set_rgb(0,0,0);
 wheelshadow.alpha=192;
 local wheel=fe.add_clone(wheelshadow);
 wheel.set_rgb(255,255,255);
 wheel.x-=2;
 wheel.y-=2;
 wheel.alpha=255;
} 

function textTickles(ttime){		
 local RED=cycleValue(ttime,"cnListRed","swListRed","wkListRed",100,254,1,1);
 local GREEN=cycleValue(ttime,"cnListGreen","swListGreen","wkListGreen",100,254,1.5,1);
 local BLUE=cycleValue(ttime,"cnListBlue","swListBlue","wkListBlue",100,254,2,1);
 local grey=brightrand();
 selgame.set_rgb(RED,GREEN,BLUE);
 filtername.set_rgb(RED,GREEN,BLUE);
 instructions.set_rgb(RED,GREEN,BLUE);
 freeplayleft.set_rgb(grey,grey,grey);
 freeplayright.set_rgb(grey,grey,grey);
}

function fades(ttype,var,ttime){
 switch (ttype){
  case Transition.FromGame:
   fltsh.msg="<  [FilterName]  >";
   filtername.msg="<  [FilterName]  >";
   return false;
   break;
	case Transition.ToGame:
   if (ttime<10){
    fltsh.msg="LOADING...";
    filtername.msg="LOADING...";
    return true;
   }
   break;
  case Transition.EndLayout:
   if (ttime<10){
    filtername.msg="EXITING";
    return true;
   }
   return false;
 }
}

fe.add_ticks_callback( "textTickles" );
fe.add_transition_callback( "fades" );
