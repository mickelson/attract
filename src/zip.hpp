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
#include <cstdint>

typedef void *(*FE_ZIP_ALLOC_CALLBACK) ( size_t );
bool fe_zip_open_to_buff(
	const char *archive,
	const char *filename,
	std::vector< char > &buff );

bool fe_zip_get_dir(
	const char *archive,
	std::vector<std::string> &result );

//
// Gather files with the specified basename from archive contents
//
// if an ext_filter is provided, files matching the ext_filter are returned
// in "in_list", otherwise they are returned in "out_list"
// listed extensions are matched
//
void gather_archive_filenames_with_base(
        std::vector < std::string > &in_list,
        std::vector < std::string > &out_list,
        const std::string &archive,
        const std::string &basename,
        const char **ext_filter=NULL );

// single file version of the above
// return true if found
bool get_archive_filename_with_base(
        std::string &filename,
        const std::string &archive,
        const std::string &basename,
        const char **ext_filter=NULL );


extern const char *FE_ARCHIVE_EXT[];
bool is_supported_archive( const std::string & );

class FeZipStream : public sf::InputStream
{
public:
	FeZipStream();
	FeZipStream( const std::string &archive );

	~FeZipStream();

	FeZipStream( const FeZipStream& ) = delete;
	FeZipStream& operator=( const FeZipStream& ) = delete;

	bool open( const std::string &filename );
	std::optional<std::size_t> read( void *data, std::size_t size ) override;

	std::optional<std::size_t> seek( std::size_t position ) override;
	std::optional<std::size_t> tell() override;
	std::optional<std::size_t> getSize() override;
	void setArchive( const std::string &archive );
	char *getData();

private:
	void clear();

	std::string m_archive;
	std::vector < char > m_data;
	std::size_t m_pos;
};

#endif
