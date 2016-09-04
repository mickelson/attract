///////////////////////////////////////////////////
//
// Attract-Mode Frontend - KeyboardSearch plugin
//
///////////////////////////////////////////////////
//
class UserConfig </ help="A plugin that enables simple keyboard searches" /> {

	</ label="Trigger",
		help="The action that triggers a search.  Keys can be mapped to these under Attract-Mode's 'Controls' configuration",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6",
		order=1 />
	trigger="Custom1";

	</ label="Results Mode",
		help="Configure how search results should be handled",
		options="Jump to Next Match,Show Results",
		order=2 />
	results_mode="Show Results";
}

class KeyboardSearch
{
	_last_search="";
	_trigger="custom1";
	_my_config=null;

	constructor()
	{
		fe.add_signal_handler( this, "on_signal" )
		_my_config=fe.get_config();
		_trigger=_my_config["trigger"].tolower();
	}

	function _massage( str )
	{
		local words = split( str, " " );

		local temp="";
		foreach ( w in words )
		{
			if ( temp.len() > 0 )
				temp += " ";

			local f = w.slice( 0, 1 );
			temp += "["+f.toupper()+f.tolower()+"]"+w.slice(1);
		}

		return temp;
	}

	function _select( emu, game )
	{
		for ( local i=0; i<fe.list.size; i++ )
		{
			if (( fe.game_info( Info.Emulator, i ) == emu )
					&& ( fe.game_info( Info.Name, i ) == game ))
			{
				fe.list.index += i;
				return true;
			}
		}

		return false;
	}

	function on_signal( signal )
	{
		if ( signal == _trigger )
		{
			_last_search = fe.overlay.edit_dialog(
				"Search for:",
				_last_search );

			if ( _my_config["results_mode"] == "Show Results" )
			{
				local sel_emu = fe.game_info( Info.Emulator );
				local sel_game = fe.game_info( Info.Name );

				if ( _last_search.len() < 1 )
					fe.list.search_rule = "";
				else
				{
					fe.list.search_rule = "Title contains "
						+ _massage( _last_search );

					_select( sel_emu, sel_game );
				}

				return true;
			}

			// "Jump to Next Result" mode
			//
			local temp = _last_search.tolower();

			local s = fe.filters[fe.list.filter_index].size;

			for ( local i=1; i<s; i++ )
			{
				local name = fe.game_info(Info.Title,i).tolower();
				if ( name.len() < temp.len() )
					continue;

				// find() doesn't seem to work if the searched for
				// text happens at the very front of the string (?)
				if (( name.slice(0,temp.len())==temp )
					|| (name.find( temp ) ))
				{
					// found match
					fe.list.index = (fe.list.index+i)%s;
					break;
				}
			}
			return true;
		}
		else if (( fe.list.search_rule.len() > 0 )
				&& ( signal == "back" ))
		{
			//
			// Clear the search rule when user selects "back"
			//
			// Keep the currently selected game from the search as the selection when
			// we back out
			//
			local sel_emu = fe.game_info( Info.Emulator );
			local sel_game = fe.game_info( Info.Name );

			fe.list.search_rule = "";

			_select( sel_emu, sel_game );
			return true;
		}

		return false;
	}
}

fe.plugin[ "KeyboardSearch" ] <- KeyboardSearch();
