/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013 Andrew Mickelson
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

#ifndef FE_LISTXML_HPP
#define FE_LISTXML_HPP

#include <string>
#include <list>
class FeRomInfo;

class FeListXMLParse
{
public:
	typedef void (*StartElementHandler) (void *, const char *, const char ** );
	typedef void (*EndElementHandler) (void *, const char * );

	typedef void (*UiUpdate) (void *, int);

private:
	friend void exp_start_element_mame( void *, const char *, const char ** );
	friend void exp_start_element_mess( void *, const char *, const char ** );
	friend void exp_handle_data( void *, const char *, int );
	friend void exp_end_element_mame( void *, const char * );
	friend void exp_end_element_mess( void *, const char * );

	std::list<FeRomInfo> &m_romlist;
	std::list<FeRomInfo>::iterator m_itr;
	UiUpdate m_ui_update;
	void * m_ui_update_data;

	// "Parse state" variables:
	//
	bool m_element_open;				// both
	bool m_keep_rom;					// both
	bool m_collect_data;				// mame only
	std::string m_current_data;  	// both
	std::string m_name;				// mess only
	std::string m_description;		// mess only
	std::string m_year;				// mess only
	std::string m_man;				// mess only
	std::string m_fuzzydesc;		// mess only

	void clear_parse_state();

	void start_element_mame( const char *, const char ** );
	void start_element_mess( const char *, const char ** );
	void handle_data( const char *, int );
	void end_element_mame( const char * );
	void end_element_mess( const char * );

	bool parse_internal( StartElementHandler, EndElementHandler, 
						const std::string & );

public:
	FeListXMLParse( std::list<FeRomInfo> &, UiUpdate u=NULL, void *d=NULL );

	void parse_mame( const std::string &base_command );
	void parse_mess( const std::string &command );
};

#endif
