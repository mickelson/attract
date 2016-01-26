///////////////////////////////////////////////////
//
// File-Format
//
// This module allows you to load files (.txt, .ini and .xml) into something usable in squirrel. Files are loaded relative to
// the fe.script_dir.
//
// REQUIREMENTS
//  file.nut module to read files
//
// USAGE
// 
//      TXT: 
//      local textfile = txt.loadFile( filename );
//      local line1 = textfile.lines[0];
//      foreach( line in textfile.lines )
//      {
//         print( line + "\n" );
//      }  
// 
//      XML:
//      local xmlfile = xml.loadFile( filename );
//      //or local xml = xml.load("<xml name="xmlfile"><test name="testing">Test Text</test></xml>");
//      local rootTag = xml.tag;
//      local rootAttr = xml.attr["name"];
//      foreach( child in xml.children )
//      {
//          local tag = child.tag;
//          local attr = ("name" in child.attr ) ? child.attr["name"] : "default";
//          local text = ( "text" in child ) ? child.text : "default";
//      }
//
//      INI:
//      local inifile = ini.loadFile( filename );
//      local setting_value = ini.map["General"]["setting"];
//      local vertWidth = 0;
//      local hasVertical = ( "Vertical" in ini.map ) ? true : false;
//      if ( hasVertical ) vertWidth = ini.map["Vertical"]["width"].tointeger() : 640;
//      foreach( head in ini.map )
//      {
//          print( "heading: " + head + "\n" );
//      }
//
// TODO
//
//  ALL
//      Validation of XML, INI, TXT (checking tag-endtags match), proper XML format
//      Allow for reading a string or reading from filename for INI and TXT
//      Write to XML, INI, TXT in squirrel
//      Additional helper functions

fe.load_module( "file" );

::FileFormatVersion <- 1.0;
DEBUG_FILEFORMAT <- false;

local dir = fe.script_dir;

class TXTFile
{
    lines = null;
    constructor( lines )
    {
        this.lines = lines;
    }
}

class TXT
{
    constructor()
    {
    
    }
    
    function readFile( filename )
    {
        print( "Loading TXT: " + filename + "\n" );
        local lines = [];
        local f = ReadTextFile( filename );
        local pos = 0;
        while ( !f.eos() )
        {
            //read each line
            lines.push(f.read_line());
        }
        return lines;
    }
    
    function loadFile( filename )
    {
        local lines = readFile( filename );
        return TXTFile( lines );
    }
    
    function display( txtFile )
    {
        print( "\n----------\nINI Document\n----------\n\n" );
        foreach( line in txtFile.lines )
        {
            print( line + "\n" );
        }
    }
}


class INIFile
{
    map = null;
    constructor()
    {
        map = {}
    }
}

class INI
{

    constructor()
    {
    
    }
    
    function readFile( filename )
    {
        print( "Loading INI: " + filename + "\n" );
        local ini = INIFile();
        local entity=null;
        local f = ReadTextFile( filename );
        while ( !f.eos() )
        {
            local line = f.read_line();
            if (( line.len() > 0 ) && ( line[0] == '[' ))
            {
                entity = line.slice( 1, line.len()-1 );
                ini.map[ entity ] <- {};
            }
            else
            {
                local temp = split( line, "=" );
                local v = ( temp.len() > 1 ) ? temp[ 1] : "";

                if ( entity ) ini.map[entity][temp[0]] <- v;
                else ini.map[temp[0]] <- v;
            }
        }
        return ini;
    }
    
    function loadFile( filename )
    {
        return readFile( filename );
    }
        
    function display( iniFile )
    {
        print( "\n----------INI Document\n----------\n\n" );
        foreach( entity in iniFile.map )
        {
            print( "[" + entity + "]" + "\n" );
            foreach( name, val in entity )
            {
                print( name + "=" + val + "\n" );
            }
        }
    }
}

class XMLNode
{
    parent = null;
    tag = "";
    text = "";
    attr = null;
    children = null;
    startPos = 0;
    
    constructor()
    {
        attr = {}
        children = [];
    }
    
    function addAttr( name, val )
    {
        attr[name] <- val;
    }
    
    function addChild( node )
    {
        node.parent = this;
        children.push( node );
        return children.len() - 1;
    }
    
    function getChild( tag )
    {
        foreach ( child in children )
        {
            if ( child.tag == tag ) return child;
        }
        return null;
    }
    
    function toXML()
    {
        //export XMLNode to XML string
        return nodeToString( this );
    }
    
    function nodeToString( node )
    {
        local str = nodeDepth( node ) + "<" + node.tag;
        //add attrs
        foreach( name, val in node.attr )
        {
            str += " " + name + "=\"" + val + "\""; 
        }
        if ( node.text == "" && node.children.len() == 0 )
        {
            //no end tag
            str += " />\n";
        } else
        {
            //show text and/or child nodes
            str += ">\n";
            //if ( node.children == 0 && text != "" ) str += node.text;
            str += node.text;
            foreach( child in node.children )
            {
                str += nodeToString( child );
            }
            //todo if no text value, close with single tag
            str += nodeDepth( node ) + "</" + node.tag + ">\n";
        }
        return str;
    }
    
    function nodeDepth( node, depth = "" )
    {
        if ( node.parent != null )
        {
            depth += "    ";
            depth = nodeDepth( node.parent, depth );
        }
        return depth;
    }
}

class XML
{
    constructor()
    {
        
    }

    function readFile( filename )
    {
        //read XML to string
        print( "Loading XML: " + filename + "\n" );
        local str = "";
        local f = ReadTextFile( filename );
        local pos = 0;
        while ( !f.eos() )
        {
            //read each char in line, don't include new line / tab chars
            local line = f.read_line();
            while ( pos < line.len() )
            {
                local char = line[pos].tochar();
                if ( char != "\n" && char != "\t" )
                {
                    str += char;
                }
                pos++;
            }
            pos = 0;
        }
        return str;
    }
    
    function loadFile( filename )
    {
        local xml = readFile( filename );
        return load( xml );
    }
    
    
    function load( xmlStr )
    {
        local doc = XMLNode();
        local pos = 0;
        local ROOT = true;
        local QUOTES = false;
        local COMMENT = false;
        local testcount = 0;
        while ( pos < xmlStr.len() )
        {
            local prev = ( pos - 1 >= 0 ) ? xmlStr[pos - 1].tochar() : "";
            local char = xmlStr[pos].tochar();
            local next = ( pos + 1 < xmlStr.len() ) ? xmlStr.slice( pos, pos + 1 ) : "";
            local next2 = ( pos + 2 < xmlStr.len() ) ? xmlStr.slice( pos, pos + 2 ) : "";
            local next3 = ( pos + 3 < xmlStr.len() ) ? xmlStr.slice( pos, pos + 3 ) : "";
            local next4 = ( pos + 4 < xmlStr.len() ) ? xmlStr.slice( pos, pos + 4 ) : "";
            
            if ( xmlStr[pos] == 34 ) QUOTES = !QUOTES;  //need to know if we are in quotes when we encounter tag symbols
            if ( next4 == "<!--" )
            {
                COMMENT = true;      //start of comment
                pos += 3;
            }
            if ( next3 == "-->")
            {
                COMMENT = false;       //end of comment
                pos += 2;
            }
            
            //read text value
            if ( !COMMENT && !QUOTES && next == ">" && next2 != "><" )
            {
                local textEnd = pos + 2;
                while ( textEnd < xmlStr.len() )
                {
                    if ( xmlStr[textEnd] == 34 ) QUOTES = !QUOTES;
                    if ( !QUOTES )
                    {
                        if ( xmlStr[textEnd].tochar() == "<" ) break;
                    }
                    textEnd++;
                }
                local text = xmlStr.slice( pos + 1, textEnd );
                doc.text = text;
                if ( DEBUG_FILEFORMAT ) print( "text: " + text + "\n" );
            }

            if ( !COMMENT && !QUOTES && char == "<" && next2 != "</" && next2 != "<!" )
            {
                //start of next tag, find tag name
                local tagEnd = pos + 1;
                while ( tagEnd < xmlStr.len() && xmlStr[tagEnd].tochar() != " " && xmlStr[tagEnd].tochar() != ">" ) tagEnd++;
                local tag = xmlStr.slice( pos + 1, tagEnd );
                
                if ( ROOT )
                {
                    if ( DEBUG_FILEFORMAT ) print( "root node: " + tag + "\n" );
                    ROOT = false;
                    doc.startPos = pos;
                } else
                {
                    //create a new node as a child of the current one
                    local node = XMLNode();
                    node.parent = doc;
                    node.startPos = pos;
                    local index = doc.addChild(node);
                    doc = doc.children[index];
                }
                doc.tag = tag;
                
                if ( DEBUG_FILEFORMAT ) print( doc.tag + " (" + doc.startPos + ") : " );
                pos = tagEnd + 1;
                
                //read attributes?
                local attrEnd = pos - 1;
                while ( attrEnd < xmlStr.len() )
                {
                    if ( xmlStr[attrEnd] == 34 ) QUOTES = !QUOTES;
                    if ( !QUOTES )
                    {
                        if ( xmlStr[attrEnd].tochar() == "/" || xmlStr[attrEnd].tochar() == ">" ) break;
                    }
                    attrEnd++;
                }
                local attributes = xmlStr.slice( pos - 1, attrEnd );
                if ( DEBUG_FILEFORMAT ) print( attributes + "\n" );

                local p=0;
                while ( p < attributes.len() )
                {
                   local eq = attributes.find( "=", p );
                   if ( eq == null )
                      break;

                   local name = strip( attributes.slice( p, eq ) );
                   p = eq+1;

                   // pass whitespace after '='
                   while ( attributes[p] <= 32 )
                      p++;

                   local val;
                   if ( attributes[p] == 34 ) // double quote
                   {
                      p++;
                      eq = attributes.find( "\"", p );
                      val = attributes.slice( p, eq );
                   }
                   else
                   {
                      eq = attributes.find( " ", p );
                      if ( eq == null )
                         eq = attributes.len();

                      val = attributes.slice( p, eq );
                   }
                   p = eq+1;
                   doc.attr[ name ] <- val;
                }

                pos = attrEnd - 1;
            }

            if ( !COMMENT && !QUOTES && ( next2 == "/>" || next2 == "</" ) )
            {
                if ( next2 == "/>" ) pos += 1;
                if ( next2 == "</" ) pos += doc.tag.len() + 1;
                
                local n = ( pos < xmlStr.len() ) ? xmlStr[pos].tochar() : "";
                if ( DEBUG_FILEFORMAT ) print( "  " + doc.tag + " - next: " + n + "\n");
                
                if ( doc.parent == null )
                {
                    ROOT = true;
                } else
                {
                    doc = doc.parent;
                }
            }
            
            pos++;
        }
        return doc;
    }
 
    function display( xmlNode )
    {
        print( "\n----------\nXML Document\n----------\n\n" );
        print( "Root: " + xmlNode.tag + "\n" );
        print( "Children: " + xmlNode.children.len() + "\n" );
        print( xmlNode.toXML() + "\n" );
    }
    
    function tabstr( count )
    {
        local str = "";
        for ( local i = 0; i < count; i++ ) str += "\t";
        return str;
    }
}

txt <- TXT();
ini <- INI();
xml <- XML();
