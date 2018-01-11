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

#include "fe_file.hpp"

FeFileInputStream::FeFileInputStream( const std::string &fn )
	: m_file( NULL )
{
	m_file = fopen( fn.c_str(), "rb" );
}

FeFileInputStream::~FeFileInputStream()
{
	if ( m_file )
		fclose( m_file );
}

sf::Int64 FeFileInputStream::read( void *data, sf::Int64 size )
{
	if ( m_file )
		return fread( data, 1, (size_t)size, m_file );

	return -1;
}

sf::Int64 FeFileInputStream::seek( sf::Int64 pos )
{
	if ( m_file )
	{
		if ( fseek( m_file, (size_t)pos, SEEK_SET ) )
			return -1;

		return tell();
	}

	return -1;
}

sf::Int64 FeFileInputStream::tell()
{
	if ( m_file )
		return ftell( m_file );

	return -1;
}

sf::Int64 FeFileInputStream::getSize()
{
	if ( m_file )
	{
		sf::Int64 pos = tell();
		fseek( m_file, 0, SEEK_END );
		sf::Int64 size = tell();
		seek( pos );
		return size;
	}

	return -1;
}
