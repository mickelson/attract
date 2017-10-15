///////////////////////////////////////////////////
//
// Attract-Mode Frontend - SpecificDisplay plugin
//
///////////////////////////////////////////////////
class UserConfig </ help="Load a specific display on custom input" /> {

	</ label="Display to Show", help="The display to be shown when input is recieved", order=1, options="" />
	display="";

	</ label="Trigger",
		help="The custom command that will show this display (see also: Config->Controls->Costom#)",
		order=2, options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6" />
	trigger="";
};

local dattr = UserConfig.getattributes("display");
foreach ( d in fe.displays )
{
	if ( dattr["options"].len() < 1 )
		dattr["options"] = d.name;
	else
		dattr["options"] += "," + d.name;
}

UserConfig.setattributes("display", dattr);

class SpecificDisplay
{
	trigger = "";
	display_index = 0;

	constructor()
	{
		local my_config = fe.get_config();

		trigger = my_config["trigger"].tolower();

		for ( local i=0; i<fe.displays.len(); i++ )
		{
			if ( fe.displays[i].name == my_config[ "display" ] )
			{
				display_index = i;
				break;
			}
		}

		if ( display_index == -1 )
		{
			print( "SpecificDisplay plugin: could not find configured display: "
				+ my_config["display"] + "\n" );
			display_index = 0;
		}

		fe.add_signal_handler( this, "on_signal" );
	}

	function on_signal( signal_str )
	{
		if ( signal_str == trigger )
			fe.set_display( display_index );

		return false;
	}
}

fe.plugin[ "SpecificDisplay" ] <- SpecificDisplay();

