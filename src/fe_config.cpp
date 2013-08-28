/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013 Andrew Mickelson
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

#include "fe_config.hpp"
#include "fe_info.hpp"
#include "fe_settings.hpp"
#include "fe_util.hpp"
#include <iostream>
#include "media.hpp"

FeMenuOpt::FeMenuOpt( int t, const std::string &set, const std::string &val )
	: m_list_index( -1 ), type( t ), opaque( 0 ), setting( set )
{
	if ( !val.empty() )
		set_value( val );
}

const std::string &FeMenuOpt::get_value() const
{
	if ( m_list_index == -1 )
		return m_edit_value;
	else
		return values_list[ m_list_index ];
}

int FeMenuOpt::get_vindex() const
{
	return m_list_index;
}

void FeMenuOpt::set_value( const std::string &s )
{
	if ( !values_list.empty() )
	{
		for ( unsigned int i=0; i < values_list.size(); i++ )
		{
			if ( values_list[i].compare( s ) == 0 )
			{
				set_value( i );
				return;
			}	
		}
	}

	ASSERT( m_list_index == -1 );
	m_edit_value = s;
}

void FeMenuOpt::set_value( int i )
{
	m_list_index = i;
	m_edit_value.clear(); //may be set for lists if no match previously 
}
   
void FeMenuOpt::append_vlist( const char **clist )
{
	int i=0;
	while ( clist[i] != NULL )
	{
		values_list.push_back( clist[i++] );

		if (( m_list_index == -1 ) 
					&& ( m_edit_value.compare( values_list.back() ) == 0 ))
			set_value( values_list.size() - 1 );
	}
}

void FeMenuOpt::append_vlist( const std::vector< std::string > &list )
{
	for ( std::vector<std::string>::const_iterator it=list.begin(); 
			it!=list.end(); ++it )
	{
		values_list.push_back( *it );

		if (( m_list_index == -1 ) && ( m_edit_value.compare( *it ) == 0 ))
			set_value( values_list.size() - 1 );
	}
}

FeConfigContext::FeConfigContext( FeSettings &f )
	: fe_settings( f ), curr_sel( -1 ), save_req( false )
{
}

void FeConfigContext::add_opt( int t, const std::string &s, 
		const std::string &v, const std::string &h )
{
	opt_list.push_back( FeMenuOpt( t, s, v ) );
	fe_settings.get_resource( h, opt_list.back().help_msg );
}

void FeConfigContext::add_optl( int t, const std::string &s, 
		const std::string &v, const std::string &h )
{
	std::string ss;
	fe_settings.get_resource( s, ss );
	add_opt( t, ss, v, h );
}

void FeConfigContext::set_style( Style s, const std::string &t )
{
	style = s;
	fe_settings.get_resource( t, title );
}

void FeBaseConfigMenu::get_options( FeConfigContext &ctx )
{
	ctx.add_optl( Opt::DEFAULTEXIT, "Back", "", "_help_back" );
	ctx.back_opt().opaque = -1;
}

bool FeBaseConfigMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	return true;
}

bool FeBaseConfigMenu::save( FeConfigContext &ctx )
{
	return true;
}

FeEmuArtEditMenu::FeEmuArtEditMenu()
	: m_emulator( NULL )
{
}

void FeEmuArtEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Edit Artwork" );

	std::string path;
	if ( m_emulator )
		m_emulator->get_artwork( m_art_name, path );

	ctx.add_optl( Opt::EDIT, "Artwork Label", m_art_name, "_help_art_label" );
	ctx.add_optl( Opt::EDIT, "Artwork Path", path, "_help_art_path" );
	FeBaseConfigMenu::get_options( ctx );
}

bool FeEmuArtEditMenu::save( FeConfigContext &ctx )
{
	if (!m_emulator)
		return false;

	if ( !m_art_name.empty() )
		m_emulator->delete_artwork( m_art_name );

	std::string label = ctx.opt_list[0].get_value();
	if ( !label.empty() )
		m_emulator->set_artwork( label, ctx.opt_list[1].get_value() );

	return true;
}

void FeEmuArtEditMenu::set_art( FeEmulatorInfo *emu, 
					const std::string &art_name )
{
	m_emulator = emu;
	m_art_name = art_name;
}

FeEmulatorEditMenu::FeEmulatorEditMenu()
	: m_emulator( NULL ), m_is_new( false ), m_romlist_exists( false )
{
}

void FeEmulatorEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Edit Emulator" );

	if ( m_emulator )
	{
		// Don't allow editting of the name. User can set it when adding new
		//
		ctx.add_optl( Opt::INFO, "Emulator Name", 
				m_emulator->get_info( FeEmulatorInfo::Name ) );

		for ( int i=1; i < FeEmulatorInfo::LAST_INDEX; i++ )
		{
			std::string help( "_help_emu_" );
			help += FeEmulatorInfo::indexStrings[i];

			ctx.add_optl( Opt::EDIT, 
					FeEmulatorInfo::indexDispStrings[i],
					m_emulator->get_info( (FeEmulatorInfo::Index)i ),
					help );
		}

		std::vector<std::pair<std::string, std::string> > alist;

		m_emulator->get_artwork_list( alist );

		std::vector<std::pair<std::string,std::string> >::iterator itr;
		for ( itr=alist.begin(); itr!=alist.end(); ++itr )
		{
			ctx.add_opt( Opt::SUBMENU, (*itr).first, (*itr).second );
			ctx.back_opt().opaque = 1;
		}

		ctx.add_optl( Opt::SUBMENU, "Add Artwork", "", "_help_art_add" );
		ctx.back_opt().opaque = 2;

		ctx.add_optl( Opt::SUBMENU, "Generate Romlist", "", 
								"_help_emu_gen_romlist" );
		ctx.back_opt().opaque = 3;

		ctx.add_optl( Opt::EXIT, "Delete this Emulator", "", "_help_emu_delete" );
		ctx.back_opt().opaque = 4;
	}

	// We need to call save if this is a new emulator
	//
	if ( m_is_new )
		ctx.save_req = true;

	FeBaseConfigMenu::get_options( ctx );
}

void my_ui_update( void *d, int i )
{
	((FeConfigContext *)d)->splash_message( "Generating Rom List: $1%", 
					as_str( i ) );
}

bool FeEmulatorEditMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( !m_emulator )
		return true;

	switch ( o.opaque )
	{
	case 1: //	 Edit artwork
		m_art_menu.set_art( m_emulator, o.setting );
		submenu = &m_art_menu;
		break;

	case 2: //	 Add new artwork
		m_art_menu.set_art( m_emulator, "" );
		submenu = &m_art_menu;
		break;
	
	case 3: // Generate Romlist
		{
			// Need to save now so that we have the appropriate emulator
			// settings to generate the rom list with
			//
			if ( ctx.save_req )
			{
				save( ctx );
				ctx.save_req = false;
			}

			std::string emu_name = m_emulator->get_info( FeEmulatorInfo::Name );

			if ( m_romlist_exists )
			{
				if ( ctx.confirm_dialog( "Overwrite existing '$1' romlist?", 
									emu_name ) == false )
					return false;
			}

			int list_size( 0 );
			ctx.fe_settings.build_romlist( emu_name, my_ui_update, &ctx, 
								list_size );

			ctx.fe_settings.get_resource( "Wrote $1 entries to romlist.", 
											as_str(list_size), ctx.help_msg );
		}
		break;

	case 4: // Delete this Emulator
		{
			std::string name = m_emulator->get_info(FeEmulatorInfo::Name);

			if ( ctx.confirm_dialog( "Delete emulator '$1'?", name ) == false )
				return false;

			ctx.fe_settings.delete_emulator( 
					m_emulator->get_info(FeEmulatorInfo::Name) );
		}
		break;

	default:
		break;
	}

	return true;
}

bool FeEmulatorEditMenu::save( FeConfigContext &ctx )
{
	if ( !m_emulator )
		return false;

	for ( int i=0; i < FeEmulatorInfo::LAST_INDEX; i++ )
		m_emulator->set_info( (FeEmulatorInfo::Index)i, 
				ctx.opt_list[i].get_value() );

	std::string filename = ctx.fe_settings.get_config_dir();
	confirm_directory( filename, FE_EMULATOR_SUBDIR );

	filename += FE_EMULATOR_SUBDIR;
	filename += m_emulator->get_info( FeEmulatorInfo::Name );
	filename += FE_EMULATOR_FILE_EXTENSION;
	m_emulator->save( filename );

	return false;
}

void FeEmulatorEditMenu::set_emulator( 
				FeEmulatorInfo *emu, bool is_new, const std::string &romlist_dir )
{
	m_emulator=emu;
	m_is_new=is_new;

	if ( emu )
	{
		std::string filename = m_emulator->get_info( FeEmulatorInfo::Name );
		filename += FE_ROMLIST_FILE_EXTENSION;

		m_romlist_exists = file_exists( romlist_dir + filename );
	}
	else
		m_romlist_exists = false;
}

void FeEmulatorSelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Config / Emulators" );

	std::string path = ctx.fe_settings.get_config_dir();
	path += FE_EMULATOR_SUBDIR;

	std::vector<std::string> emu_file_list;
	get_basename_from_extension( 
			emu_file_list,
			path,
			FE_EMULATOR_FILE_EXTENSION );

	for ( std::vector<std::string>::iterator itr=emu_file_list.begin();
			itr < emu_file_list.end(); ++itr )
		ctx.add_opt( Opt::MENU, *itr, "", "_help_emu_sel" );

	ctx.add_opt( Opt::MENU, "Add Emulator", "", "_help_emu_add" );
	ctx.back_opt().opaque = 1;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeEmulatorSelMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	FeEmulatorInfo *e( NULL );
	bool flag=false;

	if ( o.opaque < 0 )
		return true;

	if ( o.opaque == 1 )
	{
		std::string res;
		ctx.edit_dialog( "Enter Emulator Name", res );
		
		if ( res.empty() )
			return false;

		e = ctx.fe_settings.create_emulator( res );
		flag = true;
	}
	else
		e = ctx.fe_settings.get_emulator( o.setting );

	if ( e ) 
	{
		std::string romlist_dir = ctx.fe_settings.get_config_dir();
		romlist_dir += FE_ROMLIST_SUBDIR;
		m_edit_menu.set_emulator( e, flag, romlist_dir );
		submenu = &m_edit_menu;
	}

	return true; 
}

FeFilterEditMenu::FeFilterEditMenu()
	: m_list( NULL )
{
}

void FeFilterEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Filter Edit" );

	FeRomInfo::Index target( FeRomInfo::LAST_INDEX );
	FeListInfo::FilterComp comp( FeListInfo::LAST_COMPARISON );
	std::string what, target_str, comp_str;

	if ( m_list )
		m_list->get_filter( target, comp, what );
	
	if ( target != FeRomInfo::LAST_INDEX )
		target_str = FeRomInfo::indexStrings[ target ];

	if ( comp != FeListInfo::LAST_COMPARISON )
		comp_str = FeListInfo::filterCompStrings[ comp ];

	ctx.add_optl( Opt::LIST, "Filter Target", target_str, "_help_filter_target" );
	ctx.back_opt().append_vlist( FeRomInfo::indexStrings );

	ctx.add_optl( Opt::LIST, "Filter Comparison", comp_str, "_help_filter_comp" );
	ctx.back_opt().append_vlist( FeListInfo::filterCompStrings );

	ctx.add_optl( Opt::EDIT, "Filter Value", what, "_help_filter_value" );

	ctx.add_optl( Opt::RELOAD, "Clear Filter", "", "_help_filter_clear" );
	ctx.back_opt().opaque = 1;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeFilterEditMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();
	if ( o.opaque == 1 ) // the "clear" option
	{
		// clear and reload
		ctx.opt_list[0].set_value( -1 );
		ctx.opt_list[1].set_value( -1 );
		ctx.opt_list[2].set_value( "" );
		ctx.save_req = true;
	}

	return true;
}

bool FeFilterEditMenu::save( FeConfigContext &ctx )
{
	int i = ctx.opt_list[0].get_vindex();
	if ( i == -1 ) 
		i = FeRomInfo::LAST_INDEX;
		
	int c = ctx.opt_list[1].get_vindex();
	if ( c == -1 )
		c = FeListInfo::LAST_COMPARISON;

	std::string what = ctx.opt_list[2].get_value();

	if ( m_list )
		m_list->set_filter( (FeRomInfo::Index)i, 
							(FeListInfo::FilterComp)c, 
							what );
	return true;
}

void FeFilterEditMenu::set_list( FeListInfo *list )
{
   m_list=list;
}

FeListEditMenu::FeListEditMenu()
	: m_list( NULL )
{
}

void FeListEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "List Edit" );

	if ( m_list )
	{
		ctx.add_optl( Opt::EDIT, "List Name", 
				m_list->get_info( FeListInfo::Name ), "_help_list_name" );

		ctx.add_optl( Opt::LIST, "Layout", 
				m_list->get_info( FeListInfo::Layout ), "_help_list_layout" );

		std::string path = ctx.fe_settings.get_config_dir();
		path += FE_LAYOUT_SUBDIR;

		std::vector<std::string> values;
		get_basename_from_extension( values, path, "" );
		ctx.back_opt().append_vlist( values );

		ctx.add_optl( Opt::LIST, "Rom List", 
				m_list->get_info( FeListInfo::Romlist ), "_help_list_romlist" );

		path = ctx.fe_settings.get_config_dir();
		path += FE_ROMLIST_SUBDIR;

		std::vector<std::string> additions;
		get_basename_from_extension( additions, path, FE_ROMLIST_FILE_EXTENSION );
		ctx.back_opt().append_vlist( additions );

		FeRomInfo::Index i;
		FeListInfo::FilterComp c;
		std::string v, what;
		if ( m_list->get_filter( i, c, what ) )
			ctx.fe_settings.get_resource( "Yes", v );
		else
			ctx.fe_settings.get_resource( "No", v );
		
		ctx.add_optl( Opt::SUBMENU, "Filter", 
				v, "_help_list_filter" );
		ctx.back_opt().opaque = 1;

		ctx.add_optl( Opt::EXIT, "Delete this List", "", "_help_list_delete" );
		ctx.back_opt().opaque = 2;
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeListEditMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	switch ( o.opaque )
	{
	case 1: // Filter selected
		submenu=&m_filter_menu;
		break;

	case 2: // "Delete this List" 
		if ( ctx.confirm_dialog( "Delete list '$1'?", m_name ) == false )
			return false;

		ctx.fe_settings.delete_list( m_name );
		m_list=NULL;
		ctx.save_req=true;
		break;

	default:
		break;
	}

	return true;
}

bool FeListEditMenu::save( FeConfigContext &ctx )
{
	if ( m_list )
	{
		for ( int i=0; i< FeListInfo::Filter; i++ )
			m_list->set_info( i, ctx.opt_list[i].get_value() );
	}

	return true;
}

void FeListEditMenu::set_list( FeListInfo *l )
{
	m_list=l;
	m_filter_menu.set_list( l );
	m_name = l->get_info( FeListInfo::Name );
}

void FeListSelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure / Lists" );

	std::vector<std::string> name_list;
	ctx.fe_settings.get_list_names( name_list );

	for ( std::vector<std::string>::iterator itr=name_list.begin();
			itr < name_list.end(); ++itr )
		ctx.add_opt( Opt::MENU, (*itr), "", "_help_list_sel" );

	ctx.add_opt( Opt::MENU, "Add New List", "", "_help_list_add" );
	ctx.back_opt().opaque = 1;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeListSelMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque < 0 )
		return true;

	FeListInfo *l( NULL );

	if ( o.opaque == 1 )
	{
		std::string res;
		ctx.edit_dialog( "Enter List Name", res );

		if ( res.empty() )
			return false;		// if they don't enter a name then cancel
		
		ctx.save_req=true;
		l = ctx.fe_settings.create_list( res );
	}
	else
		l = ctx.fe_settings.get_list( o.setting );

	if ( l )
	{
		m_edit_menu.set_list( l );
		submenu = &m_edit_menu;
	}

	return true;
}

FeInputEditMenu::FeInputEditMenu()
	: m_mapping( NULL )
{
}

void FeInputEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Edit Input" );

	if (m_mapping)
	{
		ctx.add_optl( Opt::INFO, 
				"Action", 
				FeInputMap::commandDispStrings[m_mapping->command], 
				"_help_input_action" );

		std::vector< std::string >::iterator it;
		for ( it=m_mapping->input_list.begin();
				it<m_mapping->input_list.end(); ++it )
		{
			ctx.add_optl( Opt::RELOAD,"Remove Input",(*it),"_help_input_delete" );
		}

		ctx.add_optl( Opt::RELOAD, "Add Input", "", "_help_input_add" );
		ctx.back_opt().opaque = 1;
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeInputEditMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();
	if (!m_mapping)
		return true;

	switch ( o.opaque )
	{
	case 0:
		{
			std::vector< std::string >::iterator it;
			for ( it=m_mapping->input_list.begin(); 
					it<m_mapping->input_list.end(); ++it )
			{
				if ( (*it).compare( o.get_value() ) == 0 )
				{
					// User selected to remove this mapping
					//
					m_mapping->input_list.erase( it );
					ctx.save_req = true;
					break;
				}
			}
		}
		break;

	case 1:
		{
			std::string res;
			FeInputMap::Command conflict( FeInputMap::LAST_COMMAND );
			ctx.input_map_dialog( "Press Input", res, conflict );

			bool save=true;
			if (( conflict != FeInputMap::LAST_COMMAND )
				&& ( conflict != m_mapping->command ))
			{
				save = ctx.confirm_dialog( 
					"This will overwrite an existing mapping ($1).  Proceed?",
					FeInputMap::commandDispStrings[ conflict ]  );
			}

			if ( save )
			{
				m_mapping->input_list.push_back( res );
				ctx.save_req = true;
			}
		}
		break;

	default:
		break;
	}

	return true;
}

bool FeInputEditMenu::save( FeConfigContext &ctx )
{
	if ( m_mapping )
	{
		ctx.fe_settings.set_mapping( *m_mapping );
	}

	return true;
}

void FeInputEditMenu::set_mapping( FeMapping *mapping )
{
	m_mapping = mapping;
}

void FeInputSelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Configure / Input" );
	ctx.fe_settings.get_mappings( m_mappings );

	std::vector < FeMapping >::iterator it;
	for ( it=m_mappings.begin(); it != m_mappings.end(); ++it )
	{
		std::string setstr, help, orstr;
		ctx.fe_settings.get_resource( "OR", orstr );
		std::string value;
		std::vector < std::string >::iterator iti;
		for ( iti=(*it).input_list.begin(); iti != (*it).input_list.end(); ++iti )
		{
			if ( iti > (*it).input_list.begin() )
			{
				value += " ";
				value += orstr;
				value += " ";
			}

			value += (*iti);
		}

		ctx.add_optl( Opt::SUBMENU, 
			FeInputMap::commandDispStrings[(*it).command], 
			value, 
			"_help_input_sel" );
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeInputSelMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque == 0 )
	{
		m_edit_menu.set_mapping( &(m_mappings[ ctx.curr_sel ]) );
		submenu = &m_edit_menu;
	}

	return true;
}

void FeSoundMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Configure / Sound" );

	std::vector<std::string> volumes(11);
	for ( int i=0; i<11; i++ )
		volumes[i] = as_str( 100 - ( i * 10 ) );

	std::string setstr, help;

	//
	// Sound, Ambient and Movie Volumes
	//
	for ( int i=0; i<3; i++ )
	{
		int v = ctx.fe_settings.get_set_volume( (FeSoundInfo::SoundType)i );
		ctx.add_optl( Opt::LIST, FeSoundInfo::settingDispStrings[i], 
					as_str(v), "_help_volume" );
		ctx.back_opt().append_vlist( volumes );
	}

	//
	// Get the list of available sound files
	// Note the sound_list vector gets copied to each option!
	//
	std::vector<std::string> sound_list;
	std::string path = ctx.fe_settings.get_config_dir();
	path += FE_SOUND_SUBDIR;

	get_basename_from_extension(
		sound_list,
		path,
		"" );

#ifndef NO_MOVIE
	for ( std::vector<std::string>::iterator itr=sound_list.begin();
			itr != sound_list.end(); )
	{
		if ( !FeMedia::is_supported_media_file( *itr ) )
			itr = sound_list.erase( itr );
		else
			itr++;
	}
#endif

	sound_list.push_back( "" );

	//
	// Sounds that can be mapped to input commands.  
	//
	for ( int i=0; i < FeInputMap::LAST_EVENT; i++ )
	{
		// there is a NULL in the middle of the commandStrings list
		if ( FeInputMap::commandDispStrings[i] != NULL ) 
		{
			std::string v;
			ctx.fe_settings.get_sound_file( (FeInputMap::Command)i, v, false );

			ctx.add_optl( Opt::LIST,
				FeInputMap::commandDispStrings[i], v, "_help_sound" );

			ctx.back_opt().append_vlist( sound_list );
			ctx.back_opt().opaque = i;
		}
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeSoundMenu::save( FeConfigContext &ctx )
{
	//
	// Sound, Ambient and Movie Volumes
	//
	int i;
	for ( i=0; i<3; i++ )
		ctx.fe_settings.set_volume( (FeSoundInfo::SoundType)i, 
					ctx.opt_list[i].get_value() );

	//
	// Save the sound settings
	//
	for ( i=3; i< (int)ctx.opt_list.size(); i++ )
	{
		if ( ctx.opt_list[i].opaque >= 0 ) 
			ctx.fe_settings.set_sound_file( 
					(FeInputMap::Command)ctx.opt_list[i].opaque, 
					ctx.opt_list[i].get_value() );
	}

	return true;
}

void FeMiscMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Configure / Miscellaneous" );

	ctx.add_optl( Opt::LIST, 
			"Auto Rotate", 
			FeSettings::rotationDispTokens[ ctx.fe_settings.get_autorotate() ], 
			"_help_autorot" );
	ctx.back_opt().append_vlist( FeSettings::rotationDispTokens );

	ctx.add_optl( Opt::EDIT, 
			"Exit Command", 
			ctx.fe_settings.get_info( FeSettings::ExitCommand ), 
			"_help_exit_command" );

	ctx.add_optl( Opt::EDIT, 
			"Default Font", 
			ctx.fe_settings.get_info( FeSettings::DefaultFont ), 
			"_help_default_font" );

	ctx.add_optl( Opt::EDIT, 
			"Font Path", 
			ctx.fe_settings.get_info( FeSettings::FontPath ), 
			"_help_font_path" );

	ctx.add_optl( Opt::EDIT, 
			"Screen Saver Timeout", 
			ctx.fe_settings.get_info( FeSettings::ScreenSaverTimeout ), 
			"_help_ssaver_timeout" );

	FeBaseConfigMenu::get_options( ctx );
}

bool FeMiscMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_info( FeSettings::AutoRotate, 
			FeSettings::rotationTokens[ ctx.opt_list[0].get_vindex() ] );

	ctx.fe_settings.set_info( FeSettings::ExitCommand, 
			ctx.opt_list[1].get_value() );

	ctx.fe_settings.set_info( FeSettings::DefaultFont, 
			ctx.opt_list[2].get_value() );

	ctx.fe_settings.set_info( FeSettings::FontPath, 
			ctx.opt_list[3].get_value() );

	ctx.fe_settings.set_info( FeSettings::ScreenSaverTimeout, 
			ctx.opt_list[4].get_value() );

	return true;
}

void FeConfigMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure" );
	ctx.help_msg = FE_COPYRIGHT;

	ctx.add_optl( Opt::SUBMENU, "Emulators", "", "_help_emulators" );
	ctx.add_optl( Opt::SUBMENU, "Lists", "", "_help_lists" );
	ctx.add_optl( Opt::SUBMENU, "Input", "", "_help_input" );
	ctx.add_optl( Opt::SUBMENU, "Sound", "", "_help_sound" );
	ctx.add_optl( Opt::SUBMENU, "General", "", "_help_misc" );

	//
	// Force save if there is no config file
	//
	if ( ctx.fe_settings.config_file_exists() == false )
		ctx.save_req = true;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeConfigMenu::on_option_select( 
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	switch (ctx.curr_sel)
	{
	case 0:
		submenu = &m_emu_menu;
		break;
	case 1:
		submenu = &m_list_menu;
		break;
	case 2:
		submenu = &m_input_menu;
		break;
	case 3:
		submenu = &m_sound_menu;
		break;
	case 4:
		submenu = &m_misc_menu;
		break;
	default:
		break;
	}

	return true;
}

bool FeConfigMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.save();
	return true;
}
