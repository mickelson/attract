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

#ifndef FE_BASE_HPP
#define FE_BASE_HPP

#include <string>

extern const char *FE_IDENTITY;
extern const char *FE_NAME_AND_VERSION;
extern const char *FE_WHITESPACE;

class FeBaseConfigurable
{
protected:
	void invalid_setting( 
			const std::string &filename,
			const char *base, 
			const std::string &setting, 
			const char **valid1,
			const char **valid2=NULL,
			const char *label="setting" );

public:
	virtual int process_setting( const std::string &setting, 
								const std::string &value,
								const std::string &filename )=0;
};

class FeFileConfigurable : public FeBaseConfigurable
{
public:
	bool load_from_file( const std::string &filename,
								const char *sep=FE_WHITESPACE );
};

class FeSettings;
namespace sf { class Drawable; };

class FeBasePresentable
{
public:
	virtual ~FeBasePresentable();
	virtual void on_new_selection( FeSettings * );
	virtual void on_new_list( FeSettings * );
	virtual const sf::Drawable &drawable()=0;

	virtual bool tick( FeSettings * );
	virtual void set_play_state( bool play );
	virtual void set_vol( float volume );
};

#endif
