/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2017 Andrew Mickelson
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

#ifndef FE_FILE_HPP
#define FE_FILE_HPP

#include <string>
#include <SFML/System/InputStream.hpp>
#include <cstdio>

class FeFileInputStream : public sf::InputStream
{
public:
	FeFileInputStream( const std::string &fn );
	~FeFileInputStream();

	sf::Int64 read( void *data, sf::Int64 size );
	sf::Int64 seek( sf::Int64 pos );
	sf::Int64 tell();
	sf::Int64 getSize();

private:
	FILE *m_file;
};

#endif
