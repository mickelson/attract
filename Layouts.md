Attract-Mode Frontend
=====================

Layout and Plug-in Programming Reference
----------------------------------------

Overview
--------

The Attract-mode layout sets out what gets displayed to the user. Layouts
consist of at least one script file and a collection of related resources
(images, etc.) used by the script.

Layouts are stored under the "layouts" subdirectory of the Attract-Mode 
config directory.  Each layout gets its own separate subdirectory.  Each
layout can have one or more ".nut" script files in it.  The "Toggle Layout" 
command in Attract-Mode allows users to cycle between each of the ".nut" 
script files located in the layout's directory.  Attract-Mode remembers the 
last file toggled to for each layout and will go back to that same file the 
next time the layout is loaded.  This allows for variations of a particular 
layout to be implemented and easily selected by the user (for example, a 
layout could provide "landscape" and "portrait" versions).

The Attract-Mode screen saver is really just a special case layout that is 
loaded after a user-configured period of inactivity.  The screen saver script
is located in the "screensaver.nut" file stored in the "layouts" subdirectory.

Squirrel Language
-----------------

Attract-Mode's layouts are scripts written in the Squirrel programming
language.  Squirrel's standard "Math" and "String" library functions are 
available for use in a script. For more information on programming in 
Squirrel and using its standard libraries, consult the Squirrel manuals:

   * [Squirrel 3.0 Reference Manual][SQREFMAN]
   * [Squirrel 3.0 Standard Library Manual][SQLIBMAN]

[SQREFMAN]: http://www.squirrel-lang.org/doc/squirrel3.html
[SQLIBMAN]: http://www.squirrel-lang.org/doc/sqstdlib3.html

Frontend Binding
----------------

All of the functions, objects and classes that Attract-Mode exposes to 
Squirrel are arranged under the `fe` table, which is bound to Squirrel's 
root table.

Example:

		fe.layout.orient = RotateScreen.Right;
		fe.add_image( "bg.png", 0, 0 );
		local marquee = fe.add_artwork( "marquee", 256, 20, 512, 256 );
		marquee.set_rgb( 100, 100, 100 );

The remainder of this document describes the functions, objects, classes
and constants that are exposed to layout and plug-in scripts.

Functions
---------

#### `fe.add_image()` ####

    fe.add_image( name )	
    fe.add_image( name, x, y )	
    fe.add_image( name, x, y, w, h )
 
Add a static image to the end of Attract-Mode's draw list.  
 
Parameters:   

   * name - the name of an image file located in the layout directory. 
     Supported file formats include: PNG, JPEG, GIF, BMP and TGA.
   * x - the x coordinate of the top left corner of the image (in layout 
     coordinates).
   * y - the y coordinate of the top left corner of the image (in layout 
     coordinates).
   * w - the width of the image (in layout coordinates).  Image will be 
     scaled accordingly.  If set to 0 image is left unscaled.  Default value
     is 0.
   * h - the height of the image (in layout coordinates).  Image will be 
     scaled accordingly.  If set to 0 image is left unscaled.  Default value
     is 0.
 
Return Value:

   * An instance of the class `fe.Image` which can be used to interact with
     the added image.


#### `fe.add_artwork()` ####

    fe.add_artwork( label )	
    fe.add_artwork( label, x, y )	
    fe.add_artwork( label, x, y, w, h )

Add an artwork to the end of Attract-Mode's draw list.  The image displayed
in an artwork is updated automatically whenever the user changes the game
selection.

Parameters:   

   * label - the label of the artwork to display.  This needs to correspond
     to an artwork resource configured in Attract-Mode (artworks are 
     configured per emulator in Attract-Mode's configuration mode).  
   * x - the x coordinate of the top left corner of the artwork (in layout 
     coordinates). 
   * y - the y coordinate of the top left corner of the artwork (in layout 
     coordinates).
   * w - the width of the artwork (in layout coordinates).  Artworks will be 
     scaled accordingly.  If set to 0 artwork is left unscaled.  Default 
     value is 0.
   * h - the height of the artwork (in layout coordinates).  Artworks will be 
     scaled accordingly.  If set to 0 artwork is left unscaled.  Default 
     value is 0.

Return Value:

   * An instance of the class `fe.Image` which can be used to interact with
     the added artwork.


#### `fe.add_clone()` ####

    fe.add_clone( img )	

Clone an image or artwork and add the clone to the back of Attract-Mode's 
draw list.  The texture pixel data of the original and clone is shared 
as a result.

Parameters:   

   * img - the image or artwork object to clone.  This needs to be an 
     instance of the class `fe.Image`.

Return Value:

   * An instance of the class `fe.Image` which can be used to interact with
     the added clone.


#### `fe.add_text()` ####

    fe.add_text( msg, x, y, w, h )

Add a text label to the end of Attract-Mode's draw list.  

Parameters:   

   * msg - the text to display.  The tokens below that appear in the 'msg'
     string will be substituted appropriately:
      - `[ListTitle]` - the name of the current display list
      - `[ListSize]` - the number of items in the current display list
      - `[ListEntry]` - the number of the current selection in the current
        display list
      - `[ListFilterName]` - the name of the current display list filter
      - `[Name]` - the short name of the selected game
      - `[Title]` - the full name of the selected game
      - `[Emulator]` - the emulator to use for the selected game
      - `[CloneOf]` - the short name of the game that the selection is a 
        clone of
      - `[Year]` - the year for the current selected game
      - `[Manufacturer]` - the manufacturer for the selected game
      - `[Category]` - the category for the selected game
      - `[Players]` - the number of players for the selected game
      - `[Rotation]` - the rotation for the selected game
      - `[Control]` - the primary control for the selected game
      - `[Status]` - the status for the selected game
   * x - the x coordinate of the top left corner of the text (in layout 
     coordinates).
   * y - the y coordinate of the top left corner of the text (in layout 
     coordinates).
   * w - the width of the text (in layout coordinates).
   * h - the height of the text (in layout coordinates).

Return Value:

   * An instance of the class `fe.Text` which can be used to interact with
     the added text.


#### `fe.add_listbox()` ####

    fe.add_listbox( x, y, w, h )

Add a listbox to the end of Attract-Mode's draw list.  

Parameters:   

   * x - the x coordinate of the top left corner of the listbox (in layout 
     coordinates).
   * y - the y coordinate of the top left corner of the listbox (in layout 
     coordinates).
   * w - the width of the listbox (in layout coordinates). 
   * h - the height of the listbox (in layout coordinates). 

Return Value:

   * An instance of the class `fe.ListBox` which can be used to interact with
     the added text.


#### `fe.add_sound()` ####

    fe.add_sound( name )

Add a small sound sample for the layout to play.  The entire file is loaded 
into memory.

Parameters:   

   * name - the name of the sound file located in the layout directory. 

Return Value:

   * An instance of the class `fe.Sound` which can be used to interact with
     the added sound.


#### `fe.add_ticks_callback()` ####

    fe.add_ticks_callback( function_name )

Register a function in your script to get "tick" callbacks.  Tick callbacks
occur continuously during the running of the frontend.  The function that is
registered should be in the following form:

    function tick( tick_time )
    {
       // do stuff...
    }

The single parameter passed to the tick function is the amount of time (in 
milliseconds) since the layout began.

Parameters:   

   * function_name - a string naming the function to be called.

Return Value:

   * None.


#### `fe.add_transition_callback()` ####

    fe.add_transition_callback( function_name )

Register a function in your script to get transition callbacks.  Transition
callbacks are triggered by certain events in the frontend.  The function 
that is registered should be in the following form:

    function transition( ttype, var, transition_time )
    {
       local redraw_needed = false;

       // do stuff...

       if ( redraw_needed )
          return true;

       return false;
    }

The `ttype` parameter passed to the transition function indicates what is
happening.  It will have one of the following values:

   * `Transition.StartLayout`
   * `Transition.EndLayout`
   * `Transition.ToNewSelection`
   * `Transition.ToGame`
   * `Transition.FromGame`
   * `Transition.ToNewList`

The value of the `var` parameter passed to the transition function depends
upon the value of `ttype`:

   * When `ttype` is `Transition.ToNewSelection`, `var` indicates the index 
     offset of the selection being transitioned to (i.e. -1 when moving back 
     one position in the list, 1 when moving forward one position, 2 when
     moving forward two positions, etc.)

   * When `ttype` is `Transition.StartLayout` or `Transition.ToNewList`, 
     `var` will be one of the following: 
      - `FromTo.Frontend` if the frontend is just starting,
      - `FromTo.ScreenSaver` if the layout is starting (or the list is being
        loaded) because the built-in screen saver has stopped, or
      - `FromTo.NoValue` otherwise.

   * When `ttype` is `Transition.EndLayout`, `var` will be:
      - `FromTo.Frontend` if the frontend is shutting down, 
      - `FromTo.ScreenSaver` if the layout is stopping because the built-in 
        screen saver is starting, or 
      - `FromTo.NoValue` otherwise.

   * When `ttype` is `Transition.ToGame` or `Transition.FromGame`, `var` 
     will be `FromTo.NoValue`.

The `transition_time` parameter passed to the transition function is the
amount of time (in milliseconds) since the transition began.

The transition function must return a boolean value.  It should return
`true` if a redraw is required, in which case Attract-Mode will redraw the
screen and immediately call the transition function again with an updated 
`transition_time`.

**The transition function must eventually return `false` to notify 
Attract-Mode that the transition effect is done, allowing the normal 
operation of the frontend to proceed.**

Parameters:   

   * function_name - a string naming the function to be called.

Return Value:

   * None.


#### `fe.game_info()` ####

    fe.game_info( id )
    fe.game_info( id, index_offset )

Get information about the selected game.  

Parameters:   

   * id - id of the information attribute to get.  Can be one of the 
     following values:
      - `Info.Name`
      - `Info.Title`
      - `Info.Emulator`
      - `Info.CloneOf`
      - `Info.Year`
      - `Info.Manufacturer`
      - `Info.Category`
      - `Info.Players`
      - `Info.Rotation`
      - `Info.Control`
      - `Info.Status`
   * index_offset - the offset (from the current selection) of the game to 
     retrieve info on.  i.e. -1=previous game, 0=current game, 1=next game...
     and so on.  Default value is 0.

Return Value:

   * A string containing the requested information.


#### `fe.is_keypressed()` ####

    fe.is_keypressed( keycode )

Check if a keyboard key is currently being pressed. 

Parameters:

   * keycode - the key to test.  Can be one of the following values:
      - `Key.A` to `Key.Z`
      - `Key.Num0` to `Key.Num9`
      - `Key.Escape`
      - `Key.LControl`, `Key.LShift`, `Key.LAlt`, `Key.LSystem`
      - `Key.RControl`, `Key.RShift`, `Key.RAlt`, `Key.RSystem`
      - `Key.Menu`
      - `Key.LBracket`, `Key.RBracket`
      - `Key.Semicolon`, `Key.Comma`, `Key.Period`, `Key.Quote`, `Key.Slash`
      - `Key.Backslash`, `Key.Tilde`, `Key.Equal`, `Key.Dash`
      - `Key.Space`
      - `Key.Return`
      - `Key.Backspace`
      - `Key.Tab`
      - `Key.PageUp`, `Key.PageDown`
      - `Key.End`, `Key.Home`, `Key.Insert`, `Key.Delete`
      - `Key.Add`, `Key.Subtract`, `Key.Multiply`, `Key.Divide`
      - `Key.Left`, `Key.Right`, `Key.Up`, `Key.Down`
      - `Key.Numpad0` to `Key.Numpad9`
      - `Key.F1` to `Key.F15`
      - `Key.Pause`

Return Value:

   * `true` if key is pressed, `false` otherwise.


#### `fe.is_joybuttonpressed()` ####

    fe.is_joybuttonpressed( joy_id, button_num )

Check if a joystick button is currently being pressed.

Parameters:

   * joy_id - index of joystick from 0 (for first joystick) to 7.
   * button_num - button number. 

Return Value:

   * `true` if button is pressed, `false` otherwise.


#### `fe.get_joyaxispos()` ####

    fe.get_joyaxispos( joy_id, axis_id )

Return the current position for the specified joystick axis.

Parameters:

   * joy_id - index of joystick from 0 (for first joystick) to 7.
   * axis_id - the axis to check. Can be one of the following values:
      - `Axis.X`
      - `Axis.Y`
      - `Axis.Z`
      - `Axis.R`
      - `Axis.U`
      - `Axis.PovX`
      - `Axis.PovY`

Return Value:

   * Current position of the axis, in range [-100 ... 100].


#### `fe.do_nut()` ####

    fe.do_nut( name )

Execute another Squirrel script.

Parameters:

   * name - the name of the script file located in the layout directory. 

Return Value:

   * None.

#### `fe.plugin_command()` ####

    fe.plugin_command( plugin_name, arg_string )
    fe.plugin_command( plugin_name, arg_string, callback_function )

Execute a plug-in command and wait until the command is done.

Parameters:

   * plugin_name - the name of the plug-in command to run.
   * arg_string - the arguments to pass when the plug-in command is executed.
   * callback_function - a string containing the name of the function in 
     Squirrel to call with any output that the plug-in command provides on 
     stdout.  The function should be in the following form:

      function callback_function( op )
      {
      }

     If provided, this function will be called with each line of output from 
     the plug-in command in the `op` variable.

Return Value:

   * None.

#### `fe.plugin_command_bg()` ####

    fe.plugin_command_bg( plugin_name, arg_string )

Execute a plug-in command in the background and return immediately.

Parameters:

   * plugin_name - the name of the plug-in command to run.
   * arg_string - the arguments to pass when the plug-in command is executed.

Return Value:

   * None.

#### `fe.path_expand( path )` ####

Expand the given path name.  A leading `~` or `$HOME` token will be become
the user's home directory.  On Windows systems, a leading `%SYSTEMROOT%` 
token will become the path to the Windows directory and a leading
`%PROGRAMFILES%` will become the path to the "Program Files" directory.

Parameters:

   * path - the path string to expand.

Return Value:

   * The expansion of path. 


Objects and Variables
---------------------

#### `fe.layout` ####

`fe.layout` is an instance of the `fe.LayoutGlobals` class and is where
global layout settings are stored.


#### `fe.list` ####

`fe.list` is an instance of the `fe.CurrentList` class and is where current
list settings are stored.


#### `fe.objs` ####

`fe.objs` contains the Attract-Mode draw list.  It is an array of `fe.Image`,
`fe.Text` and `fe.ListBox` instances.


#### `fe.init_name` ####

When Attract-Mode runs a plug-in script, `fe.init_name` is set to the 
name of the plug-in.  NOTE this variable will *not* be valid when callback
functions are executed.

#### `fe.uconfig` ####

When Attract-Mode runs a plug-in script, the `fe.uconfig` table contains the
user configured plug-in settings.  NOTE this table will *not* be valid when
callback functions are executed.  A plug-in script can signal its user
configured settings to Attract-Mode by defining a class named "UserConfig"
at the very start of the script.  Please see one of the plug-ins included
with Attract-Mode for an example.

Classes
-------

#### `fe.LayoutGlobals` ####

This class is a container for global layout settings.  The instance of this
class is the `fe.layout` object.  This class cannot be otherwise instantiated
in a script.

Attributes:   

   * `width` - Get/set the layout width.  Default value is `ScreenWidth`.
   * `height` - Get/set the layout height.  Default value is `ScreenHeight`.
   * `font` - Get/set the layout font name.  Default value is the default
     font configured for Attract-Mode.
   * `orient` - Get/set the global layout orientation.  Can be one of the 
     following values:
      - `RotateScreen.None` (default)
      - `RotateScreen.Right`
      - `RotateScreen.Flip`
      - `RotateScreen.Left`

#### `fe.CurrentList` ####

This class is a container for current list settings.  The instance of this
class is the `fe.list` object.  This class cannot be otherwise instantiated
in a script.

Attributes:   

   * `name` - Get the name of the current list.
   * `filter` - Get the name of the current list filter.
   * `size` - Get the size of the current list.
   * `index` - Get/set the current list selection index.


#### `fe.Image` ####

The class representing an image in Attract-Mode.  Instances of this class
are returned by the `fe.add_image()`, `fe.add_artwork()` and 
`fe.add_clone()` functions and also appear in the `fe.objs` array (the 
Attract-Mode draw list).  This class cannot be otherwise instantiated
in a script.

Attributes:   

   * `x` - Get/set x position of top left corner (in layout coordinates). 
   * `y` - Get/set y position of top left corner (in layout coordinates). 
   * `width` - Get/set width of image (in layout coordinates), 0 if the 
     image is unscaled.  Default value is 0.
   * `height` - Get/set height of image (in layout coordinates), if 0 the
     image is unscaled.  Default value is 0.
   * `visible` - Get/set whether image is visible (boolean).  Default value
     is `true`.
   * `rotation` - Get/set rotation of image. Range is [0 ... 360].  Default
     value is 0.
   * `red` - Get/set red colour level for image. Range is [0 ... 255].
     Default value is 255.
   * `green` - Get/set green colour level for image. Range is [0 ... 255].
     Default value is 255.
   * `blue` - Get/set blue colour level for image. Range is [0 ... 255].
     Default value is 255.
   * `alpha` - Get/set alpha level for image. Range is [0 ... 255].  Default 
     value is 255.
   * `index_offset` - [artwork only] Get/set offset from current selection 
     for the artwork to display.  For example, set to -1 for the image 
     corresponding to the previous list entry, or 1 for the next list entry,
     etc.  Default value is 0.
   * `shear_x` - Get/set the x shear factor for image (in layout coordinate
     units).  Default value is 0.
   * `shear_y` - Get/set the y shear factor for image (in layout coordinate
     units).  Default value is 0.
   * `texture_width` - Get the width of the image texture (in pixels).
   * `texture_height` - Get the height of the image texture (in pixels).
   * `subimg_x` - Get/set the x position of top left corner of the image 
     texture sub-rectangle to display.  Default value is 0.
   * `subimg_y` - Get/set the y position of top left corner of the image 
     texture sub-rectangle to display.  Default value is 0.
   * `subimg_width` - Get/set the width of the image texture sub-rectangle 
     to display.  Default value is `texture_width`.
   * `subimg_height` - Get/set the height of the image texture sub-rectangle 
     to display.  Default value is `texture_height`.
   * `movie_enabled` - [artwork only] Get/set whether movies may be displayed
     in the artwork if configured by the user (boolean).  Default value is 
     `true`.  

Functions:   

   * `set_rgb( r, g, b )` - Set the red, green and blue colour values for the 
     image.  Range is [0 ... 255].
     

#### `fe.Text` ####

The class representing a text label in Attract-Mode.  Instances of this 
class are returned by the `fe.add_text()` function and also appear in the 
`fe.objs` array (the Attract-Mode draw list).  This class cannot be 
otherwise instantiated in a script.

Attributes:   

   * `msg` - Get/set the text label's message.  The tokens below that appear
     in the 'msg' string will be substituted appropriately:
      - `[ListTitle]` - the name of the current display list
      - `[ListSize]` - the number of items in the current display list
      - `[ListEntry]` - the number of the current selection in the current
        display list
      - `[ListFilterName]` - the name of the current display list filter
      - `[Name]` - the short name of the selected game
      - `[Title]` - the full name of the selected game
      - `[Emulator]` - the emulator to use for the selected game
      - `[CloneOf]` - the short name of the game that the selection is a 
        clone of
      - `[Year]` - the year for the current selected game
      - `[Manufacturer]` - the manufacturer for the selected game
      - `[Category]` - the category for the selected game
      - `[Players]` - the number of players for the selected game
      - `[Rotation]` - the rotation for the selected game
      - `[Control]` - the primary control for the selected game
      - `[Status]` - the status for the selected game
   * `x` - Get/set x position of top left corner (in layout coordinates). 
   * `y` - Get/set y position of top left corner (in layout coordinates). 
   * `width` - Get/set width of text (in layout coordinates).
   * `height` - Get/set height of text (in layout coordinates).
   * `visible` - Get/set whether text is visible (boolean).  Default value
     is `true`.
   * `rotation` - Get/set rotation of text. Range is [0 ... 360].  Default
     value is 0.
   * `red` - Get/set red colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `green` - Get/set green colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `blue` - Get/set blue colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `alpha` - Get/set alpha level for text. Range is [0 ... 255].  Default 
     value is 255.
   * `index_offset` - Get/set offset from current game selection for text 
     info to display.  For example, set to -1 to show text info for the 
     previous list entry, or 1 for the next list entry.  Default value is 0.
   * `bg_red` - Get/set red colour level for text background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_green` - Get/set green colour level for text background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_blue` - Get/set blue colour level for text background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_alpha` - Get/set alpha level for text background. Range is [0 ... 
     255].  Default value is 0 (transparent).
   * `charsize` - Get/set the forced character size.  If this is <= 0 
     then Attract-Mode will autosize based on `height`.  Default value is -1.
   * `style` - Get/set the text style.  Can be a combination of one or more
     of the following (i.e. `Style.Bold | Style.Italic`):
      - `Style.Regular` (default)
      - `Style.Bold`
      - `Style.Italic`
      - `Style.Underlined`
   * `align` - Get/set the text alignment.  Can be one of the following 
     values:
      - `Align.Centre` (default)
      - `Align.Left`
      - `Align.Right`
   * `word_wrap` - Get/set whether word wrapping is enabled in this text
     (boolean).  Default is `false`.

Functions:   

   * `set_rgb( r, g, b )` - Set the red, green and blue colour values for the 
     text.  Range is [0 ... 255].
   * `set_bg_rgb( r, g, b )` - Set the red, green and blue colour values for 
     the text background.  Range is [0 ... 255].


#### `fe.ListBox` ####

The class representing the listbox in Attract-Mode.  Instances of this 
class are returned by the `fe.add_listbox()` function and also appear in the 
`fe.objs` array (the Attract-Mode draw list).  This class cannot be 
otherwise instantiated in a script.

Attributes:   

   * `x` - Get/set x position of top left corner (in layout coordinates). 
   * `y` - Get/set y position of top left corner (in layout coordinates). 
   * `width` - Get/set width of listbox (in layout coordinates).
   * `height` - Get/set height of listbox (in layout coordinates).
   * `visible` - Get/set whether listbox is visible (boolean).  Default value
     is `true`.
   * `rotation` - Get/set rotation of listbox. Range is [0 ... 360].  Default
     value is 0.
   * `red` - Get/set red colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `green` - Get/set green colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `blue` - Get/set blue colour level for text. Range is [0 ... 255].
     Default value is 255.
   * `alpha` - Get/set alpha level for text. Range is [0 ... 255].  
     Default value is 255.
   * `index_offset` - Not used.
   * `bg_red` - Get/set red colour level for background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_green` - Get/set green colour level for background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_blue` - Get/set blue colour level for background. Range is 
     [0 ... 255].  Default value is 0.
   * `bg_alpha` - Get/set alpha level for background. Range is [0 ... 
     255].  Default value is 0 (transparent).
   * `sel_red` - Get/set red colour level for selection text. Range is 
     [0 ... 255].  Default value is 255.
   * `sel_green` - Get/set green colour level for selection text. Range is 
     [0 ... 255].  Default value is 255.
   * `sel_blue` - Get/set blue colour level for selection text. Range is 
     [0 ... 255].  Default value is 0.
   * `sel_alpha` - Get/set alpha level for selection text. Range is 
     [0 ... 255].  Default value is 255.
   * `selbg_red` - Get/set red colour level for selection background. Range 
     is [0 ... 255].  Default value is 0.
   * `selbg_green` - Get/set green colour level for selection background. 
     Range is  [0 ... 255].  Default value is 0.
   * `selbg_blue` - Get/set blue colour level for selection background. 
     Range is [0 ... 255].  Default value is 255.
   * `selbg_alpha` - Get/set alpha level for selection background. Range is 
     [0 ... 255].  Default value is 255.
   * `rows` - Get/set the number of listbox rows.  Default value is 11.
   * `charsize` - Get/set the forced character size.  If this is <= 0 
     then Attract-Mode will autosize based on the value of `height`/`rows`.
     Default value is -1.
   * `style` - Get/set the text style.  Can be a combination of one or more
     of the following (i.e. `Style.Bold | Style.Italic`):
      - `Style.Regular` (default)
      - `Style.Bold`
      - `Style.Italic`
      - `Style.Underlined`
   * `align` - Get/set the text alignment.  Can be one of the following 
     values:
      - `Align.Centre` (default)
      - `Align.Left`
      - `Align.Right`
   * `sel_style` - Get/set the selection text style.  Can be a combination 
     of one or more of the following (i.e. `Style.Bold | Style.Italic`):
      - `Style.Regular` (default)
      - `Style.Bold`
      - `Style.Italic`
      - `Style.Underlined`
   * `align` - Get/set the text alignment.  Can be one of the following 

Functions:

   * `set_rgb( r, g, b )` - Set the red, green and blue colour values for the 
     text.  Range is [0 ... 255].
   * `set_bg_rgb( r, g, b )` - Set the red, green and blue colour values for 
     the text background.  Range is [0 ... 255].
   * `set_sel_rgb( r, g, b )` - Set the red, green and blue colour values 
     for the selection text.  Range is [0 ... 255].
   * `set_selbg_rgb( r, g, b )` - Set the red, green and blue colour values 
     for the selection background.  Range is [0 ... 255].


#### `fe.Sound` ####

The class representing a sound sample.  Instances of this class are returned 
by the `fe.add_sound()` function.  This class cannot be otherwise 
instantiated in a script.

Attributes:   

   * `is_playing` - Get whether the sound is currently playing (boolean). 
   * `pitch` - Get/set the sound's pitch (float). Default value is 1.
   * `x` - Get/set the x position of the sound.  Default value is 0.
   * `y` - Get/set the y position of the sound.  Default value is 0.
   * `z` - Get/set the z position of the sound.  Default value is 0.

Functions:

   * `play()` - Play the sound.


Constants
---------

#### `FeVersion` [string] ####
#### `FeVersionNum` [int] ####

The current Attract-Mode version.

#### `ScreenWidth` [int] ####
#### `ScreenHeight` [int] ####

The screen width and height in pixels.

#### `ScreenSaverActive` [bool] ####

Whether the screen saver is active.

#### `OS` [string] ####

The Operating System that Attract-Mode is running under.  Will be one of:
"Windows", "OSX", "FreeBSD", "Linux" or "Unknown"
