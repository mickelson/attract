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

#include "fe_util_sq.hpp"
#include <sqrat.h>

bool fe_get_object_string(
	HSQUIRRELVM vm,
	HSQOBJECT obj,
	std::string &out_string )
{
	sq_pushobject(vm, obj);
	sq_tostring(vm, -1);
	const SQChar *s;
	SQRESULT res = sq_getstring(vm, -1, &s);
	bool r = SQ_SUCCEEDED(res);
	if (r)
		out_string = s;

	sq_pop(vm,2);
	return r;
}

bool fe_get_attribute_string(
	HSQUIRRELVM vm,
	HSQOBJECT obj,
	const std::string &key,
	const std::string &attribute,
	std::string & out_string )
{
	sq_pushobject(vm, obj);

	if ( key.empty() )
		sq_pushnull(vm);
	else
		sq_pushstring(vm, key.c_str(), key.size());

	if ( SQ_FAILED( sq_getattributes(vm, -2) ) )
	{
		sq_pop(vm, 2);
		return false;
	}

	HSQOBJECT att_obj;
	sq_resetobject( &att_obj );
	sq_getstackobj( vm, -1, &att_obj );

	Sqrat::Object attTable( att_obj, vm );
	sq_pop( vm, 2 );

	if ( attTable.IsNull() )
		return false;

	Sqrat::Object attVal = attTable.GetSlot( attribute.c_str() );
	if ( attVal.IsNull() )
		return false;

	return fe_get_object_string( vm, attVal.GetObject(), out_string );
}
