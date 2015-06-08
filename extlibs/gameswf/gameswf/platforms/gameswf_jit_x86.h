// gameswf_jit_x86.h	-- Julien Hamaide <julien.hamaide@gmail.com>	2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Jit x86

#ifndef GAMESWF_JIT_X86_H
#define GAMESWF_JIT_X68_H

#include "gameswf_jit.h"

enum jit_register
{   
	jit_eax = 0,
	jit_ebx = 3,
	jit_ecx = 1,
	jit_edx = 2,
	jit_esi = 6,
	jit_edi = 7,
	jit_ebp = 5,
	jit_esp = 4
};

template<typename T> void* cast_to_voidp( T value )
{
	union
	{
		T _value;
		void * _voidp;
	} cast;

	cast._value = value;
	return cast._voidp;
}

struct jit_register_offset_address
{
	jit_register_offset_address( jit_register reg, int offset ) : m_register( reg ), m_offset( offset ) {}

	jit_register m_register;
	int m_offset;
};

#define jit_this_pointer jit_ecx
#define jit_result  jit_eax
#define jit_stack_pointer jit_esp

#define jit_push( _function_, _variable_ ) { jit_add_bytecode_u8( _function_, 0x50 | _variable_ ); }

#define jit_pop( _function_, _variable_ ) { jit_add_bytecode_u8( _function_,  0x50 | _variable_ + 8 ); }

#define jit_mov( _function_, _register1_, _register2_ ) jit_mov_implementation( _function_, _register1_, _register2_ )
#define jit_load( _function_, _register_, _memory_ ) jit_load_implementation( _function_, _register_, _memory_ ); 
#define jit_store( _function_, _register_, _memory_ )

#define jit_subi( _function_, _register_, _value_ ) jit_sub_implementation( _function_, _register_, _value_ )

#define jit_getarg( _index_ ) jit_register_offset_address( jit_ebp, 4 * ( 2 + _index_ ) )
#define jit_pusharg( _function_, _arg_ ) jit_push( _function_, _arg_ )
#define jit_pushargi( _function_, _arg_ ) jit_pushi( _function_, _arg_ )
#define jit_popargs( _function_, _arg_ ) jit_subi( _function_, jit_esp, 4 * _arg_ )

#define jit_this_call( _function_, _address_ ) { jit_call( _function_, _address_);}
#define jit_call( _function_, _address_ ) { jit_add_bytecode_u8( _function_, 0xE8 ); _function_.add_address_patch( cast_to_voidp(_address_), 4 );}
#define jit_ret( _function_ ) jit_add_bytecode_u8( _function_, 0xC3 )

#define jit_add_bytecode_u8( _function_, _value_ ) { _function_.push_byte( static_cast<unsigned char>( _value_ ) ); }
#define jit_add_bytecode_u32( _function_, _value_ ) { _function_.push_integer( (unsigned int)( _value_ ) ); }

#define jit_getaddress( _function_, _destination_, _source_, _offset_ ) jit_lea( _function_, _destination_, jit_register_offset_address( _source_, _offset_) )

// Implementations 

void jit_mov_implementation( jit_function & function, const jit_register destination, const jit_register source );
void jit_load_implementation( jit_function & function, const jit_register destination, const jit_register_offset_address & address );
void jit_sub_implementation( jit_function & function, const jit_register destination, const int value );
void jit_lea( jit_function & function, const jit_register destination, const jit_register_offset_address & address );

void jit_pushi( jit_function & function, const void * pointer );
void jit_pushi( jit_function & function, int value );
void jit_pushi( jit_function & function, uint8 value );
void jit_pushi( jit_function & function, double value );


#endif