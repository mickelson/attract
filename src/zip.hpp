/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2015 Andrew Mickelson
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

#ifndef FE_ZIP_HPP
#define FE_ZIP_HPP

#include <string>
#include <vector>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/NonCopyable.hpp>

typedef void *(*FE_ZIP_ALLOC_CALLBACK) ( size_t );
bool fe_zip_open_to_buff(
        const char *archive,
        const char *filename,
        FE_ZIP_ALLOC_CALLBACK callback,
        void **buff,
        size_t *buff_size );

bool fe_zip_get_dir(
        const char *archive,
	std::vector<std::string> &result );

extern const char *FE_ARCHIVE_EXT[];
bool is_supported_archive( const std::string & );

class FeZipStream : public sf::InputStream, sf::NonCopyable
{
public:
	FeZipStream();
	FeZipStream( const std::string &archive );

	~FeZipStream();

	bool open( const std::string &filename );
	sf::Int64 read( void *data, sf::Int64 size );

	sf::Int64 seek( sf::Int64 position );
	sf::Int64 tell();
	sf::Int64 getSize();
	void setArchive( const std::string &archive );
	char *getData();

private:
	void clear();

	std::string m_archive;
	char *m_data;
	sf::Int64 m_pos;
	sf::Int64 m_size;
};

#endif
