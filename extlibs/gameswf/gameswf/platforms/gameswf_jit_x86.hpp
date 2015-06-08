// gameswf_jit_x86.hpp	-- Julien Hamaide <julien.hamaide@gmail.com>	2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Jit x86

enum jit_x86_addressing_mode
{
	RegisterIndirect = 0,	//Except ESP
	Indexed8BitDisplacement = 1,  // Except ESP
	Indexed32BitDisplacement = 2, // Except ESP
	RegisterDirect = 3
};

enum jit_x86_scale_factor
{
	ScaleFactorOne = 0,
	ScaleFactorTwo = 1,
	ScaleFactorFour = 2,
	ScaleFactorEight = 3
};

struct jit_x86_modrm
{
	jit_register
m_rm:3;		// Source
	jit_register
m_register_opcode:3;  // Dest
	jit_x86_addressing_mode
m_mode:2;

	operator unsigned char()
	{
		return *( unsigned char *)this;
	}
};

struct jit_x86_sib
{
	jit_register
m_base_register:3,
m_index_register:3;
	jit_x86_scale_factor
m_scale:2;

	operator unsigned char()
	{
		return *( unsigned char *)this;
	}
};

void jit_mov_implementation( jit_function & function, const jit_register destination, const jit_register source )
{
	function.push_byte( 0x8b ); //mov Gv, Ev
	jit_x86_modrm modrm;

	modrm.m_register_opcode = destination;
	modrm.m_rm = source;
	modrm.m_mode = RegisterDirect;

	function.push_byte( modrm );
}

void jit_load_implementation( jit_function & function, const jit_register destination, const jit_register_offset_address & address )
{
	function.push_byte( 0x8b ); //mov Gv, Ev
	jit_x86_modrm modrm;

	modrm.m_register_opcode = destination;
	modrm.m_rm = address.m_register;
	if( address.m_offset > 127 || address.m_offset < -128 )
	{
		modrm.m_mode = Indexed32BitDisplacement;
		function.push_byte( modrm );
		function.push_integer( address.m_offset );
	}
	else
	{
		modrm.m_mode = Indexed8BitDisplacement;
		function.push_byte( modrm );
		function.push_byte( address.m_offset );
	}
}

void jit_sub_implementation( jit_function & function, const jit_register destination, const int value )
{

	if( value <= 127 && value >= -128 )
	{
		jit_x86_modrm modrm;

		function.push_byte( 0x83 );
		modrm.m_register_opcode = (jit_register)5;
		modrm.m_mode = RegisterDirect;
		modrm.m_rm = destination;
		function.push_byte( modrm );
		//__asm sub esp, 4;
		function.push_byte( value );
	}
	else
	{
		jit_x86_modrm modrm;

		function.push_byte( 0x81 );
		modrm.m_register_opcode = (jit_register)5;
		modrm.m_mode = RegisterDirect;
		modrm.m_rm = destination;
		function.push_byte( modrm );
		//__asm sub esp, 4;
		function.push_integer( value );
	}
}


void jit_lea( jit_function & function, const jit_register destination, const jit_register_offset_address & address )
{
	function.push_byte( 0x8d ); //mov Gv, Ev
	jit_x86_modrm modrm;

	modrm.m_register_opcode = destination;
	modrm.m_rm = address.m_register;
	if( address.m_offset > 127 || address.m_offset < -128 )
	{
		modrm.m_mode = Indexed32BitDisplacement;
		function.push_byte( modrm );
		function.push_integer( address.m_offset );
	}
	else
	{
		modrm.m_mode = Indexed8BitDisplacement;
		function.push_byte( modrm );
		function.push_byte( address.m_offset );
	}
}

void jit_pushi( jit_function & function, const void * pointer )
{
	jit_add_bytecode_u8( function, 0x68 ); 
	jit_add_bytecode_u32( function, pointer );
}

void jit_pushi( jit_function & function, int value )
{
	jit_add_bytecode_u8( function, 0x68 ); 
	jit_add_bytecode_u32( function, value );
}

void jit_pushi( jit_function & function, uint8 value )
{
	jit_add_bytecode_u8( function, 0x6A ); 
	jit_add_bytecode_u8( function, value );
}

void jit_pushi( jit_function & function, double value )
{
	int * int_value = (int*)& value;
	jit_pushi( function, int_value[ 1 ] );
	jit_pushi( function, int_value[ 0 ] );
}


#define jit_pushi( _function_, _value_ ) \
{ \
	if( jit_is_8bit( _value_ ) ) \
{ \
	jit_add_bytecode_u8( _function_, 0x6A ); jit_add_bytecode_u8( _function_, _value_ ); \
} \
	else \
{ \
	jit_add_bytecode_u8( _function_, 0x68 ); jit_add_bytecode_u32( _function_, _value_ ); \
} \
}