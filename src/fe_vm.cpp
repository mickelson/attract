/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2014-16 Andrew Mickelson
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

#include <SFML/OpenGL.hpp>
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
#include "fe_blend.hpp"

#include "fe_util.hpp"
#include "fe_util_sq.hpp"
#include "image_loader.hpp"
#include "zip.hpp"

#include <sqrat.h>

#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

#include "nowide/fstream.hpp"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <stdarg.h>

const char *FE_SCRIPT_NV_FILE = "script.nv";

namespace
{
	//
	// Squirrel callback functions
	//
	void printFunc(HSQUIRRELVM v, const SQChar *s, ...)
	{
		char buff[2048];
		va_list vl;
		va_start(vl, s);
		vsnprintf( buff, 2048, s, vl);
		va_end(vl);

		FeLog() << buff;
	}

	bool my_callback( const char *buffer, void *opaque )
	{
		try
		{
			Sqrat::Function *func = (Sqrat::Function *)opaque;

			if ( !func->IsNull() )
				func->Execute( buffer );

			return true; // return false to cancel callbacks
		}
		catch( Sqrat::Exception e )
		{
			FeLog() << "Script Error: " << e.Message() << std::endl;
		}

		return false;
	}

	bool run_script( const std::string &path,
		const std::string &filename,
		bool silent=false )
	{
		std::string path_to_run=path;
		try
		{
			Sqrat::Script sc;
			if ( is_supported_archive( path ) )
			{
				FeZipStream zip( path );
				if ( !zip.open( filename ) )
				{
					// Error opening specified filename.  Try to correct
					// in case filename is in a subdir of the archive
					std::string temp;
					if ( get_archive_filename_with_base(
							temp, path, filename ) )
						zip.open( temp );
					else
						return false;
				}

				// in case error reporting is needed
				path_to_run += " (";
				path_to_run += filename;
				path_to_run += ")";

				char *d = zip.getData();

				// CompileString chokes on whitespace at the start of a file,
				// while CompileFile (below) doesn't seem to have the same
				// problem...
				//
				int i=0;
				while (( i < zip.getSize() ) && ( d[i] < 32 ))
					i++;

				if ( i == zip.getSize() )
					return false;

				std::string str( &(d[i]), zip.getSize()-i );
				sc.CompileString( str );
			}
			else
			{
				path_to_run += filename;

				if ( !file_exists( path_to_run ) )
					return false;

				sc.CompileFile( path_to_run );
			}

			FeDebug() << "Running script: " << path_to_run << std::endl;
			sc.Run();
			FeDebug() << "Done script: " << path_to_run << std::endl;
		}
		catch(Sqrat:: Exception e )
		{
			if ( !silent )
				FeLog() << "Script Error in " << path_to_run
					<< " - " << e.Message() << std::endl;
		}
		return true;
	}

	SQInteger zip_extract_file(HSQUIRRELVM vm)
	{
		if ( sq_gettop( vm ) != 3 )
			return sq_throwerror( vm, "two parameters expected" );

		// param 2 is archive name, 3 is filename
		const SQChar *archive;
		const SQChar *filename;

		if ( SQ_FAILED( sq_getstring( vm, 2, &archive ) ) )
			return sq_throwerror( vm, "incorrect archive parameter" );

		if ( SQ_FAILED( sq_getstring( vm, 3, &filename ) ) )
			return sq_throwerror( vm, "incorrect filename parameter" );

		std::string path;
		if ( is_relative_path( archive ) )
			path = FePresent::script_get_base_path() + archive;
		else
			path = archive;

		std::vector < char > buff;
		if ( !fe_zip_open_to_buff(
				path.c_str(),
				filename,
				buff ) )
			return sq_throwerror( vm, "error reading zip" );

		SQUserPointer my_ptr = sqstd_createblob( vm, buff.size() );
		memcpy( my_ptr, &(buff[0]), buff.size() );
		return 1;
	}

	SQInteger zip_get_dir(HSQUIRRELVM vm)
	{
		if ( sq_gettop( vm ) != 2 )
			return sq_throwerror( vm, "one parameter expected" );

		// param 2 is archive name
		const SQChar *archive;

		if ( SQ_FAILED( sq_getstring( vm, 2, &archive ) ) )
			return sq_throwerror( vm, "incorrect archive parameter" );

		std::string path;
		if ( is_relative_path( archive ) )
			path = FePresent::script_get_base_path() + archive;
		else
			path = archive;

		std::vector < std::string > res;

		// if we are given a directory, simply return the contents of
		// the directory
		//
		if ( directory_exists( path ) )
			get_basename_from_extension( res, path, "", false );
		else if ( is_supported_archive( path ) )
			fe_zip_get_dir( path.c_str(), res );

		sq_newarray( vm, 0 );

		for ( std::vector<std::string>::iterator itr=res.begin();
			itr != res.end(); ++itr )
		{
			sq_pushstring( vm, (*itr).c_str(), -1 );
			sq_arrayappend(vm, -2);
		}

		return 1;
	}

	std::string read_non_volatile_to_string( const SQChar *name )
	{
		Sqrat::Table fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
		Sqrat::Table snv( fe.GetSlot( name ) );

		if ( snv.IsNull() )
			return "";

		return fe_to_json_string( snv );
	}

	void write_non_volatile_from_string( const char *name,
		std::string &nv )
	{
		if ( !nv.empty() )
		{
			std::string temp = "fe.";
			temp += name;
			temp += " <- ";
			temp += nv;
			temp += ";";

			try
			{
				Sqrat::Script sc;
				sc.CompileString( temp );
				sc.Run();
			}
			catch ( Sqrat::Exception e )
			{
				FeLog() << "Error compiling " << name << " string: [" << nv << "] - " << e.Message() << std::endl;
			}
		}
	}
};

FeCallback::FeCallback( int pid,
		const Sqrat::Object &env,
		const std::string &fn,
		FeSettings &fes )
	: m_sid( pid ),
	m_env( env ),
	m_fn( fn )
{
	// the layout/screensaver/intro will have a m_pid < 0
	if ( pid < 0 )
	{
		fes.get_path( FeSettings::Current,
			m_path,
			m_file );

		m_cfg = &(fes.get_current_config( FeSettings::Current ));
	}
	else // otherwise this is a plugin
	{
		const std::vector< FePlugInfo > &pg = fes.get_plugins();
		fes.get_plugin_full_path( pg[pid].get_name(), m_path, m_file );
		m_cfg = &(pg[pid]);
	}
}

Sqrat::Function &FeCallback::get_fn()
{
	if ( m_cached_fn.IsNull() )
		m_cached_fn = Sqrat::Function( m_env, m_fn.c_str() );

	return m_cached_fn;
}

const char *FeVM::transitionTypeStrings[] =
{
		"StartLayout",
		"EndLayout",
		"ToNewSelection",
		"FromOldSelection",
		"ToGame",
		"FromGame",
		"ToNewList",
		"EndNavigation",
		"ShowOverlay",
		"HideOverlay",
		"NewSelOverlay",
		"ChangedTag",
		NULL
};

FeVM::FeVM( FeSettings &fes, FeFontContainer &defaultfont, FeWindow &wnd, FeSound &ambient_sound, bool console_input )
	: FePresent( &fes, defaultfont, wnd ),
	m_window( wnd ),
	m_overlay( NULL ),
	m_ambient_sound( ambient_sound ),
	m_redraw_triggered( false ),
	m_process_console_input( console_input ),
	m_script_cfg( NULL ),
	m_script_id( -1 )
{
	srand( time( NULL ) );
	vm_init();

	//
	// Read the "non-volatile" table (fe.nv) from the filesystem now
	//
	std::string filename = m_feSettings->get_config_dir();
	filename += FE_SCRIPT_NV_FILE;

	nowide::ifstream infile( filename.c_str() );
	if ( !infile.is_open() )
		return;

	std::string nv;
	while ( infile.good() )
	{
		std::string line;
		getline( infile, line );
		nv += line;
	}

	Sqrat::Table fe;
	Sqrat::RootTable().Bind( _SC("fe"),  fe );
	write_non_volatile_from_string( "nv", nv );
}

FeVM::~FeVM()
{
	clear();

	//
	// Save the "non-volatile" squirrel table (fe.nv) to a file now
	//
	std::string filename = m_feSettings->get_config_dir();
	filename += FE_SCRIPT_NV_FILE;

	nowide::ofstream outfile( filename.c_str() );
	if ( outfile.is_open() )
	{
		outfile << read_non_volatile_to_string( _SC( "nv" ) ) << std::endl;
		outfile.close();
	}

	vm_close();
}

void FeVM::set_overlay( FeOverlay *feo )
{
	m_overlay = feo;
}

bool FeVM::poll_command( FeInputMap::Command &c, sf::Event &ev, bool &from_ui )
{
	from_ui=false;

	if ( !m_posted_commands.empty() )
	{
		c = (FeInputMap::Command)m_posted_commands.front();
		m_posted_commands.pop();
		ev.type = sf::Event::Count;

		return true;
	}
	else if ( m_window.pollEvent( ev ) )
	{
		int t = m_layoutTimer.getElapsedTime().asMilliseconds();

		// Debounce to stop multiples when triggered by a key combo
		//
		if ( t - m_last_ui_cmd.asMilliseconds() < 30 )
			return false;

		c = m_feSettings->map_input( ev );

		if ( c != FeInputMap::LAST_COMMAND )
			m_last_ui_cmd = m_layoutTimer.getElapsedTime();

		from_ui = true;
		return true;
	}

	return false;
}

void FeVM::clear()
{
	FePresent::clear();

	m_last_ui_cmd = sf::Time();
	m_ticks.clear();
	m_trans.clear();
	m_sig_handlers.clear();

	while ( !m_posted_commands.empty() )
		m_posted_commands.pop();
}

void FeVM::add_ticks_callback( Sqrat::Object func, const char *slot )
{
	m_ticks.push_back(
		FeCallback( m_script_id, func, slot, *m_feSettings ) );
}

void FeVM::add_transition_callback( Sqrat::Object func, const char *slot )
{
	m_trans.push_back(
		FeCallback( m_script_id, func, slot, *m_feSettings ) );
}

void FeVM::add_signal_handler( Sqrat::Object func, const char *slot )
{
	m_sig_handlers.push_back(
		FeCallback( m_script_id, func, slot, *m_feSettings ) );
}

void FeVM::remove_signal_handler( Sqrat::Object func, const char *slot )
{
	for ( std::vector<FeCallback>::iterator itr = m_sig_handlers.begin();
			itr != m_sig_handlers.end(); ++itr )
	{
		if (( (*itr).m_fn.compare( slot ) == 0 )
				&& ( fe_obj_compare( func.GetVM(), func.GetObject(), (*itr).m_env.GetObject() ) == 0 ))
		{
			m_sig_handlers.erase( itr );
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

	fe_register_global_func( vm, zip_extract_file, "zip_extract_file" );
	fe_register_global_func( vm, zip_get_dir, "zip_get_dir" );

	Sqrat::DefaultVM::Set( vm );
}

void FeVM::update_to_new_list( int var, bool reset_display, bool suppress_transition )
{
	if ( reset_display )
	{
		//
		// Populate script arrays that may change from display to display (currently just fe.filters)
		//
		Sqrat::Table fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );

		Sqrat::Table ftab;  // hack Table to Array because creating the Array straight up doesn't work
		fe.Bind( _SC("filters"), ftab );
		Sqrat::Array farray( ftab.GetObject() );

		farray.Resize( 0 );

		FeDisplayInfo *di = m_feSettings->get_display( m_feSettings->get_current_display_index() );
		if ( di )
		{
			for ( int i=0; i < di->get_filter_count(); i++ )
				farray.SetInstance( farray.GetSize(), di->get_filter( i ) );
		}
	}

	FePresent::update_to_new_list( var, reset_display, suppress_transition );
}

namespace
{
	bool nesting_compare( FeBaseTextureContainer *one, FeBaseTextureContainer *two )
	{
		if ( one->get_presentable_parent() != NULL && two->get_presentable_parent() == NULL )
			return true;

		if ( one->get_presentable_parent() == NULL && two->get_presentable_parent() == NULL )
			return false;

		if ( one->get_presentable_parent() != NULL && two->get_presentable_parent() != NULL )
			return ( one->get_presentable_parent()->get_nesting_level() > two->get_presentable_parent()->get_nesting_level() );

		return false;
	}
};

bool FeVM::on_new_layout()
{
	using namespace Sqrat;

	const FeLayoutInfo &layout_params
		= m_feSettings->get_current_config( FeSettings::Current );


	// Grab the contents of the existing "non-volatile" squirrel table
	// before reinitializing
	//
	std::string nv = read_non_volatile_to_string( _SC( "nv" ) );

	// Squirrel VM gets reinitialized on each layout
	//
	vm_init();

	FeSettings::FePresentState ps = m_feSettings->get_present_state();

	// Set fe-related constants
	//

	ConstTable()
		.Const( _SC("FeVersion"), FE_VERSION)
		.Const( _SC("FeVersionNum"), FE_VERSION_NUM)
		.Const( _SC("ScreenWidth"), (int)m_mon[0].size.x )
		.Const( _SC("ScreenHeight"), (int)m_mon[0].size.y )
		.Const( _SC("ScreenRefreshRate"), m_refresh_rate )
		.Const( _SC("ScreenSaverActive"), ( ps == FeSettings::ScreenSaver_Showing ) )
		.Const( _SC("IntroActive"), ( ps == FeSettings::Intro_Showing ) )
		.Const( _SC("OS"), get_OS_string() )
		.Const( _SC("ShadersAvailable"), sf::Shader::isAvailable() )
		.Const( _SC("FeConfigDirectory"), m_feSettings->get_config_dir().c_str() )
#ifdef DATA_PATH
		.Const( _SC("FeDataDirectory"), DATA_PATH )
#else
		.Const( _SC("FeDataDirectory"), "" )
#endif
		.Const( _SC("Language"), m_feSettings->get_info( FeSettings::Language ).c_str() )

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
			.Const( _SC("TopLeft"), FeTextPrimative::Top | FeTextPrimative::Left)
			.Const( _SC("TopCentre"), FeTextPrimative::Top | FeTextPrimative::Centre )
			.Const( _SC("TopRight"), FeTextPrimative::Top | FeTextPrimative::Right )
			.Const( _SC("BottomLeft"), FeTextPrimative::Bottom | FeTextPrimative::Left )
			.Const( _SC("BottomCentre"), FeTextPrimative::Bottom | FeTextPrimative::Centre )
			.Const( _SC("BottomRight"), FeTextPrimative::Bottom | FeTextPrimative::Right )
			.Const( _SC("MiddleLeft"), FeTextPrimative::Middle | FeTextPrimative::Left )
			.Const( _SC("MiddleCentre"), FeTextPrimative::Middle | FeTextPrimative::Centre )
			.Const( _SC("MiddleRight"), FeTextPrimative::Middle | FeTextPrimative::Right )
			)
		.Enum( _SC("RotateScreen"), Enumeration()
			.Const( _SC("None"), FeSettings::RotateNone )
			.Const( _SC("Right"), FeSettings::RotateRight )
			.Const( _SC("Flip"), FeSettings::RotateFlip )
			.Const( _SC("Left"), FeSettings::RotateLeft )
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
		.Enum( _SC("Art"), Enumeration()
			.Const( _SC("Default"), AF_Default )
			.Const( _SC("ImagesOnly"), AF_ImagesOnly )
			.Const( _SC("FullList"), AF_FullList )
			)
		.Enum( _SC("Overlay"), Enumeration()
			.Const( "Custom", 0 )
			.Const( "Exit", FeInputMap::Exit )
			.Const( "Displays", FeInputMap::DisplaysMenu )
			.Const( "Filters", FeInputMap::FiltersMenu )
			.Const( "Tags", FeInputMap::ToggleTags )
			.Const( "Favourite", FeInputMap::ToggleFavourite )
			)
		.Enum( _SC("PathTest"), Enumeration()
			.Const( "IsFileOrDirectory", IsFileOrDirectory )
			.Const( "IsFile", IsFile )
			.Const( "IsDirectory", IsDirectory )
			.Const( "IsRelativePath", IsRelativePath )
			.Const( "IsSupportedArchive", IsSupportedArchive )
			.Const( "IsSupportedMedia", IsSupportedMedia )
			)
		.Enum( _SC("BlendMode"), Enumeration()
			.Const( _SC("Alpha"), FeBlend::Alpha )
			.Const( _SC("Add"), FeBlend::Add )
			.Const( _SC("Subtract"), FeBlend::Subtract )
			.Const( _SC("Screen"), FeBlend::Screen )
			.Const( _SC("Multiply"), FeBlend::Multiply )
			.Const( _SC("Overlay"), FeBlend::Overlay )
			.Const( _SC("Premultiplied"), FeBlend::Premultiplied )
			.Const( _SC("None"), FeBlend::None )
			)
		;

	Enumeration info;
	int i=0;
	while ( FeRomInfo::indexStrings[i] != NULL )
	{
		info.Const( FeRomInfo::indexStrings[i], i );
		i++;
	}
	info.Const( "System", FeRomInfo::LAST_INDEX ); // special cases with same value
	info.Const( "NoSort", FeRomInfo::LAST_INDEX ); //

	info.Const( "Overview", FeRomInfo::LAST_INDEX+1 ); //
	info.Const( "IsPaused", FeRomInfo::LAST_INDEX+2 ); //
	info.Const( "SortValue", FeRomInfo::LAST_INDEX+3 ); //
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
		.Prop(_SC("filter_offset"), &FeBasePresentable::getFilterOffset, &FeBasePresentable::setFilterOffset )
		.Prop(_SC("shader"), &FeBasePresentable::script_get_shader, &FeBasePresentable::script_set_shader )
		.Prop(_SC("zorder"), &FeBasePresentable::get_zorder, &FeBasePresentable::set_zorder )
		.Func( _SC("set_rgb"), &FeBasePresentable::set_rgb )
		.Overload<void (FeBasePresentable::*)(float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
		.Overload<void (FeBasePresentable::*)(float, float, float, float)>(_SC("set_pos"), &FeBasePresentable::set_pos)
	);

	fe.Bind( _SC("Image"),
		DerivedClass<FeImage, FeBasePresentable, NoConstructor>()
		.Prop(_SC("origin_x"), &FeImage::get_origin_x, &FeImage::set_origin_x )
		.Prop(_SC("origin_y"), &FeImage::get_origin_y, &FeImage::set_origin_y )
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
		.Prop(_SC("sample_aspect_ratio"), &FeImage::get_sample_aspect_ratio )
		// "movie_enabled" deprecated as of version 1.3, use the video_flags property instead:
		.Prop(_SC("movie_enabled"), &FeImage::getMovieEnabled, &FeImage::setMovieEnabled )
		.Prop(_SC("video_flags"), &FeImage::getVideoFlags, &FeImage::setVideoFlags )
		.Prop(_SC("video_playing"), &FeImage::getVideoPlaying, &FeImage::setVideoPlaying )
		.Prop(_SC("video_duration"), &FeImage::getVideoDuration )
		.Prop(_SC("video_time"), &FeImage::getVideoTime )
		.Prop(_SC("preserve_aspect_ratio"), &FeImage::get_preserve_aspect_ratio,
				&FeImage::set_preserve_aspect_ratio )
		.Prop(_SC("file_name"), &FeImage::getFileName, &FeImage::setFileName )
		.Prop(_SC("trigger"), &FeImage::getTrigger, &FeImage::setTrigger )
		.Prop(_SC("smooth"), &FeImage::get_smooth, &FeImage::set_smooth )
		.Prop( _SC("blend_mode"), &FeImage::get_blend_mode, &FeImage::set_blend_mode )
		.Prop(_SC("mipmap"), &FeImage::get_mipmap, &FeImage::set_mipmap )
		.Func( _SC("swap"), &FeImage::transition_swap )
		.Func( _SC("rawset_index_offset"), &FeImage::rawset_index_offset )
		.Func( _SC("rawset_filter_offset"), &FeImage::rawset_filter_offset )
		.Func(_SC("fix_masked_image"), &FeImage::fix_masked_image)
		.Func(_SC("load_from_archive"), &FeImage::loadFromArchive)

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
		.Prop(_SC("msg_wrapped"), &FeText::get_string_wrapped )
		.Prop(_SC("bg_red"), &FeText::get_bgr, &FeText::set_bgr )
		.Prop(_SC("bg_green"), &FeText::get_bgg, &FeText::set_bgg )
		.Prop(_SC("bg_blue"), &FeText::get_bgb, &FeText::set_bgb )
		.Prop(_SC("bg_alpha"), &FeText::get_bga, &FeText::set_bga )
		// "charsize" deprecated, use the char_size property instead
		.Prop(_SC("charsize"), &FeText::get_charsize, &FeText::set_charsize )
		.Prop(_SC("char_size"), &FeText::get_charsize, &FeText::set_charsize )
		.Prop(_SC("glyph_size"), &FeText::get_glyph_size )
		.Prop(_SC("char_spacing"), &FeText::get_spacing, &FeText::set_spacing )
		.Prop(_SC("line_spacing"), &FeText::get_line_spacing, &FeText::set_line_spacing )
		.Prop(_SC("style"), &FeText::get_style, &FeText::set_style )
		.Prop(_SC("align"), &FeText::get_align, &FeText::set_align )
		.Prop(_SC("word_wrap"), &FeText::get_word_wrap, &FeText::set_word_wrap )
		.Prop(_SC("first_line_hint"), &FeText::get_first_line_hint, &FeText::set_first_line_hint )
		.Prop(_SC("msg_width"), &FeText::get_actual_width )
		.Prop(_SC("font"), &FeText::get_font, &FeText::set_font )
		// "nomargin" deprecated, use the margin property instead
		.Prop(_SC("nomargin"), &FeText::get_no_margin, &FeText::set_no_margin )
		.Prop(_SC("margin"), &FeText::get_margin, &FeText::set_margin )
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
		.Prop(_SC("list_size"), &FeListBox::get_list_size )
		// "charsize" deprecated, use the char_size property instead
		.Prop(_SC("charsize"), &FeListBox::get_charsize, &FeListBox::set_charsize )
		.Prop(_SC("char_size"), &FeListBox::get_charsize, &FeListBox::set_charsize )
		.Prop(_SC("glyph_size"), &FeListBox::get_glyph_size )
		.Prop(_SC("char_spacing"), &FeListBox::get_spacing, &FeListBox::set_spacing )
		.Prop(_SC("style"), &FeListBox::get_style, &FeListBox::set_style )
		.Prop(_SC("align"), &FeListBox::get_align, &FeListBox::set_align )
		.Prop(_SC("sel_style"), &FeListBox::getSelStyle, &FeListBox::setSelStyle )
		.Prop(_SC("font"), &FeListBox::get_font, &FeListBox::set_font )
		// "nomargin" deprecated, use the margin property instead
		.Prop(_SC("nomargin"), &FeListBox::get_no_margin, &FeListBox::set_no_margin )
		.Prop(_SC("margin"), &FeListBox::get_margin, &FeListBox::set_margin )
		.Prop(_SC("format_string"), &FeListBox::get_format_string, &FeListBox::set_format_string )
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
		.Prop( _SC("page_size"), &FePresent::get_page_size, &FePresent::set_page_size )
		.Prop(_SC("preserve_aspect_ratio"), &FePresent::get_preserve_aspect_ratio, &FePresent::set_preserve_aspect_ratio )
		.Prop(_SC("time"), &FePresent::get_layout_ms )
	);

	fe.Bind( _SC("CurrentList"), Class <FePresent, NoConstructor>()
		.Prop( _SC("name"), &FePresent::get_display_name )
		.Prop( _SC("display_index"), &FePresent::get_display_index )
		.Prop( _SC("index"), &FePresent::get_selection_index, &FePresent::set_selection_index )
		.Prop( _SC("filter_index"), &FePresent::get_filter_index, &FePresent::set_filter_index )
		.Prop( _SC("search_rule"), &FePresent::get_search_rule, &FePresent::set_search_rule )
		.Prop( _SC("size"), &FePresent::get_current_filter_size )

		// The following are deprecated as of version 1.5 in favour of using the fe.filters array:
		.Prop( _SC("filter"), &FePresent::get_filter_name )	// deprecated as of 1.5
		.Prop( _SC("sort_by"), &FePresent::get_sort_by )		// deprecated as of 1.5
		.Prop( _SC("reverse_order"), &FePresent::get_reverse_order ) // deprecated as of 1.5
		.Prop( _SC("list_limit"), &FePresent::get_list_limit ) // deprecated as of 1.5
	);

	fe.Bind( _SC("Overlay"), Class <FeVM, NoConstructor>()
		.Prop( _SC("is_up"), &FeVM::overlay_is_on )
		.Overload<void (FeVM::*)(FeText *, FeListBox *)>(_SC("set_custom_controls"), &FeVM::overlay_set_custom_controls)
		.Overload<void (FeVM::*)(FeText *)>(_SC("set_custom_controls"), &FeVM::overlay_set_custom_controls)
		.Overload<void (FeVM::*)()>(_SC("set_custom_controls"), &FeVM::overlay_set_custom_controls)
//		.Func( _SC("set_custom_controls"), &FeVM::overlay_set_custom_controls)
		.Func( _SC("clear_custom_controls"), &FeVM::overlay_clear_custom_controls)
		.Overload<int (FeVM::*)(Array, const char *, int, int)>(_SC("list_dialog"), &FeVM::list_dialog)
		.Overload<int (FeVM::*)(Array, const char *, int)>(_SC("list_dialog"), &FeVM::list_dialog)
		.Overload<int (FeVM::*)(Array, const char *)>(_SC("list_dialog"), &FeVM::list_dialog)
		.Overload<int (FeVM::*)(Array)>(_SC("list_dialog"), &FeVM::list_dialog)
		.Func( _SC("edit_dialog"), &FeVM::edit_dialog )
		.Overload<bool (FeVM::*)(const char *, const char *)>( _SC("splash_message"), &FeVM::splash_message )
		.Overload<bool (FeVM::*)(const char *)>( _SC("splash_message"), &FeVM::splash_message )
	);

	fe.Bind( _SC("Sound"), Class <FeSound, NoConstructor>()
		.Prop( _SC("file_name"), &FeSound::get_file_name, &FeSound::set_file_name )
		.Prop( _SC("playing"), &FeSound::get_playing, &FeSound::set_playing )
		.Prop( _SC("loop"), &FeSound::get_loop, &FeSound::set_loop )
		.Prop( _SC("pitch"), &FeSound::get_pitch, &FeSound::set_pitch )
		.Prop( _SC("x"), &FeSound::get_x, &FeSound::set_x )
		.Prop( _SC("y"), &FeSound::get_y, &FeSound::set_y )
		.Prop( _SC("z"), &FeSound::get_z, &FeSound::set_z )
		.Prop(_SC("duration"), &FeSound::get_duration )
		.Prop(_SC("time"), &FeSound::get_time )
		.Func( _SC("get_metadata"), &FeSound::get_metadata )
		.Func( _SC("load_from_archive"), &FeSound::load_from_archive )
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

	fe.Bind( _SC("Display"), Class <FeDisplayInfo, NoConstructor>()
		.Prop( _SC("name"), &FeDisplayInfo::get_name )
		.Prop( _SC("layout"), &FeDisplayInfo::get_layout )
		.Prop( _SC("romlist"), &FeDisplayInfo::get_romlist_name )
		.Prop( _SC("in_cycle"), &FeDisplayInfo::show_in_cycle )
		.Prop( _SC("in_menu"), &FeDisplayInfo::show_in_menu )
	);

	fe.Bind( _SC("Filter"), Class <FeFilter, NoConstructor>()
		.Prop( _SC("name"), &FeFilter::get_name )
		.Prop( _SC("index"), &FeFilter::get_rom_index )
		.Prop( _SC("size"), &FeFilter::get_size )
		.Prop( _SC("sort_by"), &FeFilter::get_sort_by )
		.Prop( _SC("reverse_order"), &FeFilter::get_reverse_order )
		.Prop( _SC("list_limit"), &FeFilter::get_list_limit )
	);

	fe.Bind( _SC("PresentableParent"), Class <FePresentableParent, NoConstructor>()
		.Overload<FeImage * (FePresentableParent::*)(const char *, int, int, int, int)>(_SC("add_image"), &FePresentableParent::add_image)
		.Overload<FeImage * (FePresentableParent::*)(const char *, int, int)>(_SC("add_image"), &FePresentableParent::add_image)
		.Overload<FeImage * (FePresentableParent::*)(const char *)>(_SC("add_image"), &FePresentableParent::add_image)
		.Overload<FeImage * (FePresentableParent::*)(const char *, int, int, int, int)>(_SC("add_artwork"), &FePresentableParent::add_artwork)
		.Overload<FeImage * (FePresentableParent::*)(const char *, int, int)>(_SC("add_artwork"), &FePresentableParent::add_artwork)
		.Overload<FeImage * (FePresentableParent::*)(const char *)>(_SC("add_artwork"), &FePresentableParent::add_artwork)
		.Func( _SC("add_clone"), &FePresentableParent::add_clone )
		.Func( _SC("add_text"), &FePresentableParent::add_text )
		.Func( _SC("add_listbox"), &FePresentableParent::add_listbox )
		.Func( _SC("add_surface"), &FePresentableParent::add_surface )
	);

	fe.Bind( _SC("Monitor"),
		DerivedClass<FeMonitor, FePresentableParent, NoConstructor>()
		.Prop( _SC("num"), &FeMonitor::get_num )
		.Prop( _SC("width"), &FeMonitor::get_width )
		.Prop( _SC("height"), &FeMonitor::get_height )
	);

	fe.Bind( _SC("ImageCache"), Class <FeImageLoader, NoConstructor>()
		.Prop( _SC("max_size"), &FeImageLoader::cache_max )
		.Prop( _SC("size"), &FeImageLoader::cache_size )
		.Prop( _SC("count"), &FeImageLoader::cache_count )
		.Func( _SC("add_image"), &FeImageLoader::cache_image )
		.Func( _SC("name_at"), &FeImageLoader::cache_get_name_at )
		.Func( _SC("size_at"), &FeImageLoader::cache_get_size_at )
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
	fe.Overload<FeSound* (*)(const char *, bool)>(_SC("add_sound"), &FeVM::cb_add_sound);
	fe.Overload<FeSound* (*)(const char *)>(_SC("add_sound"), &FeVM::cb_add_sound);
	fe.Overload<FeShader* (*)(int, const char *, const char *)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Overload<FeShader* (*)(int, const char *)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Overload<FeShader* (*)(int)>(_SC("add_shader"), &FeVM::cb_add_shader);
	fe.Overload<void (*)(const char *)>(_SC("add_ticks_callback"), &FeVM::cb_add_ticks_callback);
	fe.Overload<void (*)(Object, const char *)>(_SC("add_ticks_callback"), &FeVM::cb_add_ticks_callback);
	fe.Overload<void (*)(const char *)>(_SC("add_transition_callback"), &FeVM::cb_add_transition_callback);
	fe.Overload<void (*)(Object, const char *)>(_SC("add_transition_callback"), &FeVM::cb_add_transition_callback);
	fe.Overload<void (*)(const char *)>(_SC("add_signal_handler"), &FeVM::cb_add_signal_handler);
	fe.Overload<void (*)(Object, const char *)>(_SC("add_signal_handler"), &FeVM::cb_add_signal_handler);
	fe.Overload<void (*)(const char *)>(_SC("remove_signal_handler"), &FeVM::cb_remove_signal_handler);
	fe.Overload<void (*)(Object, const char *)>(_SC("remove_signal_handler"), &FeVM::cb_remove_signal_handler);
	fe.Func<bool (*)(const char *)>(_SC("get_input_state"), &FeVM::cb_get_input_state);
	fe.Func<int (*)(const char *)>(_SC("get_input_pos"), &FeVM::cb_get_input_pos);
	fe.Func<void (*)(const char *)>(_SC("do_nut"), &FeVM::do_nut);
	fe.Func<bool (*)(const char *)>(_SC("load_module"), &FeVM::load_module);
	fe.Overload<const char* (*)(int)>(_SC("game_info"), &FeVM::cb_game_info);
	fe.Overload<const char* (*)(int, int)>(_SC("game_info"), &FeVM::cb_game_info);
	fe.Overload<const char* (*)(int, int, int)>(_SC("game_info"), &FeVM::cb_game_info);
	fe.Overload<const char* (*)(const char *, int, int, int)>(_SC("get_art"), &FeVM::cb_get_art);
	fe.Overload<const char* (*)(const char *, int, int)>(_SC("get_art"), &FeVM::cb_get_art);
	fe.Overload<const char* (*)(const char *, int)>(_SC("get_art"), &FeVM::cb_get_art);
	fe.Overload<const char* (*)(const char *)>(_SC("get_art"), &FeVM::cb_get_art);
	fe.Overload<bool (*)(const char *, const char *, Object, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
	fe.Overload<bool (*)(const char *, const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
	fe.Overload<bool (*)(const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
	fe.Func<bool (*)(const char *, const char *)>(_SC("plugin_command_bg"), &FeVM::cb_plugin_command_bg);
	fe.Func<const char* (*)(const char *)>(_SC("path_expand"), &FeVM::cb_path_expand);
	fe.Func<bool (*)(const char *, int)>(_SC("path_test"), &FeVM::cb_path_test);
	fe.Func<Table (*)()>(_SC("get_config"), &FeVM::cb_get_config);
	fe.Func<void (*)(const char *)>(_SC("signal"), &FeVM::cb_signal);
	fe.Overload<void (*)(int, bool)>(_SC("set_display"), &FeVM::cb_set_display);
	fe.Overload<void (*)(int)>(_SC("set_display"), &FeVM::cb_set_display);
	fe.Overload<const char *(*)(const char *)>(_SC("get_text"), &FeVM::cb_get_text);

	//
	// Define variables that get exposed to Squirrel
	//

	//
	// fe.displays
	//
	Table dtab;  // hack Table to Array because creating the Array straight up doesn't work
	fe.Bind( _SC("displays"), dtab );
	Array darray( dtab.GetObject() );

	int display_count = m_feSettings->displays_count();
	for ( i=0; i< display_count; i++ )
		darray.SetInstance( darray.GetSize(),
			m_feSettings->get_display( i ) );

	//
	// Note the fe.filters array also gets repopulated in call to FeVM::update_to_new_list(), since it
	// gets reset even when the layout itself isn't necessarily reloaded (i.e. when navigating between
	// displays that use the same layout)
	//
	Table ftab;  // hack Table to Array because creating the Array straight up doesn't work
	fe.Bind( _SC("filters"), ftab );

	FeDisplayInfo *di = m_feSettings->get_display( m_feSettings->get_current_display_index() );
	if ( di )
	{
		Array farray( ftab.GetObject() );

		for ( i=0; i < di->get_filter_count(); i++ )
			farray.SetInstance( farray.GetSize(), di->get_filter( i ) );
	}

	//
	// fe.monitors
	//
	Table mtab;  // hack Table to Array because creating the Array straight up doesn't work
	fe.Bind( _SC("monitors"), mtab );
	Array marray( mtab.GetObject() );

	for ( i=0; i < (int)m_mon.size(); i++ )
		marray.SetInstance( marray.GetSize(), &m_mon[i] );

	fe.SetInstance( _SC("layout"), (FePresent *)this );
	fe.SetInstance( _SC("list"), (FePresent *)this );
	fe.SetInstance( _SC("overlay"), this );
	fe.SetInstance( _SC("ambient_sound"), &m_ambient_sound );

	FeImageLoader &il = FeImageLoader::get_ref();
	fe.SetInstance( _SC("image_cache"), &il );
	fe.SetValue( _SC("plugin"), Table() ); // an empty table for plugins to use/abuse

	// We keep a "non-volatile" table for use by layouts/plugins, the
	// string content of which gets copied over from layout to layout
	//
	fe.SetValue( _SC("nv"), Table() );

	// Each presentation object gets an instance in the
	// "obj" array available in Squirrel
	//
	Table obj; // this must created as a Table (even though it is used as an Array) bug in sqrat?
	fe.Bind( _SC("obj"), obj );
	RootTable().Bind( _SC("fe"),  fe );

	write_non_volatile_from_string( "nv", nv );

	//
	// Run the layout script
	//
	std::string path, filename;
	bool skip_layout = !m_feSettings->get_path( FeSettings::Current, path, filename );

	std::string rep_path( path );

	fe.SetValue( _SC("script_dir"), path );
	fe.SetValue( _SC("script_file"), filename );
	m_script_cfg = &layout_params;
	m_script_id = -1;

	if ( filename.empty() )
	{
		if ( ps == FeSettings::Intro_Showing )
			return false; // silent fail if intro is not found
		else
		{
			// if there is no script file at this point,
			// we try loader script instead
			//
			m_feSettings->get_path( FeSettings::Loader,
				path, filename );

			fe.SetValue( _SC("loader_dir"), path );
		}
	}

	if ( skip_layout )
	{
		FeDisplayInfo *di = m_feSettings->get_display(
			m_feSettings->get_current_display_index() );

		if ( di )
			FeLog() << " ! Error opening layout: "
				<< di->get_info( FeDisplayInfo::Layout ) << std::endl;
	}
	else if ( !run_script( path, filename ) )
	{
		if ( ps == FeSettings::Intro_Showing )
			return false; // silent fail if intro is not found
		else
			FeLog() << " ! Script file not found: " << path
				<< " (" << filename << ")" << std::endl;
	}

	//
	// Now run any plugin script(s)
	//
	const std::vector< FePlugInfo > &plugins = m_feSettings->get_plugins();

	for ( int i=0; i<(int)plugins.size(); i++ )
	{
		// Don't run disabled plugins...
		if ( plugins[i].get_enabled() == false )
			continue;

		std::string plug_path, plug_name;
		m_feSettings->get_plugin_full_path(
				plugins[i].get_name(),
				plug_path,
				plug_name );

		if ( !plug_name.empty() )
		{
			fe.SetValue( _SC("script_dir"), plug_path );
			fe.SetValue( _SC("script_file"), plug_name );
			m_script_cfg = &(plugins[i]);
			m_script_id = i;

			run_script( plug_path, plug_name );
		}
	}

	fe.SetValue( _SC("script_dir"), "" );
	fe.SetValue( _SC("script_file"), "" );
	m_script_cfg = NULL;

	if ( !skip_layout && ( ps == FeSettings::Layout_Showing ))
	{
		FeLog() << " - Loaded layout: " << rep_path
			<< " (" << filename << ")" << std::endl;
	}

	// To avoid frame delay of nested surfaces we have to sort them here
	// so the most nested ones are redrawn first
	FePresent *fep = FePresent::script_get_fep();
	std::stable_sort( fep->m_texturePool.begin(), fep->m_texturePool.end(), nesting_compare );

	return true;
}

void FeVM::set_for_callback( const FeCallback &c )
{
	Sqrat::Table fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );

	fe.SetValue( _SC("script_dir"), c.m_path );
	fe.SetValue( _SC("script_file"), c.m_file );

	m_script_cfg = c.m_cfg;
	m_script_id = c.m_sid;
}

bool FeVM::process_console_input()
{
	if ( !m_process_console_input )
		return false;

	std::string script;
	if ( !get_console_stdin( script ) )
		return false;

	bool retval=false;
	try
	{
		Sqrat::Script sc;
		sc.CompileString( script );
		sc.Run();

		retval = true;
	}
	catch( Sqrat::Exception e )
	{
		FeLog() << "Error: " << script << " - " << e.Message() << std::endl;
	}

	return retval;
}

bool FeVM::on_tick()
{
	using namespace Sqrat;
	m_redraw_triggered = process_console_input();

	for ( std::vector<FeCallback>::iterator itr = m_ticks.begin();
		itr != m_ticks.end(); )
	{
		// Assumption: Ticks list is empty if no vm is active
		//
		ASSERT( DefaultVM::Get() );

		set_for_callback( *itr );
		bool remove=false;
		try
		{
			Function &func = (*itr).get_fn();
			if ( !func.IsNull() )
				func.Execute( m_layoutTimer.getElapsedTime().asMilliseconds() );
		}
		catch( Exception &e )
		{
			FeLog() << "Script Error in tick function: " << (*itr).m_fn << " - "
					<< e.Message() << std::endl;

			// Knock out this entry.   If it causes a script error, we don't
			// want to call it anymore
			//
			remove=true;
		}

		if ( remove )
			itr = m_ticks.erase( itr );
		else
			++itr;
	}

	return m_redraw_triggered;
}

void FeVM::on_transition(
	FeTransitionType t,
	int var )
{
	using namespace Sqrat;

	FeDebug() << "[Transition] type=" << transitionTypeStrings[t] << ", var=" << var << std::endl;

	sf::Clock ttimer;

	std::vector<FeCallback *> worklist( m_trans.size() );
	for ( unsigned int i=0; i < m_trans.size(); i++ )
		worklist[i] = &(m_trans[i]);

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
		for ( std::vector<FeCallback *>::iterator itr=worklist.begin();
			itr != worklist.end(); )
		{
			set_for_callback( *(*itr) );
			bool keep=false;
			try
			{
				Function &func = (*itr)->get_fn();
				if ( !func.IsNull() )
				{
					keep = func.Evaluate<bool>(
						(int)t,
						var,
						ttimer.getElapsedTime().asMilliseconds() );
				}
			}
			catch( Exception &e )
			{
				FeLog() << "Script Error in transition function: " << (*itr)->m_fn
						<< " - " << e.Message() << std::endl;
			}

			if ( !keep )
				itr = worklist.erase( itr );
			else
				++itr;
		}

		// redraw now if we are doing another pass...
		//
		if (( !worklist.empty() ) && ( m_window.isOpen() ))
		{
			video_tick();

			redraw_surfaces();
			m_window.clear();
			m_window.draw( *this );
			m_window.display();

#ifdef SFML_SYSTEM_LINUX
			//
			// On SFML 2.2-2.3 Linux, I am getting flicker on a
			// multi monitor setup during animated transitions.
			// Processing window events between each draw fixes
			// it.
			//
			// TODO: It is probably a good idea to do this for
			// every platform... needs investigation.
			//
			sf::Event ev;
			while (m_window.pollEvent(ev))
			{
				//sf::sleep( sf::milliseconds( 10 ) );
			}
#endif
		}
	}
}

bool FeVM::script_handle_event( FeInputMap::Command c )
{
	using namespace Sqrat;

	//
	// Go through the list in reverse so that the most recently registered signal handler
	// gets the first shot at handling the signal.
	//
	for ( std::vector<FeCallback>::reverse_iterator itr = m_sig_handlers.rbegin();
		itr != m_sig_handlers.rend(); ++itr )
	{
		// Assumption: Handlers list is empty if no vm is active
		//
		ASSERT( DefaultVM::Get() );
		set_for_callback( *itr );

		try
		{
			Function &func = (*itr).get_fn();
			if (( !func.IsNull() )
					&& ( func.Evaluate<bool>( FeInputMap::commandStrings[ c ] )))
				return true;
		}
		catch( Exception &e )
		{
			FeLog() << "Script Error in signal handler: " << (*itr).m_fn << " - "
					<< e.Message() << std::endl;
		}
	}

	return false;
}

int FeVM::list_dialog( Sqrat::Array t, const char *title, int default_sel, int cancel_sel )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();

	if ( m_overlay->overlay_is_on() )
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
		return m_overlay->common_list_dialog(
				std::string( title ),
				list_entries,
				default_sel,
				cancel_sel );
	}
	else
	{
		return m_overlay->common_basic_dialog(
				std::string( title ),
				list_entries,
				default_sel,
				cancel_sel );
	}
}

int FeVM::list_dialog( Sqrat::Array t, const char *title, int default_sel )
{
	return list_dialog( t, title, default_sel, -1 );
}

int FeVM::list_dialog( Sqrat::Array t, const char *title )
{
	return list_dialog( t, title, 0, -1 );
}

int FeVM::list_dialog( Sqrat::Array t )
{
	return list_dialog( t, NULL, 0, -1 );
}

const char *FeVM::edit_dialog( const char *msg, const char *txt )
{
	static std::string local_copy;
	local_copy = txt;

	if ( !m_overlay->overlay_is_on() )
		m_overlay->edit_dialog( msg, local_copy );

	return local_copy.c_str();
}

bool FeVM::overlay_is_on()
{
	return m_overlay->overlay_is_on();
}

void FeVM::overlay_set_custom_controls( FeText *caption, FeListBox *opts )
{
	m_overlay_caption = caption;
	m_overlay_lb = opts;
	m_custom_overlay = true;
}

void FeVM::overlay_set_custom_controls( FeText *caption )
{
	overlay_set_custom_controls( caption, NULL );
}

void FeVM::overlay_set_custom_controls()
{
	overlay_set_custom_controls( NULL, NULL );
}

void FeVM::overlay_clear_custom_controls()
{
	m_overlay_caption = NULL;
	m_overlay_lb = NULL;
	m_custom_overlay = false;
}

bool FeVM::splash_message( const char *msg, const char *aux )
{
	m_overlay->splash_message( msg, aux );
	return m_overlay->check_for_cancel();
}

bool FeVM::splash_message( const char *msg )
{
	return splash_message( msg, "" );
}

//
// Script static functions
//
FePresent *FePresent::script_get_fep()
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( vm )
		return (FePresent *)sq_getforeignptr( vm );

	return NULL;
}

void FePresent::script_process_magic_strings( std::string &str,
		int filter_offset,
		int index_offset )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	if ( !vm )
		return;

	const char *TOK = "[!";
	int TOK_LEN = 2;

	size_t pos = str.find( TOK );
	while ( pos != std::string::npos )
	{
		size_t end = str.find_first_of( ']', pos + TOK_LEN );
		if ( end == std::string::npos )
			break;

		std::string magic = str.substr( pos + TOK_LEN, end-pos-TOK_LEN );

		try
		{
			Sqrat::Function func( Sqrat::RootTable(), magic.c_str() );
			if ( !func.IsNull() )
			{
				std::string result;

				switch ( fe_get_num_params( vm, func.GetFunc(), func.GetEnv() ) )
				{
				case 2:
					result = func.Evaluate<const char *>( index_offset, filter_offset );
					break;
				case 1:
					result = func.Evaluate<const char *>( index_offset );
					break;
				default:
					result = func.Evaluate<const char *>();
					break;
				}

				str.replace( pos, end-pos+1, result );
				pos += result.size();
			}
			else
			{
				FeDebug() << "Potential magic string ignored, no corresponding function in script: "
					<< TOK << magic << "]" << std::endl;

				pos += TOK_LEN;
			}
		}
		catch( Sqrat::Exception &e )
		{
			FeLog() << "Script Error in magic string function: "
				<< magic << " - "
				<< e.Message() << std::endl;
		}

		pos = str.find( TOK, pos );
	}
}

//
//
//
class FeConfigVM
{
private:
	HSQUIRRELVM m_stored_vm;
	HSQUIRRELVM m_vm;

public:
	FeConfigVM(
		const FeScriptConfigurable &configurable,
		const std::string &script_path,
		const std::string &script_file,
		bool limited=false )
	{
		m_stored_vm = Sqrat::DefaultVM::Get();
		FeVM *fe_vm = (FeVM *)sq_getforeignptr( m_stored_vm );

		m_vm = sq_open( 1024 );
		sq_setforeignptr( m_vm, fe_vm );
		sq_setprintfunc( m_vm, printFunc, printFunc );
		sq_pushroottable( m_vm );

		if ( !limited )
		{
			sqstd_register_bloblib( m_vm );
			sqstd_register_iolib( m_vm );
			sqstd_register_mathlib( m_vm );
			sqstd_register_stringlib( m_vm );
			sqstd_register_systemlib( m_vm );

#ifdef FE_DEBUG
			// We purposefully do not set this in release builds, because we
			// use this FeConfigVM to run scripts in config mode and often that means
			// they crash and burn, and we don't want to be displaying the error messages
			// for that.
			sqstd_seterrorhandlers( m_vm );
#endif

			fe_register_global_func( m_vm, zip_extract_file, "zip_extract_file" );
			fe_register_global_func( m_vm, zip_get_dir, "zip_get_dir" );
		}

		Sqrat::DefaultVM::Set( m_vm );

		Sqrat::ConstTable()
			.Const( _SC("FeVersion"), FE_VERSION)
			.Const( _SC("FeVersionNum"), FE_VERSION_NUM)
			.Const( _SC("OS"), get_OS_string() )
			.Const( _SC("ShadersAvailable"), sf::Shader::isAvailable() )
			.Enum( _SC("PathTest"), Sqrat::Enumeration()
				.Const( "IsFile", FeVM::IsFile )
				.Const( "IsDirectory", FeVM::IsDirectory )
				.Const( "IsFileOrDirectory", FeVM::IsFileOrDirectory )
				.Const( "IsRelativePath", FeVM::IsRelativePath )
				.Const( "IsSupportedArchive", FeVM::IsSupportedArchive )
				.Const( "IsSupportedMedia", FeVM::IsSupportedMedia )
			);

		Sqrat::ConstTable().Const( _SC("FeConfigDirectory"), fe_vm->m_feSettings->get_config_dir().c_str() );

		Sqrat::Table fe;

		if ( !limited )
		{
			//
			// We only expose a very limited set of frontend functionality
			// to scripts when they are run in the config mode
			//
			fe.Bind( _SC("Overlay"), Sqrat::Class <FeVM, Sqrat::NoConstructor>()
				.Prop( _SC("is_up"), &FeVM::overlay_is_on )
				.Overload<bool (FeVM::*)(const char *, const char *)>( _SC("splash_message"), &FeVM::splash_message )
				.Overload<bool (FeVM::*)(const char *)>( _SC("splash_message"), &FeVM::splash_message )
			);

			fe.SetInstance( _SC("overlay"), fe_vm );

			fe.Bind( _SC("Display"), Sqrat::Class <FeDisplayInfo, Sqrat::NoConstructor>()
				.Prop( _SC("name"), &FeDisplayInfo::get_name )
				.Prop( _SC("layout"), &FeDisplayInfo::get_layout )
				.Prop( _SC("romlist"), &FeDisplayInfo::get_romlist_name )
				.Prop( _SC("in_cycle"), &FeDisplayInfo::show_in_cycle )
				.Prop( _SC("in_menu"), &FeDisplayInfo::show_in_menu )
			);

			fe.Bind( _SC("Filter"), Sqrat::Class <FeFilter, Sqrat::NoConstructor>()
				.Prop( _SC("name"), &FeFilter::get_name )
				.Prop( _SC("index"), &FeFilter::get_rom_index )
				.Prop( _SC("size"), &FeFilter::get_size )
				.Prop( _SC("sort_by"), &FeFilter::get_sort_by )
				.Prop( _SC("reverse_order"), &FeFilter::get_reverse_order )
				.Prop( _SC("list_limit"), &FeFilter::get_list_limit )
			);

			fe.Bind( _SC("Monitor"), Sqrat::Class <FeMonitor, Sqrat::NoConstructor>()
				.Prop( _SC("num"), &FeMonitor::get_num )
				.Prop( _SC("width"), &FeMonitor::get_width )
				.Prop( _SC("height"), &FeMonitor::get_height )
			);

			//
			// fe.displays
			//
			Sqrat::Table dtab;  // hack Table to Array because creating the Array straight up doesn't work
			fe.Bind( _SC("displays"), dtab );
			Sqrat::Array darray( dtab.GetObject() );

			int display_count = fe_vm->m_feSettings->displays_count();
			for ( int i=0; i< display_count; i++ )
				darray.SetInstance( darray.GetSize(),
					fe_vm->m_feSettings->get_display( i ) );

			//
			// fe.filters
			//
			FeDisplayInfo *di = fe_vm->m_feSettings->get_display(
				fe_vm->m_feSettings->get_current_display_index() );

			Sqrat::Table ftab;  // hack Table to Array because creating the Array straight up doesn't work
			fe.Bind( _SC("filters"), ftab );
			Sqrat::Array farray( ftab.GetObject() );

			if ( di )
			{
				for ( int i=0; i < di->get_filter_count(); i++ )
					farray.SetInstance( farray.GetSize(), di->get_filter( i ) );
			}

			// hack Table to Array because creating the Array straight up doesn't work
			Sqrat::Table mtab;
			fe.Bind( _SC("monitors"), mtab );
			Sqrat::Array marray( mtab.GetObject() );

			for ( int i=0; i < (int)fe_vm->m_mon.size(); i++ )
				marray.SetInstance( marray.GetSize(), &(fe_vm->m_mon[i]) );

			fe.Overload<bool (*)(const char *, const char *, Sqrat::Object, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
			fe.Overload<bool (*)(const char *, const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
			fe.Overload<bool (*)(const char *, const char *)>(_SC("plugin_command"), &FeVM::cb_plugin_command);
			fe.Func<bool (*)(const char *)>(_SC("load_module"), &FeVM::load_module);
			fe.Func<void (*)(const char *)>(_SC("do_nut"), &FeVM::do_nut);
			fe.Func<const char* (*)(const char *)>(_SC("path_expand"), &FeVM::cb_path_expand);
			fe.Func<bool (*)(const char *, int)>(_SC("path_test"), &FeVM::cb_path_test);
			fe.Overload<const char *(*)(const char *)>(_SC("get_text"), &FeVM::cb_get_text);
		}

		Sqrat::RootTable().Bind( _SC("fe"),  fe );

		std::string path = script_path;
		std::string file = script_file;
		fe.SetValue( _SC("script_dir"), path );
		fe.SetValue( _SC("script_file"), file );
		fe_vm->m_script_cfg = &configurable;

		if ( file.empty() )
		{
			// if there is no script file at this point,
			// we try loader script instead
			fe_vm->m_feSettings->get_path( FeSettings::Loader,
				path, file );

			fe.SetValue( _SC("loader_dir"), path );
		}

		run_script( path, file, true );
	};

	~FeConfigVM()
	{
		// reset to our usual VM and close the temp vm
		Sqrat::DefaultVM::Set( m_stored_vm );
		sq_close( m_vm );
	};

	HSQUIRRELVM &get_vm() { return m_vm; };
};

void FeVM::script_run_config_function(
		const FeScriptConfigurable &configurable,
		const std::string &script_path,
		const std::string &script_file,
		const std::string &func_name,
		std::string &return_message )
{
	FeConfigVM config_vm( configurable, script_path, script_file );
	sqstd_seterrorhandlers( config_vm.get_vm() );

	Sqrat::Function func( Sqrat::RootTable(), func_name.c_str() );

	if ( !func.IsNull() )
	{
		const char *help_msg = NULL;
		try
		{
			help_msg = func.Evaluate<const char *>( cb_get_config() );
		}
		catch( Sqrat::Exception &e )
		{
			return_message = "Script error";
			FeLog() << "Script Error in " << script_file
				<< " - " << e.Message() << std::endl;
		}

		if ( help_msg )
			return_message = help_msg;
	}
	else
	{
		return_message = "Script error: Function not found";
		FeLog() << "Script Error in " << script_file
			<< " - Function not found: " << func_name << std::endl;
	}

}

void FeVM::script_get_config_options(
		std::string &gen_help,
		const std::string &script_path,
		const std::string &script_file )
{
	if ( !script_path.empty() )
	{
		FeScriptConfigurable ignored;
		FeConfigVM config_vm( ignored, script_path, script_file, true );

		Sqrat::Object uConfig = Sqrat::RootTable().GetSlot( "UserConfig" );
		if ( !uConfig.IsNull() )
		{
			fe_get_attribute_string(
				config_vm.get_vm(),
				uConfig.GetObject(), "", "help", gen_help );
		}
	}
}

void FeVM::script_get_config_options(
		FeConfigContext &ctx,
		std::string &gen_help,
		FeScriptConfigurable &configurable,
		const std::string &script_path,
		const std::string &script_file )
{
	if ( !script_path.empty() )
	{
		FeConfigVM config_vm( configurable, script_path, script_file );

		Sqrat::Object uConfig = Sqrat::RootTable().GetSlot( "UserConfig" );
		if ( !uConfig.IsNull() )
		{
			fe_get_attribute_string(
				config_vm.get_vm(),
				uConfig.GetObject(), "", "help", gen_help );

			// Now Ccnstruct the UI elements for plug-in/layout specific configuration
			//
			std::multimap<int,FeMenuOpt> my_opts;

			Sqrat::Object::iterator it;
			while ( uConfig.Next( it ) )
			{
				std::string key;
				fe_get_object_string( config_vm.get_vm(), it.getKey(), key );

				std::string value, label, help, options, is_input, is_func;

				// use the default value from the script if a value has
				// not already been configured
				//
				if ( !configurable.get_param( key, value ) )
					fe_get_object_string( config_vm.get_vm(), uConfig.GetSlot( key.c_str() ), value );

				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "label", label);

				if ( label.empty() )
					label = key;

				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "help", help);

				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "options", options);

				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "is_input", is_input);

				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "is_function", is_func);

				std::string otmp;
				int order=-1;
				fe_get_attribute_string(
						config_vm.get_vm(),
						uConfig.GetObject(), key, "order", otmp);

				if ( !otmp.empty() )
					order = as_int( otmp );

				std::multimap<int,FeMenuOpt>::iterator it;
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

					it = my_opts.insert( std::pair <int, FeMenuOpt>(
						order,
						FeMenuOpt(Opt::LIST, label, value, help, 0, key ) ) );

					(*it).second.append_vlist( options_list );
				}
				else if ( config_str_to_bool( is_input ) )
				{
					it = my_opts.insert(
						std::pair <int, FeMenuOpt>(
							order,
							FeMenuOpt(Opt::RELOAD, label, value, help, 1, key ) ) );
				}
				else if ( config_str_to_bool( is_func ) )
				{
					FeMenuOpt temp_opt(Opt::SUBMENU, label, "", help, 2, key );
					temp_opt.opaque_str = value;

					it = my_opts.insert(
						std::pair <int, FeMenuOpt>(
							order,
							temp_opt ) );
				}
				else
				{
					it = my_opts.insert(
						std::pair <int, FeMenuOpt>(
							order,
							FeMenuOpt(Opt::EDIT, label, value, help, 0, key ) ) );
				}

				// Nice and hacky, we put an "%" at start of opaque_str if this is a "per display"
				// option.  fe_config picks this up and deals with it accordingly
				//
				std::string per_display_str;
				fe_get_attribute_string(
					config_vm.get_vm(),
					uConfig.GetObject(), key, "per_display", per_display_str );

				if ( config_str_to_bool( per_display_str ) )
				{
					std::string temp = (*it).second.opaque_str;
					(*it).second.opaque_str = "%";
					(*it).second.opaque_str += temp;
				}
			}

			for ( std::multimap<int,FeMenuOpt>::iterator itr = my_opts.begin(); itr != my_opts.end(); ++itr )
				ctx.opt_list.push_back( (*itr).second );
		}
	}
}

namespace
{
	struct generate_ui_info_struct
	{
		FeOverlay *ov;
		std::string emu;
	};

	bool generate_ui_update( void *d, int i, const std::string &aux )
	{
		generate_ui_info_struct *gi = (generate_ui_info_struct *)d;

		std::string msg = gi->emu;
		msg += " ";
		msg += as_str( i );

		gi->ov->splash_message( "Generating Rom List: $1%", msg, aux );
		return !gi->ov->check_for_cancel();
	}
};

bool FeVM::setup_wizard()
{
	m_overlay->splash_message( "Please Wait" );

	std::string path, fname;
	if ( !m_feSettings->get_emulator_setup_script( path, fname ) )
	{
		FeLog() << "Unable to get emulator setup script. path=" << path
			<< ", filaname=" << fname << std::endl;
		return false;
	}

	FeScriptConfigurable ignored;
	FeConfigVM config_vm( ignored, path, fname );

	Sqrat::Object etg = Sqrat::RootTable().GetSlot( "emulators_to_generate" );
	if ( etg.IsNull() )
	{
		FeDebug() << "Unable to get 'emulators_to_generate' from setup script: " << fname << std::endl;
		return false;
	}

	Sqrat::Array obj( etg );

	std::vector < std::string > emus_to_import;

	Sqrat::Object::iterator it;
	while ( etg.Next( it ) )
	{
		std::string value;
		fe_get_object_string( Sqrat::DefaultVM::Get(), it.getValue(), value );
		emus_to_import.push_back( value );
	}

	if ( emus_to_import.empty() )
		return false;

	// return 0 if user confirms import
	if ( m_overlay->confirm_dialog(
		"Attract-Mode detected emulator(s) that can be imported automatically.  Import them now?",
		"", true ) != 0 ) // default to "yes"
	{
		return false;
	}

	std::string read_base = m_feSettings->get_config_dir();
	read_base += FE_EMULATOR_TEMPLATES_SUBDIR;

	std::string write_base = m_feSettings->get_config_dir();
	write_base += FE_EMULATOR_SUBDIR;

	bool cancelled=false;
	for ( std::vector<std::string>::iterator itr = emus_to_import.begin();
		!cancelled && (itr != emus_to_import.end()); ++itr )
	{
		// Overwrite emulator config file with template which has just been generated
		//
		FeEmulatorInfo emu( *itr );
		std::string fname = (*itr);
		fname += FE_EMULATOR_FILE_EXTENSION;

		emu.load_from_file( read_base + fname );
		emu.save( write_base + fname );

		// Build romlist
		//
		std::vector<std::string> emu_list( 1, (*itr) );
		std::string ignored;

		struct generate_ui_info_struct gi;
		gi.ov = m_overlay;
		gi.emu = emu_list[0];

		//
		// Note: We purposefully disable scraping from the network at startup,
		//
		cancelled = !m_feSettings->build_romlist( emu_list,
				emu_list[0], generate_ui_update, &gi, ignored, false );

		// Create Display
		//
		if ( !m_feSettings->check_romlist_configured( emu_list[0] ) )
		{
			FeDisplayInfo *new_disp = m_feSettings->create_display( emu_list[0] );
			new_disp->set_info( FeDisplayInfo::Romlist, emu_list[ 0 ] );
		}
	}

	m_feSettings->save();
	m_feSettings->set_display( 0 );

	return true;
}

FeImage* FeVM::cb_add_image(const char *n, int x, int y, int w, int h )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeImage *ret = fev->add_image( false, n, x, y, w, h, fev->m_mon[0] );

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

	FeImage *ret = fev->add_image( true, n, x, y, w, h, fev->m_mon[0] );

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

	FeImage *ret = fev->add_clone( o, fev->m_mon[0] );

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

	FeText *ret = fev->add_text( n, x, y, w, h, fev->m_mon[0] );

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

	FeListBox *ret = fev->add_listbox( x, y, w, h, fev->m_mon[0] );

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

	FeImage *ret = fev->add_surface( w, h, fev->m_mon[0] );

	// Add the surface to the "fe.obj" array in Squirrel
	//
	Sqrat::Object fe ( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	Sqrat::Array obj( fe.GetSlot( _SC("obj") ) );
	obj.SetInstance( obj.GetSize(), ret );

	return ret;
}

FeSound* FeVM::cb_add_sound( const char *s, bool reuse )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return fev->add_sound( s, reuse );
	//
	// We assume the script will keep a reference to the sound
	//
}

FeSound* FeVM::cb_add_sound( const char *s )
{
	return cb_add_sound( s, true );
}

FeShader* FeVM::cb_add_shader( int type, const char *shader1, const char *shader2 )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	return fev->add_shader( (FeShader::Type)type, shader1, shader2 );
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

void FeVM::cb_add_ticks_callback( Sqrat::Object obj, const char *slot )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_ticks_callback( obj, slot );
}

void FeVM::cb_add_ticks_callback( const char *n )
{
	Sqrat::RootTable rt;
	cb_add_ticks_callback( rt, n );
}

void FeVM::cb_add_transition_callback( Sqrat::Object obj, const char *slot )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_transition_callback( obj, slot );
}

void FeVM::cb_add_transition_callback( const char *n )
{
	Sqrat::RootTable rt;
	cb_add_transition_callback( rt, n );
}

void FeVM::cb_add_signal_handler( Sqrat::Object obj, const char *slot )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->add_signal_handler( obj, slot );
}

void FeVM::cb_add_signal_handler( const char *n )
{
	Sqrat::RootTable rt;
	cb_add_signal_handler( rt, n );
}

void FeVM::cb_remove_signal_handler( Sqrat::Object obj, const char *slot )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	fev->remove_signal_handler( obj, slot );
}

void FeVM::cb_remove_signal_handler( const char *n )
{
	Sqrat::RootTable rt;
	cb_remove_signal_handler( rt, n );
}

bool FeVM::cb_get_input_state( const char *input )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

#if ( SFML_VERSION_INT >= FE_VERSION_INT( 2, 2, 0 ))
	//
	// Only test input when frontend has focus (this test only available as of SFML 2.2)
	//
	if ( !fev->m_window.hasFocus() )
		return false;
#endif

	//
	// First test if a command has been provided
	//
	for ( int i=0; i<FeInputMap::LAST_COMMAND; i++ )
	{
		if ( strcmp( input, FeInputMap::commandStrings[i] ) == 0 )
			return fev->m_feSettings->get_current_state( (FeInputMap::Command)i );
	}

	//
	// If not, then test based on it being an input string
	//
	return FeInputMapEntry( input ).get_current_state( fev->m_feSettings->get_joy_thresh() );
}

int FeVM::cb_get_input_pos( const char *input )
{
	return FeInputSingle( input ).get_current_pos();
}

// return false if file not found
bool FeVM::internal_do_nut( const std::string &work_dir,
			const std::string &script_file )
{
	std::string path;

	if ( is_relative_path( script_file ) )
		path = work_dir;

	return run_script( path, script_file );
}

void FeVM::do_nut( const char *script_file )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	std::string path;
	int script_id = fev->get_script_id();

	if ( script_id < 0 )
		fev->m_feSettings->get_path( FeSettings::Current, path );
	else
		fev->m_feSettings->get_plugin_full_path( script_id, path );

	if ( !internal_do_nut( path, script_file ) )
	{
		FeLog() << "Error, file not found: " << path
			<< " (" << script_file << ")" << std::endl;
	}
}

bool FeVM::load_module( const char *module )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	std::string module_dir;
	std::string module_file;
	fev->m_feSettings->get_module_path( module, module_dir, module_file );

	Sqrat::Table fe( Sqrat::RootTable().GetSlot( _SC("fe") ) );
	fe.SetValue( _SC("module_dir"), module_dir );
	fe.SetValue( _SC("module_file"), module_file );

	return internal_do_nut( module_dir, module_file );
}

bool FeVM::cb_plugin_command( const char *command,
		const char *args,
		Sqrat::Object obj,
		const char *fn )
{
	Sqrat::Function func( obj, fn );
	return run_program( clean_path( command ),
				args, "", my_callback, (void *)&func );
}

bool FeVM::cb_plugin_command( const char *command,
		const char *args,
		const char *fn )
{
	Sqrat::RootTable rt;
	return cb_plugin_command( command, args, rt, fn );
}

bool FeVM::cb_plugin_command( const char *command, const char *args )
{
	return run_program( clean_path( command ), args, "" );
}

bool FeVM::cb_plugin_command_bg( const char *command, const char *args )
{
	return run_program( clean_path( command ), args, "", NULL, NULL, false );
}

const char *FeVM::cb_path_expand( const char *path )
{
		static std::string internal_str;

		internal_str = absolute_path( clean_path( path ) );
		return internal_str.c_str();
}

bool FeVM::cb_path_test( const char *path, int flag )
{
	std::string p( path );

	switch ( flag )
	{
	case IsFileOrDirectory:
		return file_exists( p );

	case IsFile:
		return ( file_exists( p ) && !directory_exists( p ) );

	case IsDirectory:
		return directory_exists( p );

	case IsRelativePath:
		return is_relative_path( p );

	case IsSupportedArchive:
		return is_supported_archive( p );

	case IsSupportedMedia:
#ifndef NO_MOVIE
		return FeMedia::is_supported_media_file( p );
#else
		return ( tail_compare( p, FE_ART_EXTENSIONS ) );
#endif

	default:
		FeLog() << "Error, unrecognized path_test flag: " << flag << std::endl;
		return false;
	}
}

const char *FeVM::cb_game_info( int index, int offset, int filter_offset )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	if (( index > FeRomInfo::LAST_INDEX+3 ) || ( index < 0 ))
	{
		// TODO: the better thing to do would be to raise a squirrel error here
		//
		FeLog() << "game_info(): index out of range" << std::endl;
		return "";
	}

	static std::string retval;
	retval.clear();

	switch ( index )
	{
	case FeRomInfo::LAST_INDEX:
		{
			std::string emu_name = fev->m_feSettings->get_rom_info( filter_offset, offset, FeRomInfo::Emulator );
			FeEmulatorInfo *emu = fev->m_feSettings->get_emulator( emu_name );

			retval.clear();
			if ( emu )
				retval = emu->get_info( FeEmulatorInfo::System );

		}
		break;

	case FeRomInfo::LAST_INDEX+1: // Overview
		{
			int filter_index = fev->m_feSettings->get_filter_index_from_offset( filter_offset );

			fev->m_feSettings->get_game_overview_absolute( filter_index,
				fev->m_feSettings->get_rom_index( filter_index, offset ),
				retval );

		}
		break;

	case FeRomInfo::LAST_INDEX+2: // IsPaused
		if ( fev->m_window.has_running_process() && fev->m_feSettings->is_last_launch( filter_offset, offset ) )
			retval = "1";
		break;

	case FeRomInfo::LAST_INDEX+3: // SortValue
		{
			FeRomInfo::Index sort_by;
			bool reverse_sort;
			int list_limit;

			fev->m_feSettings->get_current_sort( sort_by, reverse_sort, list_limit );

			retval = fev->m_feSettings->get_rom_info( filter_offset, offset,
				( sort_by == FeRomInfo::LAST_INDEX ) ? FeRomInfo::Title : sort_by );
		}
		break;

	default:
		retval = fev->m_feSettings->get_rom_info( filter_offset, offset, (FeRomInfo::Index)index );
		break;
	}

	return retval.c_str();
}

const char *FeVM::cb_game_info( int index, int offset )
{
	return cb_game_info( index, offset, 0 );
}

const char *FeVM::cb_game_info( int index )
{
	return cb_game_info( index, 0, 0 );
}

const char *FeVM::cb_get_art( const char *art, int index_offset, int filter_offset, int art_flags )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );
	FeSettings *fes = fev->m_feSettings;
	int filter_index = fes->get_filter_index_from_offset( filter_offset );

	FeRomInfo *rom = fes->get_rom_absolute( filter_index,
			fes->get_rom_index( filter_index, index_offset ) );

	static std::string retval;
	retval.clear();

	if ( !rom )
		return retval.c_str();

	std::vector<std::string> vid_list, image_list;
	fes->get_best_artwork_file(
			*rom,
			art,
			vid_list,
			image_list,
			(art_flags&AF_ImagesOnly) );

	if ( art_flags&AF_FullList )
	{
		std::vector<std::string>::iterator itr;
		if ( !(art_flags&AF_ImagesOnly) &&  !vid_list.empty() )
		{
			for ( itr=vid_list.begin(); itr!=vid_list.end(); ++itr )
			{
				if ( !retval.empty() )
					retval += ";";

				// see note below re: need for absolute path
				retval += absolute_path( *itr );
			}
		}
		else
		{
			for ( itr=image_list.begin(); itr!=image_list.end(); ++itr )
			{
				if ( !retval.empty() )
					retval += ";";

				// see note below re: need for absolute path
				retval += absolute_path( *itr );
			}
		}
	}
	else
	{
		if ( !(art_flags&AF_ImagesOnly) &&  !vid_list.empty() )
			retval = vid_list.front();
		else if ( !image_list.empty() )
			retval = image_list.front();

		// We force our return value to an absolute path, to work
		// around Attract-Mode's tendency to assume that relative
		// paths are relative to the layout directory.
		//
		// We are almost certain that is not the case here...
		//
		retval = absolute_path( retval );
	}

	return retval.c_str();
}

const char *FeVM::cb_get_art( const char *art, int index_offset, int filter_index )
{
	return cb_get_art( art, index_offset, filter_index, AF_Default );
}

const char *FeVM::cb_get_art( const char *art, int index_offset )
{
	return cb_get_art( art, index_offset, 0, AF_Default );
}

const char *FeVM::cb_get_art( const char *art )
{
	return cb_get_art( art, 0, 0, AF_Default );
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

void FeVM::cb_signal( const char *sig )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );

	FeInputMap::Command c = FeInputMap::string_to_command( sig );
	if ( c != FeInputMap::LAST_COMMAND )
	{
		//
		// Post the command so it can be handled the next time we are
		// processing events...
		//
		fev->m_posted_commands.push( c );
		return;
	}

	//
	// Next check for special case signals
	//
	const char *signals[] =
	{
		"reset_window",
		"reload",
		NULL
	};

	int i=0;
	while ( signals[i] != 0 )
	{
		if ( strcmp( signals[i], sig ) == 0 )
			break;

		i++;
	}

	switch (i)
	{
	case 0: // "reset_window"
		fev->m_window.on_exit();
		fev->m_window.initial_create();
		fev->init_monitors();
		fev->m_posted_commands.push( FeInputMap::Reload );
		break;

	case 1: // "reload"
		fev->m_posted_commands.push( FeInputMap::Reload );
		break;

	default:
		FeLog() << "Error, unrecognized signal: " << sig << std::endl;
		break;

	}
}

void FeVM::cb_set_display( int idx, bool stack_previous )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );
	FeSettings *fes = fev->m_feSettings;

	if ( idx >= fes->displays_count() )
		idx = fes->displays_count()-1;
	if ( idx < 0 )
		idx = 0;

	fes->set_display( idx, stack_previous );
	fev->m_posted_commands.push( FeInputMap::Reload );
}

void FeVM::cb_set_display( int idx )
{
	cb_set_display( idx, false );
}

const char *FeVM::cb_get_text( const char *t )
{
	HSQUIRRELVM vm = Sqrat::DefaultVM::Get();
	FeVM *fev = (FeVM *)sq_getforeignptr( vm );
	FeSettings *fes = fev->m_feSettings;

	static std::string retval;
	retval.clear();

	if ( t )
		fes->get_resource( t, retval );

	return retval.c_str();
}

void FeVM::init_with_default_layout()
{
	//
	// Default to a full screen list with the
	// configured movie artwork as the background
	//
	FeImage *img = cb_add_artwork( "", 0, 0,
		m_layoutSize.x, m_layoutSize.y );

	img->setTrigger( EndNavigation );
	img->setColor( sf::Color( 100, 100, 100, 180 ) );

	FeListBox *lbs = cb_add_listbox( 2, 2, m_layoutSize.x, m_layoutSize.y );
	lbs->setColor( sf::Color::Black );
	lbs->setSelColor( sf::Color::Black );
	lbs->setSelBgColor( sf::Color::Transparent );

	FeListBox *lb = cb_add_listbox( 0, 0, m_layoutSize.x, m_layoutSize.y );
	lb->setSelBgColor( sf::Color( 0, 0, 255, 100 ) );
}
