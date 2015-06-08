// array.h	-- Thatcher Ulrich <tu@tulrich.com> 2003, Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Action Script Array implementation code for the gameswf SWF player library.


#ifndef GAMESWF_AS_ARRAY_H
#define GAMESWF_AS_ARRAY_H

#include "gameswf/gameswf_action.h"	// for as_object
#include "gameswf/gameswf_function.h"

namespace gameswf
{

	// constructor of an Array object
	void	as_global_array_ctor(const fn_call& fn);

	// this is an Array object
	struct as_array : public as_object
	{
		// Unique id of a gameswf resource
		enum { m_class_id = AS_ARRAY };
		virtual bool is(int class_id) const
		{
			if (m_class_id == class_id) return true;
			else return as_object::is(class_id);
		}

		exported_module virtual bool	get_member(const tu_stringi& name, as_value* val);
		exported_module virtual bool	set_member(const tu_stringi& name, const as_value& val);
		virtual void clear_refs(hash<as_object*, bool>* visited_objects, as_object* this_ptr);

		exported_module as_array(player* player);
		exported_module virtual const char* to_string();
		exported_module void push(const as_value& val) { m_array.push_back(val); }
		exported_module void remove(int index) { m_array.remove(index); }
		exported_module void insert(int index, const as_value& val)	{ m_array.insert(index, val); }
		exported_module void sort(int options, as_function* compare_function);
		exported_module int size() const { return m_array.size(); }
		exported_module void resize(int size) { m_array.resize(size); }

		tu_string m_string_value;
		array<as_value> m_array;
	};

	// this is "_global.Array" object
	struct as_global_array : public as_c_function
	{
		enum option
		{
			CASEINSENSITIVE = 1,
			DESCENDING = 2,
			UNIQUESORT = 4,
			RETURNINDEXEDARRAY = 8,
			NUMERIC = 16
		};

		as_global_array(player* player);
	};

}	// end namespace gameswf


#endif // GAMESWF_AS_ARRAY_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
