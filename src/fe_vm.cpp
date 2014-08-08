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

#include "fe_vm.hpp"
#include "fe_settings.hpp"
#include "fe_present.hpp"
#include "fe_text.hpp"
#include "fe_listbox.hpp"
#include "fe_image.hpp"
#include "fe_shader.hpp"
#include "fe_config.hpp"
#include "fe_overlay.hpp"
#include "fe_window.hpp"

#include "fe_util.hpp"
#include "fe_util_sq.hpp"

#include <sqrat.h>

#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

#include <iostream>
#include <stdio.h>
#include <ctime>
#include <stdarg.h>

namespace
{
	//
	// Squirrel callback functions
	//
	void printFunc(HSQUIRRELVM v, const SQChar *s, ...)
	{
		va_list vl;
		va_start(vl, s);
		vprintf(s, vl);
		va_end(vl);
	}

	bool my_callback( const char *buffer, void *opaque )
	{
		Sqrat::Function func( Sqrat::RootTable(), (const char *)opaque );

		if ( !func.IsNull() )
			func.Execute( buffer );

		return true; // return false to cancel callbacks
	}
};

const char *FeVM::transitionTypeStrings[] =
{
		"StartLayout",
		"EndLayout",
		"ToNewSelection",
		"FromOldSelection",
		"ToGame",
		"FromGame",
		"ToNewList",
		NULL
};

FeVM::FeVM( FeSettings &fes, FePresent &fep, FeWindow &wnd, FeOverlay &feo )
	: m_fes( fes ),
	m_fep( fep ),
	m_window( wnd ),
	m_overlay( feo ),
	m_redraw_triggered( false ),
	m_script_cfg( NULL )
{
	m_fep.m_vm = this;

	Sqrat::DefaultVM::Set( NULL );
	srand( time( NULL ) );
}

FeVM::~FeVM()
{
	clear();
	vm_close();
}

bool FeVM::poll_command( FeInputMap::Command &c, sf::Event &ev )
{
	if ( !m_posted_commands.empty( ))
	{
		c = (FeInputMap::Command)m_posted_commands.front();
		m_posted_commands.pop();
		ev.type = sf::Event::Count;

		return true;
	}
	else if ( m_window.pollEvent( ev ) )
	{
		c = m_fes.map_input( ev );
		return true;
	}

	return false;
}

void FeVM::clear()
{
	m_ticks_list.clear();
	m_transition_list.clear();
	m_signal_handlers.clear();

	while ( !m_posted_commands.empty() )
		m_posted_commands.pop();
}

void FeVM::add_ticks_callback( const std::string &n )
{
	m_ticks_list.push_back( n );
}

void FeVM::add_transition_callback( const std::string &n )
{
	m_transition_list.push_back( n );
}

void FeVM::add_signal_handler( const std::string &n )
{
	m_signal_handlers.push_back( n );
}

void FeVM::remove_signal_handler( const std::string &n )
{
	for ( std::vector<std::string>::iterator itr = m_signal_handlers.begin();
			itr != m_signal_handlers.end(); ++itr )
	{
		if ( n.compare( *itr ) == 0 )
		{
			m_signal_handlers.erase( itr );
			return;
		}
	}
}

void FeVM::vm_close()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		sq_close( vm );
		Sqrat::DefaultVM::Set( NULL );
	}
}

void FeVM::vm_init()
{
	vm_close();
	HSQUIRRELVM vm = sq_open( 1024 );
	sq_setprintfunc( vm, printFunc, printFunc );
	sq_pushroottable( vm );
	sq_setforeignptr( vm, this );

	sqstd_register_bloblib( vm );
	sqstd_register_iolib( vm );
	sqstd_register_mathlib( vm );
	sqstd_register_stringlib( vm );
	sqstd_register_systemlib( vm );
	sqstd_seterrorhandlers( vm );

	Sqrat::DefaultVM::Set( vm );
}

void FeVM::on_new_layout( const std::string &path,
	const std::string &filename, const FeLayoutInfo &layout_params )
{
	using namespace Sqrat;

	vm_close();

	// Squirrel VM gets reinitialized on each layout
	//
	vm_init();

	const sf::Vector2i &output_size = m_fep.get_output_size();

	// Set fe-related constants
	//
	ConstTable()
		.Const( _SC("FeVersion"), FE_VERSION)
		.Const( _SC("FeVersionNum"), FE_VERSION_NUM)
		.Const( _SC("ScreenWidth"), (int)output_size.x )
		.Const( _SC("ScreenHeight"), (int)output_size.y )
		.Const( _SC("ScreenSaverActive"), m_fep.get_screensaver_active() )
		.Const( _SC("OS"), get_OS_string() )
		.Const( _SC("ShadersAvailable"), sf::Shader::isAvailable() )
		.Const( _SC("FeConfigDirectory"), m_fes.get_config_dir().c_str() )

		.Enum( _SC("Style"), Enumeration()
			.Const( _SC("Regular"), sf::Text::Regular )
			.Const( _SC("Bold"), sf::Text::Bold )
			.Const( _SC("Italic"), sf::Text::Italic )
			.Const( _SC("Underlined"), sf::Text::Underlined )
			)
		.Enum( _SC("Align"), Enumeration()
			.Const( _SC("Left"), FeTextPrimative::Left )
			.Const( _SC("Centre"), FeTextPrimative::Centre )
			.Const( _SC("Right"), FeTextPrimative::Right )
			)
		.Enum( _SC("RotateScreen"), Enumeration()
			.Const( _SC("None"), FeSettings::RotateNone )
			.Const( _SC("Right"), FeSettings::RotateRight )
			.Const( _SC("Flip"), FeSettings::RotateFlip )
			.Const( _SC("Left"), FeSettings::RotateLeft )
			)
		// The "Axis" enum is deprecated along with fe.get_joyaxispos() as of version 1.2
		.Enum( _SC("Axis"), Enumeration()
			.Const( _SC("X"), sf::Joystick::X )
			.Const( _SC("Y"), sf::Joystick::Y )
			.Const( _SC("Z"), sf::Joystick::Z )
			.Const( _SC("R"), sf::Joystick::R )
			.Const( _SC("U"), sf::Joystick::U )
			.Const( _SC("V"), sf::Joystick::V )
			.Const( _SC("PovX"), sf::Joystick::PovX )
			.Const( _SC("PovY"), sf::Joystick::PovY )
			)
		.Enum( _SC("FromTo"), Enumeration()
			.Const( _SC("NoValue"), FromToNoValue )
			.Const( _SC("ScreenSaver"), FromToScreenSaver )
			.Const( _SC("Frontend"), FromToFrontend )
			)
		.Enum( _SC("Shader"), Enumeration()
			.Const( _SC("VertexAndFragment"), FeShader::VertexAndFragment )
			.Const( _SC("Vertex"), FeShader::Vertex )
			.Const( _SC("Fragment"), FeShader::Fragment )
			.Const( _SC("Empty"), FeShader::Empty )
			)
		.Enum( _SC("Vid"), Enumeration()
			.Const( _SC("Default"), VF_Normal )
			.Const( _SC("ImagesOnly"), VF_DisableVideo )
			.Const( _SC("NoAudio"), VF_NoAudio )
			.Const( _SC("NoAutoStart"), VF_NoAutoStart )
			.Const( _SC("NoLoop"), VF_NoLoop )
			)
		;

	// The "Key" enum is deprecated along with fe.get_keypressed() as of version 1.2
	Enumeration keys;
	int i=0;
	while ( FeInputSource::keyStrings[i] != NULL )
	{
		keys.Const( FeInputSource::keyStrings[i], i );
		i++;
	}

	ConstTable().Enum( _SC("Key"), keys);

	Enumeration info;
	i=0;
	while ( FeRomInfo::indexStrings[i] != NULL )
	{
		info.Const( FeRomInfo::indexStrings[i], i );
		i++;
	}
	ConstTable().Enum( _SC("Info"), info);

	Enumeration transition;
	i=0;
	while ( transitionTypeStrings[i] != NULL )
	{
		transition.Const( transitionTypeStrings[i], i );
		i++;
	}
	ConstTable().Enum( _SC("Transition"), transition );

	// All frontend functionality is in the "fe" table in Squirrel
	//
	Table fe;

	//
	// Define classes for fe objects that get exposed to Squirrel
	//

	// Base Presentable Object Class
	//
	fe.Bind( _SC("Presentable"),
		Class<FeBasePresentable, NoConstructor>()
		.Prop(_SC("visible"),
			&FeBasePresentable::get_visible, &FeBasePresentable::set_visible )
		.Prop(_SC("x"), &FeBasePresentable::get_x, &FeBasePresentable::set_x )
		.Prop(_SC("y"), &FeBasePresentable::get_y, &FeBasePresentable::set_y )
		.Prop(_SC("width"),
			&FeBasePresentable::get_width, &FeBasePresentable::set_width )
		.Prop(_SC("height"),
			&FeBasePresentable::get_height, &FeBasePresentable::set_height )
		.Prop(_SC("rotation"),
			&FeBasePresentable::getRotation, &FeBasePresentable::setRotation )
		.Prop(_SC("red"), &FeBasePresentable::get_r, &FeBasePresentable::set_r )
		.Prop(_SC("green"), &FeBasePresentable::get_g, &FeBasePresentable::set_g )
		.Prop(_SC("blue"), &FeBasePresentable::get_b, &FeBasePresentable::set_b )
		.Prop(_SC("alpha"), &FeBasePresentable::get_a, &FeBasePresentable::set_a )
		.Prop(_SC("index_offset"), &FeBasePresentable::getIndexOffset, &FeBasePresentable::setIndexOffset )
		.Prop(_SC("shader"), &FeBasePresentable::script_get_shader, &FeBasePresentable::script_set_shader )
		.Func( _SC("set_rgb"), &FeBasePresentable::set_rgb )
		.Overload<void (FeBasePresentable::*)(float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
		.Overload<void (FeBasePresentable::*)(float, float, float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
	);

	fe.Bind( _SC("Image"),
		DerivedClass<FeImage, FeBasePresentable, NoConstructor>()

		// Misnamed: shear_x and shear_y deprecated as of version 1.3, more accurately named
		// skew_x and skew_y instead
		.Prop(_SC("shear_x"), &FeImage::get_skew_x, &FeImage::set_skew_x )
		.Prop(_SC("shear_y"), &FeImage::get_skew_y, &FeImage::set_skew_y )

		.Prop(_SC("skew_x"), &FeImage::get_skew_x, &FeImage::set_skew_x )
		.Prop(_SC("skew_y"), &FeImage::get_skew_y, &FeImage::set_skew_y )
		.Prop(_SC("pinch_x"), &FeImage::get_pinch_x, &FeImage::set_pinch_x )
		.Prop(_SC("pinch_y"), &FeImage::get_pinch_y, &FeImage::set_pinch_y )
		.Prop(_SC("texture_width"), &FeImage::get_texture_width )
		.Prop(_SC("texture_height"), &FeImage::get_texture_height )
		.Prop(_SC("subimg_x"), &FeImage::get_subimg_x, &FeImage::set_subimg_x )
		.Prop(_SC("subimg_y"), &FeImage::get_subimg_y, &FeImage::set_subimg_y )
		.Prop(_SC("subimg_width"), &FeImage::get_subimg_width, &FeImage::set_subimg_width )
		.Prop(_SC("subimg_height"), &FeImage::get_subimg_height, &FeImage::set_subimg_height )
		// "movie_enabled" deprecated as of version 1.3, use the video_flags property instead:
		.Prop(_SC("movie_enabled"), &FeImage::getMovieEnabled, &FeImage::setMovieEnabled )
		.Prop(_SC("video_flags"), &FeImage::getVideoFlags, &FeImage::setVideoFlags )
		.Prop(_SC("video_playing"), &FeImage::getVideoPlaying, &FeImage::setVideoPlaying )
		.Prop(_SC("video_duration"), &FeImage::getVideoDuration )
		.Prop(_SC("video_time"), &FeImage::getVideoTime )
		.Prop(_SC("preserve_aspect_ratio"), &FeImage::get_preserve_aspect_ratio,
				&FeImage::set_preserve_aspect_ratio )
		.Prop(_SC("file_name"), &FeImage::getFileName, &FeImage::setFileName )
		.Func( _SC("swap"), &FeImage::transition_swap )

		//
		// Surface-specific functionality:
		//
		.Overload<FeImage * (FeImage::*)(const char *, int, int, int, int)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *, int, int)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *)>(_SC("add_image"), &FeImage::add_image)
		.Overload<FeImage * (FeImage::*)(const char *, int, int, int, int)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Overload<FeImage * (FeImage::*)(const char *, int, int)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Overload<FeImage * (FeImage::*)(const char *)>(_SC("add_artwork"), &FeImage::add_artwork)
		.Func( _SC("add_clone"), &FeImage::add_clone )
		.Func( _SC("add_text"), &FeImage::add_text )
		.Func( _SC("add_listbox"), &FeImage::add_listbox )
		.Func( _SC("add_surface"), &FeImage::add_surface )
	);

	fe.Bind( _SC("Text"),
		DerivedClass<FeText, FeBasePresentable, NoConstructor>()
		.Prop(_SC("msg"), &FeText::get_string, &FeText::set_string )
		.Prop(_SC("bg_red"), &FeText::get_bgr, &FeText::set_bgr )
		.Prop(_SC("bg_green"), &FeText::get_bgg, &FeText::set_bgg )
		.Prop(_SC("bg_blue"), &FeText::get_bgb, &FeText::set_bgb )
		.Prop(_SC("bg_alpha"), &FeText::get_bga, &FeText::set_bga )
		.Prop(_SC("charsize"), &FeText::get_charsize, &FeText::set_charsize )
		.Prop(_SC("style"), &FeText::get_style, &FeText::set_style )
		.Prop(_SC("align"), &FeText::get_align, &FeText::set_align )
		.Prop(_SC("word_wrap"), &FeText::get_word_wrap, &FeText::set_word_wrap )
		.Prop(_SC("font"), &FeText::get_font, &FeText::set_font )
		.Func( _SC("set_bg_rgb"), &FeText::set_bg_rgb )
	);

	fe.Bind( _SC("ListBox"),
		DerivedClass<FeListBox, FeBasePresentable, NoConstructor>()
		.Prop(_SC("bg_red"), &FeListBox::get_bgr, &FeListBox::set_bgr )
		.Prop(_SC("bg_green"), &FeListBox::get_bgg, &FeListBox::set_bgg )
		.Prop(_SC("bg_blue"), &FeListBox::get_bgb, &FeListBox::set_bgb )
		.Prop(_SC("bg_alpha"), &FeListBox::get_bga, &FeListBox::set_bga )
		.Prop(_SC("sel_red"), &FeListBox::get_selr, &FeListBox::set_selr )
		.Prop(_SC("sel_green"), &FeListBox::get_selg, &FeListBox::set_selg )
		.Prop(_SC("sel_blue"), &FeListBox::get_selb, &FeListBox::set_selb )
		.Prop(_SC("sel_alpha"), &FeListBox::get_sela, &FeListBox::set_sela )
		.Prop(_SC("selbg_red"), &FeListBox::get_selbgr, &FeListBox::set_selbgr )
		.Prop(_SC("selbg_green"), &FeListBox::get_selbgg, &FeListBox::set_selbgg )
		.Prop(_SC("selbg_blue"), &FeListBox::get_selbgb, &FeListBox::set_selbgb )
		.Prop(_SC("selbg_alpha"), &FeListBox::get_selbga, &FeListBox::set_selbga )
		.Prop(_SC("rows"), &FeListBox::get_rows, &FeListBox::set_rows )
		.Prop(_SC("charsize"), &FeListBox::get_charsize, &FeListBox::set_charsize )
		.Prop(_SC("style"), &FeListBox::get_style, &FeListBox::set_style )
		.Prop(_SC("align"), &FeListBox::get_align, &FeListBox::set_align )
		.Prop(_SC("sel_style"), &FeListBox::getSelStyle, &FeListBox::setSelStyle )
		.Prop(_SC("font"), &FeListBox::get_font, &FeListBox::set_font )
		.Func( _SC("set_bg_rgb"), &FeListBox::set_bg_rgb )
		.Func( _SC("set_sel_rgb"), &FeListBox::set_sel_rgb )
		.Func( _SC("set_selbg_rgb"), &FeListBox::set_selbg_rgb )
	);

	fe.Bind( _SC("LayoutGlobals"), Class <FePresent, NoConstructor>()
		.Prop( _SC("width"), &FePresent::get_layout_width, &FePresent::set_layout_width )
		.Prop( _SC("height"), &FePresent::get_layout_height, &FePresent::set_layout_height )
		.Prop( _SC("font"), &FePresent::get_layout_font, &FePresent::set_layout_font )
		// orient property deprecated as of 1.3.2, use "base_rotation" instead
		.Prop( _SC("orient"), &FePresent::get_base_rotation, &FePresent::set_base_rotation )
		.Prop( _SC("base_rotation"), &FePresent::get_base_rotation, &FePresent::set_base_rotation )
		.Prop( _SC("toggle_rotation"), &FePresent::get_toggle_rotation, &FePresent::set_toggle_rotation )
	);

	fe.Bind( _SC("CurrentList"), Class <FePresent, NoConstructor>()
		.Prop( _SC("name"), &FePresent::get_list_name )
		.Prop( _SC("filter"), &FePresent::get_filter_name )
		.Prop( _SC("size"), &FePresent::get_list_size )
		.Prop( _SC("index"), &FePresent::get_list_index, &FePresent::set_list_index )
	);

	fe.Bind( _SC("Sound"), Class <FeScriptSound, NoConstructor>()
		.Func( _SC("play"), &FeScriptSound::play )
		.Prop( _SC("is_playing"), &FeScriptSound::is_playing )
		.Prop( _SC("pitch"), &FeScriptSound::get_pitch, &FeScriptSound::set_pitch )
		.Prop( _SC("x"), &FeScriptSound::get_x, &FeScriptSound::set_x )
		.Prop( _SC("y"), &FeScriptSound::get_y, &FeScriptSound::set_y )
		.Prop( _SC("z"), &FeScriptSound::get_z, &FeScriptSound::set_z )
	);

	fe.Bind( _SC("Shader"), Class <FeShader, NoConstructor>()
		.Prop( _SC("type"), &FeShader::get_type )
		.Overload<void (FeShader::*)(const char *, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *, float, float, float, float)>(_SC("set_param"), &FeShader::set_param)
		.Overload<void (FeShader::*)(const char *)>( _SC("set_texture_param"), &FeShader::set_texture_param )
		.Overload<void (FeShader::*)(const char *, FeImage *)>( _SC("set_texture_param"), &FeShader::set_texture_param )
	);

	//
	// Define functions that get exposed to Squirrel
	//

	fe.Overload<FeImage* (*)(const char *, int, int, int, int)>(_SC("add_image"), &FeVM::cb_add_image);
	fe.Overload<FeImage* (*)(const char *, int, int)>(_SC("add_image"), &FeVM::cb_add_image);
	fe.Overload<FeImage* (*)(const char *)>(_SC("add_image"), &FeVM::cb_add_image);

	fe.Overload<FeImage* (*)(const char *, int, int, int, int)>(_SC("add_artwork"), &FeVM::cb_add_artwork);
	fe.Overload<FeImage* (*)(const char *, int, int)>(_SC("add_artwork"), &FeVM::cb_add_artwork);
	fe.Overload<FeImage* (*)(const char *)>(_SC("add_artwork"), &FeVM::cb_add_artwork);

	fe.Func<FeImage* (*)(FeImage *)>(_SC("add_clone"), &FeVM::cb_add_clone);

	fe.Overload<FeText* (*)(const char *, int, int, int, int)>(_SC("add_text"), &FeVM::cb_add_text);
	fe.Func<FeListBox* (*)(int, int, int, int)>(_SC("add_listbox"), &FeVM::cb_add_listbox);
	fe.Func<FeImage* (*)(int, int)>(_SC("add_surface"), &FeVM::cb_add_surface);
	fe.Func<FeScriptSound* (*)(const char *)>(_SC("add_sound"), &FeVM::cb_add_sound);
	fe.Overload<FeShader* (*)(int, const char *, const char *)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Overload<FeShader* (*)(int, const char *)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Overload<FeShader* (*)(int)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Func<void (*)(const char *)>(_SC("add_ticks_callback"), &FeVM::cb_add_ticks_callback);
	fe.Func<void (*)(const char *)>(_SC("add_transition_callback"), &FeVM::cb_add_transition_callback);
	fe.Func<void (*)(const char *)>(_SC("add_signal_handler"), &FeVM::cb_add_signal_handler);
	fe.Func<void (*)(const char *)>(_SC("remove_signal_handler"), &FeVM::cb_remove_signal_handler);
	fe.Func<bool (*)(const char *)>(_SC("get_input_state"), &FeVM::cb_get_input_state);
	fe.Func<int (*)(const char *)>(_SC("get_input_pos"), &FeVM::cb_get_input_pos);
	fe.Func<void (*)(const char *)>(_SC("do_nut"), &FeVM::do_nut);
	fe.Func<bool (*)(const char *)>(_SC("load_module"), &FeVM::load_module);
	fe.Overload<const char* (*)(int)>(_SC("game_info"), &FeVM::cb_game_info);
	fe.Overload<const char* (*)(int, int)>(_SC("game_info"), &FeVM::cb_game_info);
	fe.Overload<bool (*)(const char *, const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
	fe.Overload<bool (*)(const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
	fe.Func<bool (*)(const char *, const char *)>(_SC("plugin_command_bg"), &FeVM::cb_plugin_command_bg);
	fe.Func<const char* (*)(const char *)>(_SC("path_expand"), &FeVM::cb_path_expand);
	fe.Func<Sqrat::Table (*)()>(_SC("get_config"), &FeVM::cb_get_config);
	fe.Overload<int (*)( Sqrat::Array, const char *, int, int )>(_SC("list_dialog"), &FeVM::cb_list_dialog);
	fe.Overload<int (*)( Sqrat::Array, const char *, int )>(_SC("list_dialog"), &FeVM::cb_list_dialog);
	fe.Overload<int (*)( Sqrat::Array, const char * )>(_SC("list_dialog"), &FeVM::cb_list_dialog);
	fe.Overload<int (*)( Sqrat::Array )>(_SC("list_dialog"), &FeVM::cb_list_dialog);
	fe.Func<const char* (*)(const char *, const char *)>(_SC("edit_dialog"), &FeVM::cb_edit_dialog);
	fe.Func<void (*)(const char *)>(_SC("signal"), &FeVM::cb_signal);

	// BEGIN deprecated functions
	// is_keypressed() and is_joybuttonpressed() deprecated as of version 1.2.  Use get_input_state() instead
	fe.Func<bool (*)(int)>(_SC("is_keypressed"), &FeVM::cb_is_keypressed);
	fe.Func<bool (*)(int, int)>(_SC("is_joybuttonpressed"), &FeVM::cb_is_joybuttonpressed);
	// get_joyaxispos() deprecated as of version 1.2.  Use get_input_pos() instead
	fe.Func<float (*)(int, int)>(_SC("get_joyaxispos"), &FeVM::cb_get_joyaxispos);
	// END deprecated functions

	//
	// Define variables that get exposed to Squirrel
	//

	fe.SetInstance( _SC("layout"), &m_fep );
	fe.SetInstance( _SC("list"), &m_fep );

	// Each presentation object gets an instance in the
	// "obj" table available in Squirrel
	//
	Table obj;
	fe.Bind( _SC("obj"), obj );
	RootTable().Bind( _SC("fe"),  fe );

	//
	// Run the layout script
	//
	if ( file_exists( path + filename ) )
	{
		fe.SetValue( _SC("script_dir"), path );
		fe.SetValue( _SC("script_file"), filename );
		m_script_cfg = &layout_params;

		try
		{
			Script sc;
			sc.CompileFile( path + filename );
			sc.Run();
		}
		catch( Exception e )
		{
			std::cerr << "Script Error in " << path + filename
				<< " - " << e.Message() << std::endl;
		}
	}
	else
	{
		std::cerr << "Script file not found: " << path + filename << std::endl;
	}

	//
	// Now run any plugin script(s)
	//
	const std::vector< FePlugInfo > &plugins = m_fes.get_plugins();

	for ( std::vector< FePlugInfo >::const_iterator itr= plugins.begin();
		itr != plugins.end(); ++itr )
	{
		// Don't run disabled plugins...
		if ( (*itr).get_enabled() == false )
			continue;

		std::string plug_path, plug_name;
		m_fes.get_plugin_full_path(
				(*itr).get_name(),
				plug_path,
				plug_name );

		if ( !plug_name.empty() )
		{
			fe.SetValue( _SC("script_dir"), plug_path );
			fe.SetValue( _SC("script_file"), plug_name );
			m_script_cfg = &(*itr);

			try
			{
				Script sc;
				sc.CompileFile( plug_path + plug_name );
				sc.Run();
			}
			catch( Exception e )
			{
				std::cout << "Script Error in " << plug_path + plug_name
					<< " - " << e.Message() << std::endl;
			}
		}
	}

	fe.SetValue( _SC("script_dir"), "" );
	fe.SetValue( _SC("script_file"), "" );
	m_script_cfg = NULL;
}

bool FeVM::on_tick()
{
	using namespace Sqrat;
	m_redraw_triggered = false;

	for ( std::vector<std::string>::iterator itr = m_ticks_list.begin();
		itr != m_ticks_list.end(); )
	{
		// Assumption: Ticks list is empty if no vm is active
		//
		ASSERT( Sqrat::DefaultVM::Get() );

		bool remove=false;
		try
		{
			Function func( RootTable(), (*itr).c_str() );

			if ( !func.IsNull() )
				func.Execute( m_fep.m_layoutTimer.getElapsedTime().asMilliseconds() );
		}
		catch( Exception e )
		{
			std::cout << "Script Error in " << (*itr)
				<< "(): " << e.Message() << std::endl;

			// Knock out this entry.   If it causes a script error, we don't
			// want to call it anymore
			//
			remove=true;
		}

		if ( remove )
			itr = m_ticks_list.erase( itr );
		else
			itr++;
	}

	return m_redraw_triggered;
}

bool FeVM::on_transition(
	FeTransitionType t,
	int var )
{
	using namespace Sqrat;

#ifdef FE_DEBUG
	std::cout << "[Transition] type=" << transitionTypeStrings[t] << ", var=" << var << std::endl;
#endif // FE_DEBUG

	sf::Clock ttimer;
	m_redraw_triggered = false;

	std::vector<const char *> worklist( m_transition_list.size() );
	for ( unsigned int i=0; i < m_transition_list.size(); i++ )
		worklist[i] = m_transition_list[i].c_str();

	//
	// A registered transition callback stays in the worklist for as long
	// as it keeps returning true.
	//
	while ( !worklist.empty() )
	{
		// Assumption: Transition list is empty if no vm is active
		//
		ASSERT( DefaultVM::Get() );

		//
		// Call each remaining transition callback on each pass through
		// the worklist
		//
		for ( std::vector<const char *>::iterator itr=worklist.begin();
			itr != worklist.end(); )
		{
			bool keep=false;
			try
			{
				Function func( RootTable(), (*itr) );
				if ( !func.IsNull() )
				{
					keep = func.Evaluate<bool>(
						(int)t,
						var,
						ttimer.getElapsedTime().asMilliseconds() );
				}
			}
			catch( Exception e )
			{
				std::cout << "Script Error in " << (*itr)
					<< "(): " << e.Message() << std::endl;
			}

			if ( !keep )
				itr = worklist.erase( itr );
			else
				itr++;
		}

		// redraw now if we are doing another pass...
		//
		if (( !worklist.empty() ) && ( m_window.isOpen() ))
		{
			m_fep.video_tick();

			m_window.clear();
			m_window.draw( m_fep );
			m_window.display();

			m_redraw_triggered = false; // clear redraw flag
		}
	}

	return m_redraw_triggered;
}

bool FeVM::handle_event( FeInputMap::Command c )
{
	using namespace Sqrat;

	//
	// Go through the list in reverse so that the most recently registered signal handler
	// gets the first shot at handling the signal.
	//
	for ( std::vector<std::string>::reverse_iterator itr = m_signal_handlers.rbegin();
		itr != m_signal_handlers.rend(); ++itr )
	{
		// Assumption: Handlers list is empty if no vm is active
		//
		ASSERT( DefaultVM::Get() );

		try
		{
			Function func( RootTable(), (*itr).c_str() );

			if (( !func.IsNull() )
					&& ( func.Evaluate<bool>( FeInputMap::commandStrings[ c ] )))
				return true;
		}
		catch( Exception e )
		{
			std::cout << "Script Error in " << (*itr)
				<< "(): " << e.Message() << std::endl;
		}
	}

	return false;
}

//
// Script static functions
//
FePresent *FeVM::script_get_fep()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		FeVM *fev = (FeVM *)sq_getforeignptr( vm );

		if ( fev )
			return &(fev->m_fep);
	}

	return NULL;
}

void FeVM::script_do_update( FeBasePresentable *bp )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		FeVM *fev = (FeVM *)sq_getforeignptr( vm );

		bp->on_new_list( &(fev->m_fes),
			fev->m_fep.get_layout_scale_x(),
			fev->m_fep.get_layout_scale_y() );

		bp->on_new_selection( &(fev->m_fes) );

		fev->flag_redraw();
	}
}

void FeVM::script_do_update( FeBaseTextureContainer *tc )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		FeVM *fev = (FeVM *)sq_getforeignptr( vm );

		tc->on_new_selection( &(fev->m_fes), fev->m_fep.get_screensaver_active() );
		fev->flag_redraw();
	}
}

void FeVM::script_flag_redraw()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
	{
		FeVM *fev = (FeVM *)sq_getforeignptr( vm );
		fev->flag_redraw();
	}
}

void FeVM::script_get_config_options(
		FeConfigContext &ctx,
		std::string &gen_help,
		FeScriptConfigurable &configurable,
		const std::string &script_file )
{
	if ( !script_file.empty() )
	{
		HSQUIRRELVM stored_vm = Sqrat::DefaultVM::Get();
		HSQUIRRELVM temp_vm = sq_open( 1024 );
		sq_pushroottable( temp_vm );
		Sqrat::DefaultVM::Set( temp_vm );

		Sqrat::ConstTable()
			.Const( _SC("FeVersion"), FE_VERSION)
			.Const( _SC("FeVersionNum"), FE_VERSION_NUM)
			.Const( _SC("OS"), get_OS_string() )
			.Const( _SC("ShadersAvailable"), sf::Shader::isAvailable() );

		try
		{
			Sqrat::Script sc;
			sc.CompileFile( script_file );
			sc.Run();
		}
		catch( Sqrat::Exception e )
		{
			// ignore all errors, they are expected
		}

		// Control the scope of our Sqrat objects so they are destroyed
		// before we call sq_close() on the vm below
		//
		{
			Sqrat::Object uConfig = Sqrat::RootTable().GetSlot( "UserConfig" );
			if ( !uConfig.IsNull() )
			{
				fe_get_attribute_string(
					temp_vm,
					uConfig.GetObject(), "", "help", gen_help );

				// Now Ccnstruct the UI elements for plug-in/layout specific configuration
				//
				std::multimap<int,FeMenuOpt> my_opts;

				Sqrat::Object::iterator it;
				while ( uConfig.Next( it ) )
				{
					std::string key;
					fe_get_object_string( temp_vm, it.getKey(), key );

					std::string value, label, help, options, is_input;

					// use the default value from the script if a value has
					// not already been configured
					//
					if ( !configurable.get_param( key, value ) )
						fe_get_object_string( temp_vm, uConfig.GetSlot( key.c_str() ), value );

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), key, "label", label);

					if ( label.empty() )
						label = key;

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), key, "help", help);

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), key, "options", options);

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), key, "is_input", is_input);

					std::string otmp;
					int order=-1;
					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), key, "order", otmp);

					if ( !otmp.empty() )
						order = as_int( otmp );


					if ( !options.empty() )
					{
						std::vector<std::string> options_list;
						size_t pos=0;
						do
						{
							std::string temp;
							token_helper( options, pos, temp, "," );
							options_list.push_back( temp );
						} while ( pos < options.size() );

						std::multimap<int,FeMenuOpt>::iterator it = my_opts.insert(
								std::pair <int, FeMenuOpt>(
									order,
									FeMenuOpt(Opt::LIST, label, value, help, 0, key ) ) );

						(*it).second.append_vlist( options_list );
					}
					else if ( config_str_to_bool( is_input ) )
					{
						my_opts.insert(
								std::pair <int, FeMenuOpt>(
									order,
									FeMenuOpt(Opt::RELOAD, label, value, help, 1, key ) ) );
					}
					else
					{
						my_opts.insert(
								std::pair <int, FeMenuOpt>(
									order,
									FeMenuOpt(Opt::EDIT, label, value, help, 0, key ) ) );
					}
				}

				for ( std::multimap<int,FeMenuOpt>::iterator itr = my_opts.begin(); itr != my_opts.end(); ++itr )
					ctx.opt_list.push_back( (*itr).second );
			}
		}

		// reset to our usual VM and close the temp vm
		Sqrat::DefaultVM::Set( stored_vm );
		sq_close( temp_vm );
	}
}

//
// Callback functions
//
FeImage* FeVM::cb_add_image(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeImage *ret = fev->m_fep.add_image( false, n, x, y, w, h, fev->m_fep.m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FeVM::cb_add_image(const char *n, int x, int y )
{
	return cb_add_image( n, x, y, 0, 0 );
}

FeImage* FeVM::cb_add_image(const char *n )
{
	return cb_add_image( n, 0, 0, 0, 0 );
}

FeImage* FeVM::cb_add_artwork(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeImage *ret = fev->m_fep.add_image( true, n, x, y, w, h, fev->m_fep.m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FeVM::cb_add_artwork(const char *n, int x, int y )
{
	return cb_add_artwork( n, x, y, 0, 0 );
}

FeImage* FeVM::cb_add_artwork(const char *n )
{
	return cb_add_artwork( n, 0, 0, 0, 0 );
}

FeImage* FeVM::cb_add_clone( FeImage *o )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeImage *ret = fev->m_fep.add_clone( o, fev->m_fep.m_elements );

	// Add the image to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeText* FeVM::cb_add_text(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeText *ret = fev->m_fep.add_text( n, x, y, w, h, fev->m_fep.m_elements );

	// Add the text to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeListBox* FeVM::cb_add_listbox(int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeListBox *ret = fev->m_fep.add_listbox( x, y, w, h, fev->m_fep.m_elements );

	// Add the listbox to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe ( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeImage* FeVM::cb_add_surface( int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeImage *ret = fev->m_fep.add_surface( w, h, fev->m_fep.m_elements );

	// Add the surface to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe ( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeScriptSound* FeVM::cb_add_sound( const char *s )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return fev->m_fep.add_sound( s );
	//
	// We assume the script will keep a reference to the sound
	//
}

FeShader* FeVM::cb_add_shader( int type, const char *shader1, const char *shader2 )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return fev->m_fep.add_shader( (FeShader::Type)type, shader1, shader2 );
	//
	// We assume the script will keep a reference to the shader
	//
}

FeShader* FeVM::cb_add_shader( int type, const char *shader1 )
{
	return cb_add_shader( type, shader1, NULL );
}

FeShader* FeVM::cb_add_shader( int type )
{
	return cb_add_shader( type, NULL, NULL );
}

void FeVM::cb_add_ticks_callback( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_ticks_callback( n );
}

void FeVM::cb_add_transition_callback( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_transition_callback( n );
}

void FeVM::cb_add_signal_handler( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_signal_handler( n );
}

void FeVM::cb_remove_signal_handler( const char *n )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->remove_signal_handler( n );
}

bool FeVM::cb_is_keypressed( int k )
{
	return sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)k );
}

bool FeVM::cb_is_joybuttonpressed( int num, int b )
{
	return sf::Joystick::isButtonPressed( num, b );
}

float FeVM::cb_get_joyaxispos( int num, int a )
{
	sf::Joystick::update();
	return sf::Joystick::getAxisPosition( num, (sf::Joystick::Axis)a );
}

bool FeVM::cb_get_input_state( const char *input )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return FeInputSource( input ).get_current_state( fev->m_fes.get_joy_thresh() );
}

int FeVM::cb_get_input_pos( const char *input )
{
	return FeInputSource( input ).get_current_pos();
}

// return false if file not found
bool FeVM::internal_do_nut( const std::string &work_dir,
			const std::string &script_file )
{
	std::string path;

	if ( is_relative_path( script_file) )
	{
		path = work_dir;
		path += script_file;
	}
	else
		path = script_file;

	if ( !file_exists( path ) )
		return false;

	try
	{
		Sqrat::Script sc;
		sc.CompileFile( path );
		sc.Run();
	}
	catch( Sqrat::Exception e )
	{
		std::cout << "Script Error in " << path
			<< " - " << e.Message() << std::endl;
	}

	return true;
}

void FeVM::do_nut( const char *script_file )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	internal_do_nut( fev->m_fes.get_current_layout_dir(), script_file );
}

bool FeVM::load_module( const char *module_file )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	std::string fixed_file = module_file;
	if ( !tail_compare( fixed_file, FE_LAYOUT_FILE_EXTENSION ) )
		fixed_file += FE_LAYOUT_FILE_EXTENSION;

	std::string temp = fev->m_fes.get_module_dir( fixed_file );
	size_t len = temp.find_last_of( "/\\" );
	ASSERT( len != std::string::npos );

	std::string path = temp.substr( 0, len + 1 );
	return internal_do_nut( path, fixed_file );
}

bool FeVM::cb_plugin_command( const char *command,
		const char *args,
		const char *output_callback )
{
	return run_program( clean_path( command ),
		args, my_callback, (void *)output_callback );
}

bool FeVM::cb_plugin_command( const char *command, const char *args )
{
	return run_program( clean_path( command ), args );
}

bool FeVM::cb_plugin_command_bg( const char *command, const char *args )
{
	return run_program( clean_path( command ), args, NULL, NULL, false );
}

const char *FeVM::cb_path_expand( const char *path )
{
		static std::string internal_str;

		internal_str = clean_path( path );
		return internal_str.c_str();
}

const char *FeVM::cb_game_info( int index, int offset )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return (fev->m_fes.get_rom_info( offset, (FeRomInfo::Index)index )).c_str();
}

const char *FeVM::cb_game_info( int index )
{
	return cb_game_info( index, 0 );
}

Sqrat::Table FeVM::cb_get_config()
{
	Sqrat::Object uConfig = Sqrat::RootTable().GetSlot( "UserConfig" );
	if ( uConfig.IsNull() )
		return NULL;

	Sqrat::Table retval;
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	Sqrat::Object::iterator it;
	while ( uConfig.Next( it ) )
	{
		std::string key, value;
		fe_get_object_string( vm, it.getKey(), key );

		// use the default value from the script if a value has
		// not already been configured
		//
		if (( !fev->m_script_cfg )
				|| ( !fev->m_script_cfg->get_param( key, value ) ))
		{
			fe_get_object_string( vm, it.getValue(), value );
		}

		retval.SetValue( key.c_str(), value.c_str() );
	}

	return retval;
}

int FeVM::cb_list_dialog( Sqrat::Array t, const char *title, int default_sel, int cancel_sel )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	if ( fev->m_overlay.overlay_is_on() )
		return cancel_sel;

	std::vector < std::string > list_entries;

	Sqrat::Object::iterator it;
	while ( t.Next( it ) )
	{
		std::string value;
		fe_get_object_string( vm, it.getValue(), value );

		list_entries.push_back( value );
	}

	if ( list_entries.size() > 2 )
	{
		return fev->m_overlay.common_list_dialog(
				std::string( title ),
				list_entries,
				default_sel,
				cancel_sel );
	}
	else
	{
		return fev->m_overlay.common_basic_dialog(
				std::string( title ),
				list_entries,
				default_sel,
				cancel_sel );
	}
}

int FeVM::cb_list_dialog( Sqrat::Array t, const char *title, int default_sel )
{
	return cb_list_dialog( t, title, default_sel, -1 );
}

int FeVM::cb_list_dialog( Sqrat::Array t, const char *title )
{
	return cb_list_dialog( t, title, 0, -1 );
}

int FeVM::cb_list_dialog( Sqrat::Array t )
{
	return cb_list_dialog( t, NULL, 0, -1 );
}

const char *FeVM::cb_edit_dialog( const char *msg, const char *txt )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	static std::string local_copy;
	local_copy = txt;

	if ( !fev->m_overlay.overlay_is_on() )
		fev->m_overlay.edit_dialog( msg, local_copy );

	return local_copy.c_str();
}

void FeVM::cb_signal( const char *sig )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	//
	// First check for signals corresponding to input actions
	//
	int i=0, index=-1;
	while ( FeInputMap::commandStrings[i] != 0 )
	{
		if ( strcmp( FeInputMap::commandStrings[i], sig ) == 0 )
		{
			index = i;
			break;
		}
		i++;
	}

	if ( index > -1 )
	{
		//
		// Post the command so it can be handled the next time we are
		// processing events...
		//
		fev->m_posted_commands.push( (FeInputMap::Command)index );
		return;
	}

	//
	// Next check for special case signals
	//
	const char *signals[] =
	{
		"reset_window",
		NULL
	};

	i=0;
	while ( signals[i] != 0 )
	{
		if ( strcmp( signals[i], sig ) == 0 )
		{
			index = i;
			break;
		}
		i++;
	}

	switch (index)
	{
	case 0: // "reset_window"
		fev->m_window.on_exit();
		fev->m_window.initial_create();
		break;

	default:
		std::cerr << "Error, unrecognized signal: " << sig << std::endl;
		break;

	}
}