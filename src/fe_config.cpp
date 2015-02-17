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
#include "fe_vm.hpp"

#include <SFML/Graphics/Shader.hpp>
#ifndef NO_MOVIE
#include "media.hpp"
#endif

#include <iostream>

FeMenuOpt::FeMenuOpt( int t, const std::string &set, const std::string &val )
	: m_list_index( -1 ),
	type( t ),
	setting( set ),
	opaque( 0 )
{
	if ( !val.empty() )
		set_value( val );
}

FeMenuOpt::FeMenuOpt(int t,
		const std::string &set,
		const std::string &val,
		const std::string &help,
		int opq,
		const std::string &opq_str )
	: m_list_index( -1 ),
	type( t ),
	setting( set ),
	help_msg( help ),
	opaque( opq ),
	opaque_str( opq_str )
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

			if ( i == FeEmulatorInfo::Info_source )
			{
				const char *source_options[] =
				{
					"",
					"mame",
					"mess",
					"thegamesdb.net",
					"steam",
					NULL
				};

				ctx.add_optl( Opt::LIST,
						FeEmulatorInfo::indexDispStrings[i],
						m_emulator->get_info( (FeEmulatorInfo::Index)i ),
						help );

				ctx.back_opt().append_vlist( source_options );
			}
			else if ( i == FeEmulatorInfo::Exit_hotkey )
			{
				ctx.add_optl( Opt::RELOAD,
						FeEmulatorInfo::indexDispStrings[i],
						m_emulator->get_info( (FeEmulatorInfo::Index)i ),
						help );
				ctx.back_opt().opaque = 100;
			}
			else
			{
				ctx.add_optl( Opt::EDIT,
						FeEmulatorInfo::indexDispStrings[i],
						m_emulator->get_info( (FeEmulatorInfo::Index)i ),
						help );
			}
		}

		std::vector<std::pair<std::string, std::string> > alist;

		m_emulator->get_artwork_list( alist );

		std::vector<std::pair<std::string,std::string> >::iterator itr;
		for ( itr=alist.begin(); itr!=alist.end(); ++itr )
		{
			ctx.add_opt( Opt::SUBMENU, (*itr).first, (*itr).second, "_help_art" );
			ctx.back_opt().opaque = 1;
		}

		ctx.add_optl( Opt::SUBMENU, "Add Artwork", "", "_help_art_add" );
		ctx.back_opt().opaque = 2;

		ctx.add_optl( Opt::SUBMENU, "Generate Collection/Rom List", "",
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

namespace
{
	bool my_ui_update( void *d, int i )
	{
		FeConfigContext *od = (FeConfigContext *)d;

		od->splash_message( "Generating Rom List: $1%", as_str( i ) );
		return !od->check_for_cancel();
	}
};

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
			std::vector<std::string> paths = m_emulator->get_paths();

			for ( std::vector<std::string>::const_iterator itr = paths.begin();
					itr != paths.end(); ++itr )
			{
				std::string rom_path = clean_path( *itr );
				if ( !directory_exists( rom_path ) )
				{
					if ( ctx.confirm_dialog( "Rom path '$1' not found, proceed anyways?",
										rom_path ) == false )
						return false;
					else
						break; // only bug the user once if there are multiple paths configured
				}
			}

			std::string emu_name = m_emulator->get_info( FeEmulatorInfo::Name );

			if ( m_romlist_exists )
			{
				if ( ctx.confirm_dialog( "Overwrite existing '$1' list?",
									emu_name ) == false )
					return false;
			}

			int list_size( 0 );
			ctx.fe_settings.build_romlist( emu_name, my_ui_update, &ctx,
								list_size );

			ctx.fe_settings.get_resource( "Wrote $1 entries to Collection/Rom List",
											as_str(list_size), ctx.help_msg );

			//
			// If we don't have a display configured for this romlist,
			// configure one now
			//
			if ( !ctx.fe_settings.check_romlist_configured( emu_name ) )
			{
				FeDisplayInfo *new_disp = ctx.fe_settings.create_display( emu_name );
				new_disp->set_info( FeDisplayInfo::Romlist, emu_name );
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

	case 100: // Hotkey input
		{
			std::string res;
			FeInputMap::Command conflict( FeInputMap::LAST_COMMAND );
			ctx.input_map_dialog( "Press Exit Hotkey", res, conflict );

			bool save=false;
			if ( o.get_value().compare( res ) != 0 )
				save = true;
			else
			{
				if ( ctx.confirm_dialog( "Clear Exit Hotkey?", res ))
				{
					res.clear();
					save = true;
				}
			}

			if ( save )
			{
				o.set_value( res );
				ctx.save_req = true;
			}
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
			FE_EMULATOR_FILE_EXTENSION );

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
	: m_display( NULL ), m_index( 0 )
{
}

void FeFilterEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Filter Edit" );

	if ( m_display )
	{
		FeFilter *f = m_display->get_filter( m_index );

		if ( m_index < 0 )
		{
			std::string gf;
			ctx.fe_settings.get_resource( "Global Filter", gf );
			ctx.add_optl( Opt::INFO, "Filter Name", gf, "_help_filter_name" );
		}
		else
		{
			ctx.add_optl( Opt::EDIT, "Filter Name", f->get_name(),
				"_help_filter_name" );
		}

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

		if ( m_index >= 0 ) // don't add the folloiwng options for the global filter
		{
			std::string no_sort_str, sort_val;
			ctx.fe_settings.get_resource( "No Sort", no_sort_str );

			if ( f->get_sort_by() != FeRomInfo::LAST_INDEX )
				sort_val = FeRomInfo::indexStrings[f->get_sort_by()];
			else
				sort_val = no_sort_str;

			std::vector< std::string > targets;
			i=0;
			while ( FeRomInfo::indexStrings[i] != 0 )
			{
				targets.push_back( std::string() );
				ctx.fe_settings.get_resource( FeRomInfo::indexStrings[i], targets.back() );
				i++;
			}
			targets.push_back( no_sort_str );

			ctx.add_optl( Opt::LIST, "Sort By", sort_val, "_help_filter_sort_by" );
			ctx.back_opt().append_vlist( targets );

			ctx.add_optl( Opt::EDIT, "List Limit", as_str( f->get_list_limit() ),
				"_help_filter_list_limit" );

			ctx.add_optl(Opt::EXIT,"Delete this Filter","","_help_filter_delete");
			ctx.back_opt().opaque = 2;
		}
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeFilterEditMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	if ( !m_display )
		return true;

	FeMenuOpt &o = ctx.curr_opt();
	FeFilter *f = m_display->get_filter( m_index );

	if (( o.opaque >= 100 ) || ( o.opaque == 1 ))
	{
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
		// "Delete this Filter"
		if ( ctx.confirm_dialog( "Delete filter '$1'?",
				f->get_name() ) == false )
			return false;

		m_display->delete_filter( m_index );
		m_display=NULL;
		m_index=-1;
		ctx.save_req=true;
	}

	return true;
}

bool FeFilterEditMenu::save( FeConfigContext &ctx )
{
	if (( m_display ) && ( m_index >= 0 ))
	{
		FeFilter *f = m_display->get_filter( m_index );

		std::string name = ctx.opt_list[0].get_value();
		f->set_name( name );

		int sort_pos = ctx.opt_list.size() - 4;
		FeRomInfo::Index sort_by = (FeRomInfo::Index)ctx.opt_list[ sort_pos ].get_vindex();

		f->set_sort_by( sort_by );

		//
		// TODO - make reverse order configurable from the config menu
		//
		// right now we just arbitrarily sort players, playcount and playtime in "reverse" order so
		// higher values are first.
		//
		bool reverse_order( false );
		if (( sort_by == FeRomInfo::Players )
				|| ( sort_by == FeRomInfo::PlayedCount )
				|| ( sort_by == FeRomInfo::PlayedTime ))
			reverse_order = true;

		f->set_reverse_order( reverse_order );

		std::string limit_str = ctx.opt_list[ sort_pos + 1 ].get_value();
		int list_limit = as_int( limit_str );
		f->set_list_limit( list_limit );
	}

	return true;
}

void FeFilterEditMenu::set_filter_index( FeDisplayInfo *d, int i )
{
	m_display=d;
	m_index=i;
}

FeDisplayEditMenu::FeDisplayEditMenu()
	: m_display( NULL ),
	m_index( 0 )
{
}

void FeDisplayEditMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::EditList, "Display Edit" );

	if ( m_display )
	{
		ctx.add_optl( Opt::EDIT, "Name",
				m_display->get_info( FeDisplayInfo::Name ), "_help_display_name" );

		ctx.add_optl( Opt::LIST, "Layout",
				m_display->get_info( FeDisplayInfo::Layout ), "_help_display_layout" );

		std::vector<std::string> layouts;
		ctx.fe_settings.get_layouts_list( layouts );
		ctx.back_opt().append_vlist( layouts );

		ctx.add_optl( Opt::LIST, "Collection/Rom List",
				m_display->get_info( FeDisplayInfo::Romlist ), "_help_display_romlist" );

		std::vector<std::string> romlists;
		ctx.fe_settings.get_romlists_list( romlists );
		ctx.back_opt().append_vlist( romlists );

		FeFilter *f = m_display->get_filter( -1 );

		std::string filter_desc;
		if ( f->get_rule_count() < 1 )
			ctx.fe_settings.get_resource( "Empty", filter_desc );
		else
			ctx.fe_settings.get_resource( "$1 Rule(s)",
					as_str(f->get_rule_count()), filter_desc );

		ctx.add_optl( Opt::SUBMENU, "Global Filter", filter_desc, "_help_display_global_filter" );
		ctx.back_opt().opaque = 9;

		std::vector<std::string> filters;
		m_display->get_filters_list( filters );
		int i=0;

		for ( std::vector<std::string>::iterator itr=filters.begin();
				itr != filters.end(); ++itr )
		{
			ctx.add_optl( Opt::SUBMENU, "Filter",
				(*itr), "_help_display_filter" );
			ctx.back_opt().opaque = 100 + i;
			i++;
		}

		ctx.add_optl( Opt::SUBMENU, "Add Filter", "", "_help_display_add_filter" );
		ctx.back_opt().opaque = 1;

		ctx.add_optl( Opt::SUBMENU, "Layout Options", "", "_help_display_layout_options" );
		ctx.back_opt().opaque = 2;

		ctx.add_optl( Opt::EXIT, "Delete this Display", "", "_help_display_delete" );
		ctx.back_opt().opaque = 3;
	}

	FeBaseConfigMenu::get_options( ctx );
}

bool FeDisplayEditMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( !m_display )
		return true;

	if (( o.opaque >= 100 ) || ( o.opaque == 1 ) | ( o.opaque == 9 ))
	{
		// a filter or "Add Filter" is selected
		int f_index=0;

		if ( o.opaque == 1 )
		{
			std::string res;
			ctx.edit_dialog( "Enter Filter Name", res );

			if ( res.empty() )
				return false;		// if they don't enter a name then cancel

			ctx.fe_settings.create_filter( *m_display, res );

			f_index = m_display->get_filter_count() - 1;
			ctx.save_req=true;
		}
		else if ( o.opaque == 9 )
			f_index = -1; // this will get us the global filter
		else
			f_index = o.opaque - 100;

		m_filter_menu.set_filter_index( m_display, f_index );
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
		// "Delete this Display"
		if ( ctx.confirm_dialog( "Delete display '$1'?", m_display->get_info( FeDisplayInfo::Name ) ) == false )
			return false;

		ctx.fe_settings.delete_display( m_index );
		m_display=NULL;
		ctx.save_req=true;
	}

	return true;
}

bool FeDisplayEditMenu::save( FeConfigContext &ctx )
{
	if ( m_display )
	{
		for ( int i=0; i< FeDisplayInfo::LAST_INDEX; i++ )
			m_display->set_info( i, ctx.opt_list[i].get_value() );
	}

	return true;
}

void FeDisplayEditMenu::set_display( FeDisplayInfo *d, int index )
{
	m_display=d;
	m_index=index;
}

void FeDisplaySelMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure / Displays" );

	std::vector<std::string> name_list;
	ctx.fe_settings.get_display_names( name_list );

	int i=0;
	for ( std::vector<std::string>::iterator itr=name_list.begin();
			itr < name_list.end(); ++itr )
	{
		ctx.add_opt( Opt::MENU, (*itr), "", "_help_display_sel" );
		ctx.back_opt().opaque = i;
		i++;
	}

	ctx.add_optl( Opt::MENU, "Add New Display", "", "_help_display_add" );
	ctx.back_opt().opaque = 100000;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeDisplaySelMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque < 0 )
		return true;

	FeDisplayInfo *d( NULL );
	int index(0);

	if ( o.opaque == 100000 )
	{
		std::string res;
		ctx.edit_dialog( "Enter Display Name", res );

		if ( res.empty() )
			return false;		// if they don't enter a name then cancel

		ctx.save_req=true;
		d = ctx.fe_settings.create_display( res );
		index = ctx.fe_settings.displays_count() - 1;
	}
	else
	{
		d = ctx.fe_settings.get_display( o.opaque );
		index = o.opaque;
	}

	if ( d )
	{
		m_edit_menu.set_display( d, index );
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
			if (( m_mapping->input_list.size() <= 1 ) && (
					( m_mapping->command == FeInputMap::Select )
					|| ( m_mapping->command == FeInputMap::Up )
					|| ( m_mapping->command == FeInputMap::Down ) ) )
			{
				// We don't let the user unmap all "Up", "Down" or "Select" controls.
				// Doing so would prevent them from further navigating configure mode.
				break;
			}

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

		std::string help_msg( "_help_control_" );
		help_msg += FeInputMap::commandStrings[(*it).command];

		ctx.add_optl( Opt::SUBMENU,
			FeInputMap::commandDispStrings[(*it).command],
			value,
			help_msg );
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

	std::vector<std::string> bool_opts( 2 );
	ctx.fe_settings.get_resource( "Yes", bool_opts[0] );
	ctx.fe_settings.get_resource( "No", bool_opts[1] );

	ctx.add_optl( Opt::LIST,
			"Allow Exit from 'Displays Menu'",
			ctx.fe_settings.get_displays_menu_exit() ? bool_opts[0] : bool_opts[1],
			"_help_displays_menu_exit" );
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

	ctx.add_optl( Opt::LIST,
			"Track Usage",
			ctx.fe_settings.track_usage() ? bool_opts[0] : bool_opts[1],
			"_help_track_usage" );
	ctx.back_opt().append_vlist( bool_opts );

	std::string filterwrapmode;
	ctx.fe_settings.get_resource( FeSettings::filterWrapDispTokens[ ctx.fe_settings.get_filter_wrap_mode() ], filterwrapmode );
	std::vector < std::string > wrap_modes;
	i=0;
	while ( FeSettings::filterWrapDispTokens[i] != 0 )
	{
		wrap_modes.push_back( std::string() );
		ctx.fe_settings.get_resource( FeSettings::filterWrapDispTokens[ i ], wrap_modes.back() );
		i++;
	}
	ctx.add_optl( Opt::LIST, "Filter Wrap Mode", filterwrapmode, "_help_filter_wrap_mode" );
	ctx.back_opt().append_vlist( wrap_modes );

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

	std::string winmode;
	ctx.fe_settings.get_resource( FeSettings::windowModeDispTokens[ ctx.fe_settings.get_window_mode() ], winmode );
	std::vector < std::string > modes;
	i=0;
	while ( FeSettings::windowModeDispTokens[i] != 0 )
	{
		modes.push_back( std::string() );
		ctx.fe_settings.get_resource( FeSettings::windowModeDispTokens[ i ], modes.back() );
		i++;
	}
	ctx.add_optl( Opt::LIST, "Window Mode", winmode, "_help_window_mode" );
	ctx.back_opt().append_vlist( modes );

	FeBaseConfigMenu::get_options( ctx );
}

bool FeMiscMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_language( m_languages[ ctx.opt_list[0].get_vindex() ] );

	ctx.fe_settings.set_info( FeSettings::DisplaysMenuExit,
			ctx.opt_list[1].get_vindex() == 0 ? FE_CFG_YES_STR : FE_CFG_NO_STR );

	ctx.fe_settings.set_info( FeSettings::HideBrackets,
			ctx.opt_list[2].get_vindex() == 0 ? FE_CFG_YES_STR : FE_CFG_NO_STR );

	ctx.fe_settings.set_info( FeSettings::AutoLaunchLastGame,
			ctx.opt_list[3].get_vindex() == 0 ? FE_CFG_YES_STR : FE_CFG_NO_STR );

	ctx.fe_settings.set_info( FeSettings::ConfirmFavourites,
			ctx.opt_list[4].get_vindex() == 0 ? FE_CFG_YES_STR : FE_CFG_NO_STR );

	ctx.fe_settings.set_info( FeSettings::TrackUsage,
			ctx.opt_list[5].get_vindex() == 0 ? FE_CFG_YES_STR : FE_CFG_NO_STR );

	ctx.fe_settings.set_info( FeSettings::FilterWrapMode,
			FeSettings::filterWrapTokens[ ctx.opt_list[6].get_vindex() ] );

	ctx.fe_settings.set_info( FeSettings::ExitCommand,
			ctx.opt_list[7].get_value() );

	ctx.fe_settings.set_info( FeSettings::DefaultFont,
			ctx.opt_list[8].get_value() );

	ctx.fe_settings.set_info( FeSettings::FontPath,
			ctx.opt_list[9].get_value() );

	ctx.fe_settings.set_info( FeSettings::WindowMode,
			FeSettings::windowModeTokens[ ctx.opt_list[10].get_vindex() ] );

	return true;
}

bool FeScriptConfigMenu::on_option_select(
		FeConfigContext &ctx, FeBaseConfigMenu *& submenu )
{
	FeMenuOpt &o = ctx.curr_opt();

	if ( o.opaque == 1 )
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
	else if (( o.opaque == 2 ) && ( m_configurable ))
	{
		save( ctx );
		FeVM::script_run_config_function(
				*m_configurable,
				m_file_path,
				m_file_name,
				o.opaque_str,
				ctx.help_msg );
	}
	return true;
}

bool FeScriptConfigMenu::save_helper( FeConfigContext &ctx )
{
	m_configurable->clear_params();

	for ( unsigned int i=0; i < ctx.opt_list.size(); i++ )
	{
		m_configurable->set_param(
			ctx.opt_list[i].opaque_str,
			ctx.opt_list[i].get_value() );
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
	ctx.fe_settings.get_plugin_full_path( m_plugin->get_name(), m_file_path, m_file_name );
	m_configurable = m_plugin;

	std::string gen_help;
	FeVM::script_get_config_options( ctx, gen_help, *m_plugin, m_file_path, m_file_name );

	if ( !gen_help.empty() )
		ctx.opt_list[0].help_msg = gen_help;

	FeBaseConfigMenu::get_options( ctx );
}

bool FePluginEditMenu::save( FeConfigContext &ctx )
{
	if ( m_plugin == NULL )
		return false;

	m_plugin->set_enabled(
		ctx.opt_list[1].get_vindex() == 0 ? true : false );

	return FeScriptConfigMenu::save_helper( ctx );
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
		script_file = ctx.fe_settings.get_layout_dir( name )
				+ FE_LAYOUT_FILE_BASE + FE_LAYOUT_FILE_EXTENSION;

		std::string gen_help;
		m_file_path = ctx.fe_settings.get_layout_dir( name );

		std::vector< std::string > temp_list;
		FeSettings::get_layout_file_basenames_from_path(
					m_file_path, temp_list );

		if ( temp_list.empty() )
		{
			// set an empty m_file_name if this is a layout that gets loaded
			// by the loader script...
			m_file_name.clear();
		}
		else
		{
			m_file_name = FE_LAYOUT_FILE_BASE;
			m_file_name += FE_LAYOUT_FILE_EXTENSION;
		}

		m_configurable = m_layout;

		FeVM::script_get_config_options( ctx, gen_help, *m_layout,
				m_file_path, m_file_name );

		if ( !gen_help.empty() )
			ctx.opt_list[0].help_msg = gen_help;
	}
	FeBaseConfigMenu::get_options( ctx );
}

bool FeLayoutEditMenu::save( FeConfigContext &ctx )
{
	if ( m_layout == NULL )
		return false;

	return FeScriptConfigMenu::save_helper( ctx );
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
	ctx.fe_settings.get_screensaver_file( m_file_path, m_file_name );
	m_configurable = &(ctx.fe_settings.get_screensaver_config());

	FeVM::script_get_config_options( ctx, gen_help,
					ctx.fe_settings.get_screensaver_config(),
					m_file_path, m_file_name );

	if ( !gen_help.empty() )
		ctx.opt_list[0].help_msg = gen_help;

	FeBaseConfigMenu::get_options( ctx );
}

bool FeSaverEditMenu::save( FeConfigContext &ctx )
{
	ctx.fe_settings.set_info( FeSettings::ScreenSaverTimeout,
			ctx.opt_list[0].get_value() );

	return FeScriptConfigMenu::save_helper( ctx );
}

void FeConfigMenu::get_options( FeConfigContext &ctx )
{
	ctx.set_style( FeConfigContext::SelectionList, "Configure" );
	ctx.help_msg = FE_COPYRIGHT;

	ctx.add_optl( Opt::SUBMENU, "Emulators", "", "_help_emulators" );
	ctx.add_optl( Opt::SUBMENU, "Displays", "", "_help_displays" );
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
