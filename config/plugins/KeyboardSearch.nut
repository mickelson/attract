///////////////////////////////////////////////////
//
// Attract-Mode Frontend - KeyboardSearch plugin
//
///////////////////////////////////////////////////
//
class UserConfig </ help="A plugin that enables simple keyboard searches" /> {

	</ label="Trigger",
		help="Input that triggers a search",
		options="Custom1,Custom2,Custom3,Custom4,Custom5,Custom6",
		order=1 />
	trigger="Custom1";
}

class KeyboardSearch
{
	_last_search="";
	_trigger="custom1";

	constructor()
	{
		fe.add_signal_handler( this, "on_signal" )
		_trigger=fe.get_config()["trigger"].tolower();
	}

	function on_signal( signal )
	{
		if ( signal == _trigger )
		{
			_last_search = fe.overlay.edit_dialog(
				"Search for:",
				_last_search ).tolower();

			local s = fe.filters[fe.list.filter_index].size;

			for ( local i=1; i<s; i++ )
			{
				local name = fe.game_info(Info.Title,i).tolower();
				if ( name.len() < _last_search.len() )
					continue;

				// find() doesn't seem to work if the searched for
				// text happens at the very front of the string (?)
				if (( name.slice(0,_last_search.len())==_last_search )
					|| (name.find( _last_search ) ))
				{
					// found match
					fe.list.index = (fe.list.index+i)%s;
					return true;
				}
			
			}

			return true;
		}

		return false;
	}
}

fe.plugin[ "KeyboardSearch" ] <- KeyboardSearch();
