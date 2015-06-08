// array.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003, Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#include "gameswf/gameswf_as_classes/as_array.h"
#include "gameswf/gameswf_function.h"
#include "gameswf/gameswf_character.h"

namespace gameswf
{

	// public join([delimiter:String]) : String
	// delimiter:String [optional] - A character or string that separates array elements in the returned string.
	// If you omit this parameter, a comma (,) is used as the default separator.
	void	as_array_join(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		const char* delimiter = fn.nargs > 0 ? fn.arg(0).to_string() : ",";
		if (a)
		{
			tu_string result;
			for (int index = 0; index < a->size(); ++index)
			{
				result += a->m_array[index].to_tu_string();
				if (index < a->size() - 1)
				{
					result += delimiter;
				}
			}
			fn.result->set_tu_string(result);
		}
	}

	void	as_array_tostring(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			fn.result->set_tu_string(a->to_string());
		}
	}

	// Adds one or more elements to the end of an array and returns the new length of the array.
	void	as_array_push(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			for (int i = 0; i < fn.nargs; i++)
			{
				a->push(fn.arg(i));
			}
			fn.result->set_int(a->size());
		}
	}

	// remove the first item of array and returns the value of that element.
	void	as_array_shift(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a && a->size() > 0)
		{
			*fn.result = a->m_array[0];
			a->remove(0);
		}
	}

	//Removes the last element from an array and returns the value of that element.
	void	as_array_pop(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a && a->size() > 0)
		{
			*fn.result = a->m_array[a->size() - 1];
			a->resize(a->size() - 1);
		}
	}


	// public splice(startIndex:Number, [deleteCount:Number], [value:Object]) : Array
	// adds elements to and removes elements from an array.
	// This method modifies the array without making a copy.
	void	as_array_splice(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a && fn.nargs >= 1)
		{
			// startIndex:Number - An integer that specifies the index of the element in the array
			// where the insertion or deletion begins.
			// You can specify a negative integer to specify a position relative to the end of the array 
			// (for example, -1 is the last element of the array).
			int index = fn.arg(0).to_int() >= 0 ? fn.arg(0).to_int() : a->size() + fn.arg(0).to_int();
			if (index >= 0 && index < a->size())
			{
				// Returns an array containing the elements that were removed from the original array.
				as_array* deleted_items = new as_array(a->get_player());
				fn.result->set_as_object(deleted_items);

				// first delete items

				// If no value is specified for the deleteCount parameter,
				// the method deletes all of the values from the startIndex element to the last element 
				int delete_count = a->size() - index;

				if (fn.nargs >= 2 && fn.arg(1).to_int() < delete_count && fn.arg(1).to_int() >= 0)
				{
					// If the value is 0, no elements are deleted.
					delete_count = fn.arg(1).to_int();
				}
				for (int i = 0; i < delete_count; i++)
				{
					deleted_items->push(a->m_array[index]);
					a->remove(index);
				}

				// then insert items

				if (fn.nargs >= 3)
				{
					// Specifies the values to insert into the array at the insertion point specified in the startIndex parameter.
					const as_value& val = fn.arg(2);

					as_array* obj = cast_to<as_array>(val.to_object());
					if (obj)
					{
						// insert an array
						for (int i = obj->size() - 1; i >= 0; i--)
						{
							a->insert(index, obj->m_array[i]);
						}
					}
					else
					{
						// insert an item
						a->insert(index, val);
					}
				}
			}
		}
	}

	// public concat([value:Object]) : Array
	// Concatenates the elements specified in the parameters with the elements
	// in an array and creates a new array. 
	// If the value parameters specify an array, the elements of that array are concatenated,
	// rather than the array itself. The array my_array is left unchanged.
	// If you don't pass any values, a duplicate of an array is created.
	void	as_array_concat(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			as_array* res = new as_array(a->get_player());
			fn.result->set_as_object(res);

			// duplicate an array
			res->resize(a->size());
			for (int i = 0; i < a->size(); i++)
			{
				res->m_array[i] = a->m_array[i];
			}

			for (int i = 0; i < fn.nargs; i++)
			{
				as_array* arg = cast_to<as_array>(fn.arg(i).to_object());
				if (arg)
				{
					// concat an array
					for (int j = 0, n = arg->size(); j < n; j++)
					{
						res->push(arg->m_array[j]);
					}
				}
				else
				{
					res->push(fn.arg(i));
				}
			}
		}
	}

	// public slice([startIndex:Number], [endIndex:Number]) : Array
	// Returns a new array that consists of a range of elements from the original array,
	// without modifying the original array. The returned array includes the startIndex 
	// element and all elements up to, but not including, the endIndex element. 
	// If you don't pass any parameters, a duplicate of the original array is created.
	void	as_array_slice(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			as_array* res = new as_array(a->get_player());
			fn.result->set_as_object(res);

			// If you don't pass any parameters, a duplicate of the original array is created.
			if (fn.nargs == 0)
			{
				res->resize(a->size());
				for (int i = 0; i < a->size(); i++)
				{
					res->m_array[i] = a->m_array[i];
				}
				return;
			}

			// start index
			int start = fn.arg(0).to_int() >= 0 ? fn.arg(0).to_int() : a->size() + fn.arg(0).to_int();

			// If you omit this parameter, the slice includes all elements
			int end = a->size();
			if (fn.nargs >= 2)
			{
				end = fn.arg(1).to_int() >= 0 ? fn.arg(1).to_int() : a->size() + fn.arg(1).to_int();
				end = imin(end, a->size());
			}

			if (start >= 0 && start < a->size())
			{
				for (int i = start; i < end; i++)
				{
					res->push(a->m_array[i]);
				}
			}
		}
	}

	// public unshift(value:Object) : Number
	// Adds one or more elements to the beginning of an array and returns the new length
	void	as_array_unshift(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			for (int i = fn.nargs - 1; i >= 0; i--)
			{
				as_array* arg = cast_to<as_array>(fn.arg(i).to_object());
				if (arg)
				{
					for (int j = arg->size() - 1; j >= 0; j--)
					{
						as_value val;
						if (arg->get_member(tu_string(j), &val))
						{
							a->insert(0, val);
						}
					}
				}
				else
				{
					a->insert(0, fn.arg(i));
				}
			}
			fn.result->set_int(a->size());
		}
	}

	// public sort([compareFunction:Object], [options:Number]) : Array
	// Sorts the elements in an array according to Unicode values.
	void	as_array_sort(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			int options = 0;
			as_function* compare_function = NULL;

			if (fn.nargs > 0)
			{
				if (fn.arg(0).is_number())
				{
					options = fn.arg(0).to_int();
				}
				else
				{
					compare_function = fn.arg(0).to_function();
				}

				if (fn.nargs > 1)
				{
					if (fn.arg(1).is_number())
					{
						options = fn.arg(1).to_int();
					}
					else
					{
						compare_function = fn.arg(1).to_function();
					}
				}
			}
			a->sort(options, compare_function);
		}
	}

	void	as_array_length(const fn_call& fn)
	{
		as_array* a = cast_to<as_array>(fn.this_ptr);
		if (a)
		{
			fn.result->set_int(a->size());
		}
	}

	void	as_global_array_ctor(const fn_call& fn)
	// Constructor for ActionScript class Array.
	{
		gc_ptr<as_array>	ao = new as_array(fn.get_player());

		// case of "var x = ["abc","def", 1,2,3,4,..];"
		// called from "init array" operation only
		if (fn.nargs == -1 && fn.first_arg_bottom_index == -1)
		{

			// Use the arguments as initializers.
			int	size = fn.env->pop().to_int();
			ao->resize(size);
			for (int i = 0; i < size; i++)
			{
				ao->m_array[i] = fn.env->pop();
			}
		}
		else

		// case of "var x = new Array(777)"
		if (fn.nargs == 1)
		{
			// Create an empty array with the given number of undefined elements.
			int size = fn.arg(0).to_int();
			ao->resize(size);
		}
		else

		// case of "var x = new Array(1,2,3,4,5,6,7,8,..);"
		{
			assert(fn.env);

			// Use the arguments as initializers.
			ao->resize(fn.nargs);
			for (int i = 0; i < fn.nargs; i++)
			{
				ao->m_array[i] = fn.arg(i);
			}
		}

		fn.result->set_as_object(ao.get_ptr());
	}

	as_global_array::as_global_array(player* player) :
		as_c_function(player, as_global_array_ctor)
	{
		builtin_member("CASEINSENSITIVE", CASEINSENSITIVE);
		builtin_member("DESCENDING", DESCENDING);
		builtin_member("UNIQUESORT", UNIQUESORT);
		builtin_member("RETURNINDEXEDARRAY ", RETURNINDEXEDARRAY);
		builtin_member("NUMERIC", NUMERIC);
	}

	as_array::as_array(player* player) :
		as_object(player)
	{
		builtin_member("join", as_array_join);
		builtin_member("concat", as_array_concat);
		builtin_member("slice", as_array_slice);
		builtin_member("unshift", as_array_unshift);
		builtin_member("sort", as_array_sort);
		//			this->set_member("sortOn", &array_not_impl);
		//			this->set_member("reverse", &array_not_impl);
		builtin_member("shift", as_array_shift);
		builtin_member("toString", as_array_tostring);
		builtin_member("push", as_array_push);
		builtin_member("pop", as_array_pop);
		builtin_member("length", as_value(as_array_length, as_value()));
		builtin_member("splice", as_array_splice);

		set_ctor(as_global_array_ctor);
	}

	const char* as_array::to_string()
	{
		m_string_value = "";
		for (int i = 0; i < size(); i++)
		{
			m_string_value += m_array[i].to_tu_string();
			if (i < size() - 1)
			{
				m_string_value +=  ",";
			}
		}
		return m_string_value.c_str();
	}

	// By default, Array.sort() works as described in the following list:
	// Sorting is case-sensitive (Z precedes a). 
	// Sorting is ascending (a precedes b). 
	// Numeric fields are sorted as if they were strings, so 100 precedes 99
	void as_array::sort(int options, as_function* compare_function)
	{
		for (int i = 0; i < size() - 1; i++)
		{
			for (int j = i + 1; j < size(); j++)
			{
				bool do_swap = false;
				if (compare_function == NULL)
				{
					if (m_array[i].to_tu_string() > m_array[j].to_tu_string())
					{
						do_swap = true;
					}
				}
				else
				{
					// -1, if A should appear before B in the sorted sequence 
					// 0, if A equals B 
					// 1, if A should appear after B in the sorted sequence 

					// use _root environment
					character* mroot = get_player()->get_root_movie();
					as_environment* env = mroot->get_environment();
					
					// keep stack size
					int stack_size = env->get_stack_size();

					env->push(m_array[j]);
					env->push(m_array[i]);
					as_value ret = call_method(compare_function, env, this, 2, env->get_top_index());

					// restore stack size
					env->set_stack_size(stack_size);

					if (ret.to_int() > 0)
					{
						do_swap = true;
					}
				}

				if (options & as_global_array::DESCENDING)
				{
					do_swap = ! do_swap;
				}

				if (do_swap)
				{
					tu_swap(&m_array[i], &m_array[j]);
				}
			}
		}
	}

	bool	as_array::get_member(const tu_stringi& name, as_value* val)
	{
		int index;
		if (string_to_number(&index, name.c_str(), 10))
		{
			if (index >= 0 && index < size())
			{
				*val = m_array[index];
				return true;
			}
		}
		return as_object::get_member(name, val);
	}

	bool	as_array::set_member(const tu_stringi& name, const as_value& val)
	{
		int index;
		if (string_to_number(&index, name.c_str(), 10))
		{
			if (index >= 0)
			{
				if (index >= size())
				{
					resize(index + 1);
				}
				m_array[index] = val;
				return true;
			}
		}
		return as_object::set_member(name, val);
	}

	void as_array::clear_refs(hash<as_object*, bool>* visited_objects, as_object* this_ptr)
	{
		// Is it a reentrance ?
		if (visited_objects->get(this, NULL))
		{
			return;
		}

		// will be set in as_object::clear_refs
//		visited_objects->set(this, true);

		as_object::clear_refs(visited_objects, this_ptr);

		// clear display list
		for (int i = 0; i < size(); i++)
		{
			as_object* obj = m_array[i].to_object();
			if (obj)
			{
				obj->clear_refs(visited_objects, this_ptr);
			}
		}
	}

};
