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

#ifndef FE_UTIL_SQ_HPP
#define FE_UTIL_SQ_HPP

#include <squirrel.h>
#include <string>

//
// Squirrel utility functions.
//
bool fe_get_object_string(
	HSQUIRRELVM vm,
	HSQOBJECT obj,
	std::string &out_string);

bool fe_get_attribute_string(
	HSQUIRRELVM vm,
	HSQOBJECT obj,
	const std::string &key,
	const std::string &attribute,
	std::string & out_string);

int fe_obj_compare(
	HSQUIRRELVM vm,
	HSQOBJECT obj1,
	HSQOBJECT obj2 );

#endif
