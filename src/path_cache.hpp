/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2020 Andrew Mickelson
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

#ifndef PATH_CACHE_HPP
#define PATH_CACHE_HPP

#include <string>
#include <vector>
#include <map>

class FePathCache
{
public:
	FePathCache();
	~FePathCache();

	void clear();

	bool get_filename_from_base(
		std::vector<std::string> &in_list,
		std::vector<std::string> &out_list,
		const std::string &path,
		const std::string &base_name,
		const char **filter );

private:
	std::map< std::string, std::vector<std::string> > m_cache;

	FePathCache( FePathCache & );
	FePathCache &operator=( FePathCache & );

	std::vector < std::string > &get_cache( const std::string &path );
};

#endif
