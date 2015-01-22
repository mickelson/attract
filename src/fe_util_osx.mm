/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2014 Andrew Mickelson
 *
 *  This file is part of Attract-Mode.
 *
 *  Attract-Mode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Attract-Mode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fe_util_osx.hpp"
#include <Cocoa/Cocoa.h>
#include <SFML/System/Utf.hpp>

namespace
{
	class cocoa_ar_pool_class
	{
	public:
		cocoa_ar_pool_class()
		{
			pool = [[NSAutoreleasePool alloc] init];
		}

		~cocoa_ar_pool_class()
		{
			[pool release];
		}
	
	private:
		NSAutoreleasePool* pool;
	};

	static std::basic_string<sf::Uint32> string_from_cocoa( NSString* s )
	{
		std::string t1 = std::string( [s UTF8String] );

		std::basic_string<sf::Uint32> t2, retval;
		sf::Utf8::toUtf32( t1.begin(),
			t1.end(),
			std::back_inserter( t2 ) );

		// clean the string
		for ( std::basic_string<sf::Uint32>::iterator itr=t2.begin();
				itr != t2.end();
				++itr )
		{
			if ( *itr >= 32 )
				retval += *itr;
		}

		return retval;
	}
};

void osx_hide_menu_bar()
{
	[NSMenu setMenuBarVisible:NO];
}

std::basic_string<sf::Uint32> osx_clipboard_get_content()
{
	cocoa_ar_pool_class pool;

	NSPasteboard* pboard = [NSPasteboard generalPasteboard];
	NSString* nstext = [pboard stringForType:NSPasteboardTypeString];
	return string_from_cocoa( nstext );
}
