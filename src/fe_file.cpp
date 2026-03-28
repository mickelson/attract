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
#include "nowide/cstdio.hpp"

FeFileInputStream::FeFileInputStream( const std::string &fn )
	: m_file( NULL )
{
	m_file = nowide::fopen( fn.c_str(), "rb" );
}

FeFileInputStream::~FeFileInputStream()
{
	if ( m_file )
		fclose( m_file );
}

std::optional<std::size_t> FeFileInputStream::read( void *data, std::size_t size )
{
	if ( m_file )
		return fread( data, 1, size, m_file );

	return std::nullopt;
}

std::optional<std::size_t> FeFileInputStream::seek( std::size_t pos )
{
	if ( m_file )
	{
		if ( fseek( m_file, (long)pos, SEEK_SET ) )
			return std::nullopt;

		return tell();
	}

	return std::nullopt;
}

std::optional<std::size_t> FeFileInputStream::tell()
{
	if ( m_file )
		return ftell( m_file );

	return std::nullopt;
}

std::optional<std::size_t> FeFileInputStream::getSize()
{
	if ( m_file )
	{
		auto pos = tell();
		if (pos == std::nullopt)
			return std::nullopt;
		fseek( m_file, 0, SEEK_END );
		auto size = tell();
		seek(*pos);
		return size;
	}

	return std::nullopt;
}
