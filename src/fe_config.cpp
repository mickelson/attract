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
#include "fe_util_sq.hpp"
#include <iostream>

#include <SFML/Graphics/Shader.hpp>
#ifndef NO_MOVIE
#include "media.hpp"
#endif

#include <sqrat.h>

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

FeBaseConfigMenu::FeBaseConfigMenu()
{
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
	{
		m_emulator->delete_artwork( label );
		m_emulator->add_artwork( label, ctx.opt_list[1].get_value() );
	}

	return true;
}

void FeEmuArtEditMenu::set_art( FeEmulatorInfo *emu,
					const std::string &art_name )
{
	m_emulator = emu;
	m_art_name = art_name;
}

FeEmulatorEditMenu::FeEmulatorEditMenu()
	: m_emulator( NULL ),
	m_is_new( false ),
	m_romlist_exists( false ),
	m_parent_save( false )
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

bool my_ui_update( void *d, int i )
{
	FeConfigContext *od = (FeConfigContext *)d;

	od->splash_message( "Generating Rom List: $1%", as_str( i ) );
	return !od->check_for_cancel();
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
			// Make sure m_emulator is set with all the configured info
			//
			for ( int i=0; i < FeEmulatorInfo::LAST_INDEX; i++ )
				m_emulator->set_info( (FeEmulatorInfo::Index)i,
					ctx.opt_list[i].get_value() );

			// Do some checks and confirmation before launching the Generator
			//
			std::string rom_path = clean_path(
				m_emulator->get_info( FeEmulatorInfo::Rom_path ) );

			if ( !file_exists( rom_path ) )
			{
				if ( ctx.confirm_dialog( "Rom path '$1' not found, proceed anyways?",
									rom_path ) == false )
					return false;
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

			ctx.fe_settings.get_resource( "Wrote $1 entries to romlist",
											as_str(list_size), ctx.help_msg );

			//
			// If we don't have a display list configured for this romlist,
			// configure one now
			//
			if ( !ctx.fe_settings.check_romlist_configured( emu_name ) )
			{
				FeListInfo *new_list = ctx.fe_settings.create_list( emu_name );
				new_list->set_info( FeListInfo::Romlist, emu_name );
			}

			ctx.save_req = true;
			m_parent_save = true;
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
		return m_parent_save;

	for ( int i=0; i < FeEmulatorInfo::LAST_INDEX; i++ )
		m_emulator->set_info( (FeEmulatorInfo::Index)i,
				ctx.opt_list[i].get_value() );

	std::string filename = ctx.fe_settings.get_config_dir();
	confirm_directory( filename, FE_EMULATOR_SUBDIR );

	filename += FE_EMULATOR_SUBDIR;
	filename += m_emulator->get_info( FeEmulatorInfo::Name );
	filename += FE_EMULATOR_FILE_EXTENSION;
	m_emulator->save( filename );

	return m_parent_save;
}

void FeEmulatorEditMenu::set_emulator(
				FeEmulatorInfo *emu, bool is_new, const std::string &romlist_dir )
{
	m_emulator=emu;
	m_is_new=is_new;
	m_parent_save=false;

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
			std::vector<std::string>(1, FE_EMULATOR_FILE_EXTENSION) );

	for ( std::vector<std::string>::iterator itr=emu_file_list.begin();
			itr < emu_file_list.end(); ++itr )
		ctx.add_opt( Opt::MENU, *itr, "", "_help_emu_sel" );

	ctx.add_optl( Opt::MENU, "Add Emulator", "", "_help_emu_add" );
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

FeRuleEditMenu::FeRuleEditMenu()
	: m_filter( NULL ), m_index( -1 )
{
}

void FeRuleEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Rule Edit" );

	FeRomInfo::Index target( FeRomInfo::LAST_INDEX );
	FeRule::FilterComp comp( FeRule::LAST_COMPARISON );
	std::string what, target_str, comp_str;

	if ( m_filter )
	{
		std::vector<FeRule> &r = m_filter->get_rules();
		if (( m_index >= 0 ) && ( m_index < (int)r.size() ))
		{
			target = r[m_index].get_target();
			comp = r[m_index].get_comp();
			what = r[m_index].get_what();
		}
	}

	if ( target != FeRomInfo::LAST_INDEX )
		ctx.fe_settings.get_resource( FeRomInfo::indexStrings[ target ], target_str );

	if ( comp != FeRule::LAST_COMPARISON )
		ctx.fe_settings.get_resource( FeRule::filterCompDisplayStrings[ comp ], comp_str );

	std::vector< std::string > targets;
	int i=0;
	while ( FeRomInfo::indexStrings[i] != 0 )
	{
		targets.push_back( std::string() );
		ctx.fe_settings.get_resource( FeRomInfo::indexStrings[i], targets.back() );
		i++;
	}

	ctx.add_optl( Opt::LIST, "Target", target_str, "_help_rule_target" );
	ctx.back_opt().append_vlist( targets );

	std::vector< std::string > comparisons;
	i=0;
	while ( FeRule::filterCompDisplayStrings[i] != 0 )
	{
		comparisons.push_back( std::string() );
		ctx.fe_settings.get_resource( FeRule::filterCompDisplayStrings[i], comparisons.back() );
		i++;
	}

	ctx.add_optl( Opt::LIST, "Comparison", comp_str, "_help_rule_comp" );
	ctx.back_opt().append_vlist( comparisons );

	ctx.add_optl( Opt::EDIT, "Filter Value", what, "_help_rule_value" );
	ctx.add_optl(Opt::EXIT,"Delete this Rule","","_help_rule_delete");
	ctx.back_opt().opaque = 1;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeRuleEditMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();
	if ( o.opaque == 1 ) // the "delete" option
	{
		std::vector<FeRule> &r = m_filter->get_rules();

		if (( m_index >= 0 ) && ( m_index < (int)r.size() ))
			r.erase( r.begin() + m_index );

		m_filter = NULL;
		m_index = -1;
		ctx.save_req = true;
	}

	return true;
}

bool FeRuleEditMenu::save( FeConfigContext &ctx )
{
	int i = ctx.opt_list[0].get_vindex();
	if ( i == -1 )
		i = FeRomInfo::LAST_INDEX;

	int c = ctx.opt_list[1].get_vindex();
	if ( c == -1 )
		c = FeRule::LAST_COMPARISON;

	std::string what = ctx.opt_list[2].get_value();

	if ( m_filter )
	{
		std::vector<FeRule> &r = m_filter->get_rules();

		if (( m_index >= 0 ) && ( m_index < (int)r.size() ))
		{
			r[m_index].set_values(
				(FeRomInfo::Index)i,
				(FeRule::FilterComp)c,
				what );
		}
	}

	return true;
}

void FeRuleEditMenu::set_rule_index( FeFilter *filter, int index )
{
	m_filter=filter;
	m_index=index;
}

FeFilterEditMenu::FeFilterEditMenu()
	: m_list( NULL ), m_index( 0 )
{
}

void FeFilterEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Filter Edit" );

	if ( m_list )
	{
		FeFilter *f = m_list->get_filter( m_index );

		ctx.add_optl( Opt::EDIT, "Filter Name", f->get_name(),
			"_help_filter_name" );

		int i=0;
		std::vector<FeRule> &rules = f->get_rules();

		for ( std::vector<FeRule>::const_iterator itr=rules.begin();
				itr != rules.end(); ++itr )
		{
			std::string rule_str;
			FeRomInfo::Index t = (*itr).get_target();

			if ( t != FeRomInfo::LAST_INDEX )
			{
				ctx.fe_settings.get_resource( FeRomInfo::indexStrings[t], rule_str );

				FeRule::FilterComp c = (*itr).get_comp();
				if ( c != FeRule::LAST_COMPARISON )
				{
					std::string comp_str;
					ctx.fe_settings.get_resource( FeRule::filterCompDisplayStrings[c], comp_str );

					rule_str += " ";
					rule_str += comp_str;
					rule_str += " ";
					rule_str += (*itr).get_what();
				}
			}
			ctx.add_optl( Opt::SUBMENU, "Rule", rule_str, "_help_filter_rule" );
			ctx.back_opt().opaque = 100 + i;
			i++;
		}

		ctx.add_optl(Opt::SUBMENU,"Add Rule","","_help_filter_add_rule");
		ctx.back_opt().opaque = 1;

		ctx.add_optl(Opt::EXIT,"Delete this Filter","","_help_filter_delete");
		ctx.back_opt().opaque = 2;

	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeFilterEditMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	if ( !m_list )
		return true;

	FeMenuOpt &o = ctx.curr_opt();
	if (( o.opaque >= 100 ) || ( o.opaque == 1 ))
	{
		FeFilter *f = m_list->get_filter( m_index );
		int r_index=0;

		if ( o.opaque == 1 )
		{
			std::vector<FeRule> &rules = f->get_rules();

			rules.push_back( FeRule() );
			r_index = rules.size() - 1;
			ctx.save_req=true;
		}
		else
			r_index = o.opaque - 100;

		m_rule_menu.set_rule_index( f, r_index );
		submenu=&m_rule_menu;
	}
	else if ( o.opaque == 2 )
	{
		FeFilter *f = m_list->get_filter( m_index );

		// "Delete this Filter"
		if ( ctx.confirm_dialog( "Delete filter '$1'?",
				f->get_name() ) == false )
			return false;

		m_list->delete_filter( m_index );
		m_list=NULL;
		m_index=-1;
		ctx.save_req=true;
	}

	return true;
}

bool FeFilterEditMenu::save( FeConfigContext &ctx )
{
	std::string name = ctx.opt_list[0].get_value();

	if ( m_list )
	{
		FeFilter *f = m_list->get_filter( m_index );
		f->set_name( name );
	}

	return true;
}

void FeFilterEditMenu::set_filter_index( FeListInfo *l, int i )
{
	m_list=l;
	m_index=i;
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

		std::vector<std::string> layouts;
		ctx.fe_settings.get_layouts_list( layouts );
		ctx.back_opt().append_vlist( layouts );

		ctx.add_optl( Opt::LIST, "Rom List",
				m_list->get_info( FeListInfo::Romlist ), "_help_list_romlist" );

		std::vector<std::string> romlists;
		ctx.fe_settings.get_romlists_list( romlists );
		ctx.back_opt().append_vlist( romlists );

		std::vector<std::string> filters;
		m_list->get_filters_list( filters );
		int i=0;

		for ( std::vector<std::string>::iterator itr=filters.begin();
				itr != filters.end(); ++itr )
		{
			ctx.add_optl( Opt::SUBMENU, "Filter",
				(*itr), "_help_list_filter" );
			ctx.back_opt().opaque = 100 + i;
			i++;
		}

		ctx.add_optl( Opt::SUBMENU, "Add Filter", "", "_help_list_add_filter" );
		ctx.back_opt().opaque = 1;

		ctx.add_optl( Opt::SUBMENU, "Layout Options", "", "_help_list_layout_options" );
		ctx.back_opt().opaque = 2;

		ctx.add_optl( Opt::EXIT, "Delete this List", "", "_help_list_delete" );
		ctx.back_opt().opaque = 3;
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeListEditMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( !m_list )
		return true;

	if (( o.opaque >= 100 ) || ( o.opaque == 1 ))
	{
		// a filter or "Add Filter" is selected
		int f_index=0;

		if ( o.opaque == 1 )
		{
			std::string res;
			ctx.edit_dialog( "Enter Filter Name", res );

			if ( res.empty() )
				return false;		// if they don't enter a name then cancel

			ctx.fe_settings.create_filter( *m_list, res );

			f_index = m_list->get_filter_count() - 1;
			ctx.save_req=true;
		}
		else
			f_index = o.opaque - 100;

		m_filter_menu.set_filter_index( m_list, f_index );
		submenu=&m_filter_menu;
	}
	else if ( o.opaque == 2 )
	{
		// Layout Options
		FeLayoutInfo &cfg = ctx.fe_settings.get_layout_config( ctx.opt_list[1].get_value() );
		m_layout_menu.set_layout( &cfg );
		submenu=&m_layout_menu;
	}
	else if ( o.opaque == 3 )
	{
		// "Delete this List"
		if ( ctx.confirm_dialog( "Delete list '$1'?", m_name ) == false )
			return false;

		ctx.fe_settings.delete_list( m_name );
		m_list=NULL;
		ctx.save_req=true;
	}

	return true;
}

bool FeListEditMenu::save( FeConfigContext &ctx )
{
	if ( m_list )
	{
		for ( int i=0; i< FeListInfo::LAST_INDEX; i++ )
			m_list->set_info( i, ctx.opt_list[i].get_value() );
	}

	return true;
}

void FeListEditMenu::set_list( FeListInfo *l )
{
	m_list=l;
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

	ctx.add_optl( Opt::MENU, "Add New List", "", "_help_list_add" );
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
	ctx.set_style( FeConfigContext::EditList, "Edit Control" );

	if (m_mapping)
	{
		std::string act;
		ctx.fe_settings.get_resource( FeInputMap::commandDispStrings[m_mapping->command], act );
		ctx.add_optl( Opt::INFO, "Action", act, "_help_input_action" );

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
				std::string command_str;
				ctx.fe_settings.get_resource( FeInputMap::commandDispStrings[ conflict ], command_str );
				save = ctx.confirm_dialog(
					"This will overwrite an existing mapping ($1).  Proceed?", command_str  );
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
		ctx.fe_settings.set_input_mapping( *m_mapping );
	}

	return true;
}

void FeInputEditMenu::set_mapping( FeMapping *mapping )
{
	m_mapping = mapping;
}

void FeInputSelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Configure / Controls" );
	ctx.fe_settings.get_input_mappings( m_mappings );

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
			"_help_control_sel" );
	}

	// Add the joystick and mouse threshold settings to this menu as well
	std::vector<std::string> thresh(21);
	thresh[0]="99";
	for ( int i=0; i<19; i++ )
		thresh[i+1] = as_str( 95 - ( i * 5 ) );
	thresh[20]="1";
	std::string setstr, help;

	ctx.add_optl( Opt::LIST, "Joystick Threshold",
		ctx.fe_settings.get_info( FeSettings::JoystickThreshold ), "_help_joystick_threshold" );
	ctx.back_opt().append_vlist( thresh );
	ctx.back_opt().opaque = 1;

	ctx.add_optl( Opt::LIST, "Mouse Threshold",
		ctx.fe_settings.get_info( FeSettings::MouseThreshold ), "_help_mouse_threshold" );
	ctx.back_opt().append_vlist( thresh );
	ctx.back_opt().opaque = 1;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeInputSelMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_info(
			FeSettings::JoystickThreshold,
			ctx.opt_list[ ctx.opt_list.size() - 3 ].get_value() );

	ctx.fe_settings.set_info(
			FeSettings::MouseThreshold,
			ctx.opt_list[ ctx.opt_list.size() - 2 ].get_value() );

	return true;
}

bool FeInputSelMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque == 0 )
	{
		// save now if needed so that the updated mouse and joystick
		// threshold values are used for any further mapping
		if ( ctx.save_req )
		{
			save( ctx );
			ctx.save_req = false;
		}

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
	ctx.fe_settings.get_sounds_list( sound_list );

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
				FeInputMap::commandDispStrings[i], v, "_help_sound_sel" );

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

	ctx.fe_settings.get_languages_list( m_languages );
	std::string cl = ctx.fe_settings.get_info( FeSettings::Language );

	std::vector<std::string> disp_lang_list( m_languages.size() );
	std::string disp_lang;

	int i=0;
	for ( std::vector<std::string>::iterator itr=m_languages.begin(); itr!=m_languages.end(); ++itr )
	{
		ctx.fe_settings.get_resource( (*itr), (disp_lang_list[i]) );

		if ( cl.compare(*itr) == 0 )
		{
			disp_lang = disp_lang_list[i];
		}

		i++;
	}

	ctx.add_optl( Opt::LIST,
			"Language",
			disp_lang,
			"_help_language" );
	ctx.back_opt().append_vlist( disp_lang_list );

	std::string autorot;
	ctx.fe_settings.get_resource( FeSettings::rotationDispTokens[ ctx.fe_settings.get_autorotate() ], autorot );
	std::vector < std::string > rotations;
	i=0;
	while ( FeSettings::rotationDispTokens[i] != 0 )
	{
		rotations.push_back( std::string() );
		ctx.fe_settings.get_resource( FeSettings::rotationDispTokens[ i ], rotations.back() );
		i++;
	}
	ctx.add_optl( Opt::LIST, "Auto Rotate", autorot, "_help_autorot" );
	ctx.back_opt().append_vlist( rotations );

	std::vector<std::string> bool_opts( 2 );
	ctx.fe_settings.get_resource( "Yes", bool_opts[0] );
	ctx.fe_settings.get_resource( "No", bool_opts[1] );

	ctx.add_optl( Opt::LIST,
			"Allow Exit from 'Lists Menu'",
			ctx.fe_settings.get_lists_menu_exit() ? bool_opts[0] : bool_opts[1],
			"_help_lists_menu_exit" );
	ctx.back_opt().append_vlist( bool_opts );

	ctx.add_optl( Opt::LIST,
			"Hide Brackets in Game Title",
			ctx.fe_settings.hide_brackets() ? bool_opts[0] : bool_opts[1],
			"_help_hide_brackets" );
	ctx.back_opt().append_vlist( bool_opts );

	ctx.add_optl( Opt::LIST,
			"Launch Last Game on Startup",
			ctx.fe_settings.autolaunch_last_game() ? bool_opts[0] : bool_opts[1],
			"_help_autolaunch_last_game" );
	ctx.back_opt().append_vlist( bool_opts );

	ctx.add_optl( Opt::LIST,
			"Confirm Favourites",
			ctx.fe_settings.confirm_favs() ? bool_opts[0] : bool_opts[1],
			"_help_confirm_favs" );
	ctx.back_opt().append_vlist( bool_opts );

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

	FeBaseConfigMenu::get_options( ctx );
}

bool FeMiscMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_language( m_languages[ ctx.opt_list[0].get_vindex() ] );

	ctx.fe_settings.set_info( FeSettings::AutoRotate,
			FeSettings::rotationTokens[ ctx.opt_list[1].get_vindex() ] );

	ctx.fe_settings.set_info( FeSettings::ListsMenuExit,
			ctx.opt_list[2].get_vindex() == 0 ? "yes" : "no" );

	ctx.fe_settings.set_info( FeSettings::HideBrackets,
			ctx.opt_list[3].get_vindex() == 0 ? "yes" : "no" );

	ctx.fe_settings.set_info( FeSettings::AutoLaunchLastGame,
			ctx.opt_list[4].get_vindex() == 0 ? "yes" : "no" );

	ctx.fe_settings.set_info( FeSettings::ConfirmFavourites,
			ctx.opt_list[5].get_vindex() == 0 ? "yes" : "no" );

	ctx.fe_settings.set_info( FeSettings::ExitCommand,
			ctx.opt_list[6].get_value() );

	ctx.fe_settings.set_info( FeSettings::DefaultFont,
			ctx.opt_list[7].get_value() );

	ctx.fe_settings.set_info( FeSettings::FontPath,
			ctx.opt_list[8].get_value() );

	return true;
}

const int FeScriptConfigMenu::OPAQUE_BASE=100;
const int FeScriptConfigMenu::INPUT_OPAQUE_BASE=200;

void FeScriptConfigMenu::get_options_helper(
		FeConfigContext &ctx,
		std::string &gen_help,
		FeScriptConfigurable &configurable,
		const std::string &script_file )
{
	m_params.clear();

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

				// Get the keys from the UserConfig class and put them in alphabetical order
				std::vector<std::string> key_list;
				Sqrat::Object::iterator it;
				while ( uConfig.Next( it ) )
				{
					std::string key;
					fe_get_object_string( temp_vm, it.getKey(), key );
					key_list.push_back( key );
				}

				std::sort( key_list.begin(), key_list.end() );

				// Now Ccnstruct the UI elements for plug-in/layout specific configuration
				//
				for ( std::vector < std::string >::iterator itr=key_list.begin(); itr!=key_list.end(); ++itr )
				{
					int opaque_base = OPAQUE_BASE;
					std::string value, label, help, options, is_input;

					// use the default value from the script if a value has
					// not already been configured
					//
					if ( !configurable.get_param( *itr, value ) )
						fe_get_object_string( temp_vm, uConfig.GetSlot( (*itr).c_str() ), value );

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), *itr, "label", label);

					if ( label.empty() )
						label = *itr;

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), *itr, "help", help);

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), *itr, "options", options);

					fe_get_attribute_string(
							temp_vm,
							uConfig.GetObject(), *itr, "is_input", is_input);

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

						ctx.add_optl( Opt::LIST, label, value, help );
						ctx.back_opt().append_vlist( options_list );
					}
					else if ( is_input.compare( "yes" ) == 0 )
					{
						opaque_base = INPUT_OPAQUE_BASE;
						ctx.add_optl( Opt::RELOAD, label, value, help );
					}
					else
					{
						ctx.add_optl( Opt::EDIT, label, value, help );
					}

					m_params.push_back( *itr );
					ctx.back_opt().opaque = opaque_base + m_params.size() - 1;
				}
			}
		}

		// reset to our usual VM and close the temp vm
		Sqrat::DefaultVM::Set( stored_vm );
		sq_close( temp_vm );
	}
}

bool FeScriptConfigMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque >= INPUT_OPAQUE_BASE )
	{
		std::string res;
		FeInputMap::Command conflict( FeInputMap::LAST_COMMAND );
		ctx.input_map_dialog( "Press Input", res, conflict );

		if (( conflict == FeInputMap::ExitMenu )
			|| ( conflict == FeInputMap::ExitNoMenu ))
		{
			// Clear the mapping if the user pushed an exit button
			res.clear();
		}

		o.set_value( res );
		ctx.save_req = true;
	}
	return true;
}

bool FeScriptConfigMenu::save_helper( FeConfigContext &ctx,
		FeScriptConfigurable &configurable )
{
	configurable.clear_params();

	for ( unsigned int i=0; i < ctx.opt_list.size(); i++ )
	{
		int op = ctx.opt_list[i].opaque;
		if ( op >= OPAQUE_BASE )
		{
			int decrement = ( op >= INPUT_OPAQUE_BASE ) ? INPUT_OPAQUE_BASE : OPAQUE_BASE;

			configurable.set_param(
				m_params[ op - decrement ],
				ctx.opt_list[i].get_value() );
		}
	}

	return true;
}

FePluginEditMenu::FePluginEditMenu()
	: m_plugin( NULL )
{
}

void FePluginEditMenu::get_options( FeConfigContext &ctx )
{
	if ( !m_plugin )
		return;

	ctx.set_style( FeConfigContext::EditList, "Configure Plug-in" );

	ctx.add_optl( Opt::INFO, "Name", m_plugin->get_name(),
		"_help_plugin_name" );

	ctx.add_optl( Opt::EDIT, "Command", m_plugin->get_command(),
		"_help_plugin_command" );

	std::vector<std::string> opts( 2 );
	ctx.fe_settings.get_resource( "Yes", opts[0] );
	ctx.fe_settings.get_resource( "No", opts[1] );

	ctx.add_optl( Opt::LIST, "Enabled",
			m_plugin->get_enabled() ? opts[0] : opts[1],
			"_help_plugin_enabled" );
	ctx.back_opt().append_vlist( opts );

	//
	// We run the plug-in script to check if a "UserConfig" class is defined.
	// If it is, then its members and member attributes set out what it is
	// that the plug-in needs configured by the user.
	//
	std::string script_file = ctx.fe_settings.get_plugin_full_path(
			m_plugin->get_name() );

	std::string gen_help;
	FeScriptConfigMenu::get_options_helper( ctx, gen_help, *m_plugin, script_file );

	if ( !gen_help.empty() )
		ctx.opt_list[0].help_msg = gen_help;

	FeBaseConfigMenu::get_options( ctx );
}

bool FePluginEditMenu::save( FeConfigContext &ctx )
{
	if ( m_plugin == NULL )
		return false;

	m_plugin->set_command(
		ctx.opt_list[1].get_value() );

	m_plugin->set_enabled(
		ctx.opt_list[2].get_vindex() == 0 ? true : false );

	return FeScriptConfigMenu::save_helper( ctx, *m_plugin );
}

void FePluginEditMenu::set_plugin( FePlugInfo *plugin )
{
	m_plugin = plugin;
}

void FePluginSelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure / Plug-ins" );

	std::vector<std::string> plugins;
	ctx.fe_settings.get_available_plugins( plugins );

	for ( std::vector<std::string>::iterator itr=plugins.begin();
			itr != plugins.end(); ++itr )
		ctx.add_opt( Opt::MENU, (*itr), "", "_help_plugin_sel" );

	FeBaseConfigMenu::get_options( ctx );
}

bool FePluginSelMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque == 0 )
	{
		m_edit_menu.set_plugin(
			ctx.fe_settings.get_plugin( o.setting ) );
		submenu = &m_edit_menu;
	}

	return true;
}

FeLayoutEditMenu::FeLayoutEditMenu()
	: m_layout( NULL )
{
}

void FeLayoutEditMenu::get_options( FeConfigContext &ctx )
{
	if ( m_layout )
	{
		const std::string &name = m_layout->get_name();
		ctx.set_style( FeConfigContext::EditList, "Configure Layout" );

		ctx.add_optl( Opt::INFO, "Name", name, "_help_layout_name" );

		std::string script_file;
		script_file = ctx.fe_settings.get_layout_dir( name ) + FE_LAYOUT_UI_KEY_FILE;

		std::string gen_help;
		FeScriptConfigMenu::get_options_helper( ctx, gen_help, *m_layout, script_file );

		if ( !gen_help.empty() )
			ctx.opt_list[0].help_msg = gen_help;
	}
	FeBaseConfigMenu::get_options( ctx );
}

bool FeLayoutEditMenu::save( FeConfigContext &ctx )
{
	if ( m_layout == NULL )
		return false;

	return FeScriptConfigMenu::save_helper( ctx, *m_layout );
}

void FeLayoutEditMenu::set_layout( FeLayoutInfo *layout )
{
	m_layout = layout;
}

void FeSaverEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Configure Screen Saver" );

	ctx.add_optl( Opt::EDIT,
			"Screen Saver Timeout",
			ctx.fe_settings.get_info( FeSettings::ScreenSaverTimeout ),
			"_help_screen_saver_timeout" );

	std::string gen_help;
	FeScriptConfigMenu::get_options_helper( ctx, gen_help,
					ctx.fe_settings.get_screensaver_config(),
					ctx.fe_settings.get_screensaver_file() );

	if ( !gen_help.empty() )
		ctx.opt_list[0].help_msg = gen_help;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeSaverEditMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_info( FeSettings::ScreenSaverTimeout,
			ctx.opt_list[0].get_value() );

	return FeScriptConfigMenu::save_helper( ctx, ctx.fe_settings.get_screensaver_config() );
}

void FeConfigMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure" );
	ctx.help_msg = FE_COPYRIGHT;

	ctx.add_optl( Opt::SUBMENU, "Emulators", "", "_help_emulators" );
	ctx.add_optl( Opt::SUBMENU, "Lists", "", "_help_lists" );
	ctx.add_optl( Opt::SUBMENU, "Controls", "", "_help_controls" );
	ctx.add_optl( Opt::SUBMENU, "Sound", "", "_help_sound" );
	ctx.add_optl( Opt::SUBMENU, "Screen Saver", "", "_help_screen_saver" );
	ctx.add_optl( Opt::SUBMENU, "Plug-ins", "", "_help_plugins" );
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
		submenu = &m_saver_menu;
		break;
	case 5:
		submenu = &m_plugin_menu;
		break;
	case 6:
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
