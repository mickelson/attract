/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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

#ifndef FE_XML_HPP
#define FE_XML_HPP

#include <string>
#include <set>
#include <map>
#include <vector>
#include "scraper_base.hpp"

class FeXMLParser
{
public:
	void set_continue_parse( bool f ) { m_continue_parse=f; };
	bool get_continue_parse() { return m_continue_parse; };

protected:
	friend void exp_handle_data( void *, const char *, int );
	friend void exp_start_element( void *, const char *, const char ** );
	friend void exp_end_element( void *, const char * );

	void handle_data( const char *, int );
	virtual void start_element( const char *, const char ** )=0;
	virtual void end_element( const char * )=0;

	UiUpdate m_ui_update;
	void * m_ui_update_data;

	// "Parse state" variables:
	//
	bool m_element_open;
	bool m_keep_rom;
	bool m_continue_parse;
	std::string m_current_data;

protected:
	FeXMLParser( UiUpdate u=NULL, void *d=NULL );
	FeXMLParser( const FeXMLParser & );
	FeXMLParser &operator=( const FeXMLParser & );

	bool parse_internal( const std::string &, const std::string &, const std::string & );
};

class FeMapComp
{
public:
	bool operator()(const char *lhs, const char *rhs) const;
};

class FeListXMLParser : public FeXMLParser
{
public:
	FeListXMLParser( FeImporterContext &ctx );

	bool parse_command( const std::string &base_command, const std::string &work_dir );
	bool parse_file( const std::string &filename );

	std::vector<std::string> get_sl_extensions() { return m_sl_exts; };

private:
	FeImporterContext &m_ctx;
	FeRomInfoListType::iterator m_itr;
	std::map<const char *, FeRomInfoListType::iterator, FeMapComp> m_map;
	std::vector<FeRomInfoListType::iterator> m_discarded;
	int m_count;
	int m_displays;
	bool m_collect_data;
	bool m_chd;
	bool m_mechanical;
	std::vector<std::string> m_sl_exts; // softlists: supported extensions

	void pre_parse();
	void post_parse();

	void start_element( const char *, const char ** );
	void end_element( const char * );
};

class FeListSoftwareParser : public FeXMLParser
{
public:
	FeListSoftwareParser( FeImporterContext &ctx );
	bool parse( const std::string &command, const std::string &work_dir,
		const std::vector < std::string > &system_names );

private:
	FeImporterContext &m_ctx;
	std::string m_name;
	std::string m_description;
	std::string m_year;
	std::string m_man;
	std::string m_cloneof;
	std::string m_altname;
	std::string m_alttitle;
	std::string m_crc;

	std::multimap<std::string, FeRomInfo *> m_crc_map;
	std::multimap<std::string, FeRomInfo *> m_fuzzy_map;

	void set_info_values( FeRomInfo &r, int score );

	void start_element( const char *, const char ** );
	void end_element( const char * );
	void clear_parse_state();
};

#endif
