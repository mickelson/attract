/////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - File Layout
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
// Requirements:
//  file.nut module
//  file-format.nut module
//
// Description:
//
// Using the xml, ini and txt from the file-format module, this module provides a base class
// to create layouts from various file formats and convert various front end layouts into Attract Mode.
// 
// The LayoutFile class is extended to map object tags and attributes into a AMLayout class.
//
// The extended class needs to use the functions provided to parse, translate and create an AM layout from
// the file content provided by the file-format readers.
// 
// Some helper functions are provided to help you convert things like colors, replace text variables, etc..
// 
// A list of items you need to consider when converting layouts:
//        layout width/height and orientation
//        fonts
//        sounds
//        text objects -> text
//        image objects -> image
//        artwork objects -> artwork
//        list objects -> list
//        views/screens -> surface
//        transitions -> transition
//        animations -> animate module
//
// TODO
//
// Detect layout type - look for specific file extensions (.lay, .mll). Otherwise, look for specific tags in xml,ini,txt?
// Additional Frontend Layouts - RetroFE is a WIP, EmulationStation next?
// Possibly allow for multiple formats for a single layout? ( AM XML, INI and TXT )
//
/////////////////////////////////////////////////////////

fe.load_module("file-format");

::FileLayoutVersion <- 1.0;
DEBUG_FILELAYOUT <- false;

// Base class that any converted layouts will be turned into
class AMLayout
{
    attr = null;
    animations = null;
    objects = null;
    modules = null;
    scripts = null;
    sounds = null;
    config = null;
    plugins = null;
    callbacks = null;
    constructor()
    {
        attr = {
            width = 640,
            height = 480,
            font = "arial",
            base_rotation = 0,
            //toggle_rotation
            //page_size
        }
        scripts = [];
        modules = [];
        animations = [];
        objects = [];
        sounds = [];
        plugins = [];
        callbacks = [];
        config = {}
    }
    
    function add_config( name, label, help, options = null, order = null )
    {
        UserConfig[name] <- "";
        local cfg = {
            label = label,
            help = help,
        }
        if ( options != null ) cfg.options <- options;
        if ( order != null) cfg.order <- order;
        
        UserConfig.setattributes( name, cfg );
    }
}

//LayoutFile class should be extended to parse content and create an AMLayout class instance.
class LayoutFile
{
    content = null;
    path = "";
    file_name = "";
    layout = null;
    
    constructor( filename )
    {
        find_path_file( filename );
        
        //read xml content from file
        switch( file_type() )
        {
            case "xml":
                content = xml.loadFile( path + "/" + file_name );
                break;
            case "ini":
                content = ini.loadFile( path + "/" +  file_name );
                break;
            case "txt":
                content = txt.loadFile( path + "/" +  file_name );
                break;
        }
        
        //print out content in DEBUG mode
        if ( DEBUG_FILELAYOUT ) display();
        
        //parse the xml content into an AMLayout instance
        layout = parse( content );
        
        //translate layout values to AM compatible ones
        layout = translate( layout );
        
        //load any included modules
        for ( local i = 0; i < layout.modules.len(); i++ )
        {
            fe.load_module( layout.modules[i] );
            if ( DEBUG_FILELAYOUT ) print( "loaded module: " + layout.modules[i] + "\n" );
        }
        
        //add layout config options
        //foreach( config in layout.config )
        //{
        //    layout.add_config("setting3", "Setting 3", "Test of default value", "default");
        //}
        
        //create AM objects from AMLayout table
        create( layout );
        
        //run any included scripts
        for ( local i = 0; i < layout.scripts.len(); i++ )
        {
            //need to run in same position as it was in xml?
            fe.do_nut( path + layout.scripts[i] );
        }

    }
    
    function file_type()
    {
        return "xml";
    }
    
    //parse is responsible for reading XML content into an AMLayout instance of string attributes and values
    // it returns an AMLayout instance
    function parse( content )
    {
        return AMLayout();
    }
    
    //translate is responsible for converting string values into ones AM can recognize
    // it returns the layout with all values translated
    function translate( layout )
    {
        return layout;
    }
    
    //create is responsible for taking the AMFormat instance and creating the layout objects
    function create( layout )
    {
        
    }
    
    //hook up animations with the animate module
    function animate( config, obj = null )
    {
        local animConfig = {}
        
        //config values for all animations
        animConfig.type <- ( "type" in config ) ? config["type"] : "PropertyAnimation";
        animConfig.when <- ( "when" in config ) ? config["when"] : When.ToNewSelection;
        animConfig.time <- ( "time" in config ) ? config["time"].tointeger() : 500;
        animConfig.delay <- ( "delay" in config ) ? config["delay"].tointeger() : 0;
        animConfig.wait <- ( "wait" in config ) ? config["wait"] : false;
        animConfig.loop <- ( "loop" in config ) ? config["loop"] : false;
        animConfig.pulse <- ( "pulse" in config ) ? config["pulse"] : false;
        animConfig.tween <- ( "tween" in config ) ? config["tween"] : Tween.Linear;
        animConfig.easing <- ( "easing" in config ) ? config["easing"] : Easing.Out;
        animConfig.restart <- ( "restart" in config ) ? config["restart"] : true;
        switch( animConfig.type )
        {
            case "PropertyAnimation":
                animConfig.property <- ( "property" in config ) ? config["property"] : "alpha";
                animConfig.start <- ( "start" in config ) ? config["start"] : 0;
                animConfig.end <- ( "end" in config ) ? config["end"] : 0;
                animation.add( PropertyAnimation( obj, animConfig ) );
                break;
            case "ParticleAnimation":
                animConfig.resources <- []
                if ( "resources" in config )
                {
                    local resources = split( config["resources"], "," );
                    foreach( resource in resources )
                    {
                        animConfig.resources.push( resource );
                    }
                }
                animConfig.ppm <- ( "ppm" in config ) ? config["ppm"].tointeger() : 60;
                animConfig.x <- ( "x" in config ) ? config["x"].tointeger() : 0;
                animConfig.y <- ( "y" in config ) ? config["y"].tointeger() : 0;
                animConfig.width <- ( "width" in config ) ? config["width"].tointeger() : 1;
                animConfig.height <- ( "height" in config ) ? config["height"].tointeger() : 1;
                animConfig.limit <- ( "limit" in config ) ? config["limit"].tointeger() : 1;
                animConfig.movement <- ( "movement" in config ) ? config["movement"] : true;
                animConfig.angle <- ( "angle" in config ) ? split( config["angle"], "," ) : "0,0";
                animConfig.speed <- ( "speed" in config ) ? split( config["speed"], "," ) : "150,150";
                animConfig.scale <- ( "scale" in config ) ? split( config["scale"], "," ) : "1.0,1.0";
                animation.add( ParticleAnimation( animConfig ) );
                break;
        }
        
    }
    
    //find the path for this layout, if it doesn't exist look in the script_dir
    function find_path_file( filename )
    {
        local hasPath = false;
        local firstPos = -1;
        local lastPos = 0;
        //look for slashes to see if we have any path
        for ( local i = 0; i < filename.len(); i++ )
        {
            if ( filename[i].tochar() == "/" || filename[i].tochar() == "\\" )
            {
                hasPath = true;
                if ( firstPos = -1 ) firstPos = i;
                lastPos = i;
            }
        }
        
        if ( hasPath )
        {
            local relativePath = filename.slice( 0, lastPos + 1 );
            local filenameOnly = filename.slice( lastPos + 1, filename.len() );
            local f = null;
            try
            {
                //first try relative to script_dir
                this.path = fe.script_dir + relativePath;
                this.file_name = filenameOnly;
                if ( DEBUG_FILELAYOUT ) print( "trying relative path: " + this.path + this.file_name + " for " + filenameOnly + "\n" );
                f = file( this.path + this.file_name, "r" );
            }
            catch ( e )
            {
                //try direct path (in case of full location provided)
                try
                {
                    this.path = relativePath;
                    this.file_name = filenameOnly;
                    if ( DEBUG_FILELAYOUT ) print( "trying direct path: " + this.path + this.file_name + " for " + filenameOnly + "\n" );
                    f = file( this.path + this.file_name );
                }
                catch ( e )
                {
                    //just look in script_dir for file
                    this.path = fe.script_dir;
                    this.file_name = filenameOnly;
                }
            }
        } else
        {
            //just look in the script_dir path
            this.path = fe.script_dir;
            this.file_name = filename;
            if ( DEBUG_FILELAYOUT ) print( "trying script_dir: " + this.path + this.file_name + " for " + filenameOnly + "\n" );
        }
        return 0;
    }
    
    function display()
    {
        switch( file_type() )
        {
            case "xml":
                xml.display( content );
                break;
            case "ini":
                ini.display( content );
                break;
            case "txt":
                txt.display( content );
                break;
        }
    }
}

//convert integer to RGB
function intToRGB( color )
{
    return {
        r = color & 255,
        b = (color >> 16) & 255,
        g = (color >> 8) & 255
    }
}

function rgbToHex(r, g, b) {
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).tostring(16).slice(1);
}

function hexTorgb( val )
{
    local color = {
        red = hex2int(val, 0),
        green = hex2int(val, 2),
        blue = hex2int(val, 4)
    }
    return color;
}

function hex2int( hex, pos )
{
    if ( hex == null || hex == "" || ( pos < 0 && pos > 4 ) ) return 255;
    local val = 0;
    if ( hex[pos+1] < 58 ) // 0-9
		val = hex[pos+1] - 48;
	else // A-F
		val = hex[pos+1] - 55;

	if ( hex[pos] < 58 )
		val += (hex[pos] - 48) << 4;
	else
		val += (hex[pos] - 55) << 4;

	return val.tointeger();
}

LAYERS <- [];

for( local i = 0; i < 10; i++ )
{
    LAYERS.push( fe.add_surface( fe.layout.width, fe.layout.height ) );
}
if ( DEBUG_FILELAYOUT ) print( "Layers created: " + LAYERS.len() + "\n" );

//Allows you to replace text in a string
function replace(string, original, replacement)
{
  //if ( typeof( string != "string" ) ) return original;
  
  // make a regexp that will match the substring to be replaced
  //
  local expression = regexp(original);

  local result = "";
  local position = 0;

  // find the first match
  //
  local captures = expression.capture(string);

  while (captures != null)
  {
    foreach (i, capture in captures)
    {
      // copy from the current position to the start of the match
      //
      result += string.slice(position, capture.begin);

      // add the replacement substring instead of the original
      //
      result += replacement;

      position = capture.end;
    }

    // find the next match
    //
    captures = expression.capture(string, position);
  }

  // add any remaining part of the string after the last match
  //
  result += string.slice(position);

  return result;
}

function print_table( table, sub = "" )
{
    foreach ( name, value in table )
    {
        if ( typeof(value) == "table" )
        {
            print( "[" + sub + "]" + "\n" );
            print_table( value, name );
        } else
        {
            print( name + ": " + value + "\n" );
        }
    }
}

//load all file-layout scripts
local path = FeConfigDirectory + "modules/file-layout";
local dir = DirectoryListing( path );
foreach ( f in dir.results )
{
    try
    {
        local name = f.slice( path.len() + 1, f.len() );
        local ext = f.slice( f.len() - 4 );
        if ( ext == ".nut" )
        {
            if ( DEBUG_FILELAYOUT ) print("Loading file-layout module: " + name + "\n" );
            fe.load_module( "file-layout/" + name );
        }
    }catch ( e )
    {
        print( "file-layout.nut: Error loading module: " + f );
    }
}


