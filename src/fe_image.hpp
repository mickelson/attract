/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013-15 Andrew Mickelson
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

#ifndef FE_IMAGE_HPP
#define FE_IMAGE_HPP

#include <SFML/Graphics.hpp>
#include "sprite.hpp"
#include "fe_presentable.hpp"
#include "fe_blend.hpp"

class FeSettings;
class FeMedia;
class FeSwf;
class FeImage;
class FeText;
class FeListBox;
class FeTextureContainer;
class FeImageLoaderEntry;

enum FeVideoFlags
{
	VF_Normal	= 0,
	VF_DisableVideo	= 0x01,
	VF_NoLoop	= 0x02,
	VF_NoAutoStart	= 0x04,
	VF_NoAudio	= 0x08
};

class FeBaseTextureContainer
{
public:
	virtual ~FeBaseTextureContainer();

	virtual const sf::Texture &get_texture()=0;

	virtual void on_new_selection( FeSettings *feSettings )=0;
	virtual void on_end_navigation( FeSettings *feSettings )=0;

	virtual void on_new_list( FeSettings *, bool new_display )=0;

	virtual bool get_visible() const;
	virtual bool tick( FeSettings *feSettings, bool play_movies ); // returns true if redraw required

	virtual void set_play_state( bool play );
	virtual bool get_play_state() const;
	virtual void set_vol( float vol );

	virtual void set_index_offset( int io, bool do_update=true );
	virtual int get_index_offset() const;
	virtual void set_filter_offset( int fo, bool do_update=true );
	virtual int get_filter_offset() const;

	virtual void set_video_flags( FeVideoFlags );
	virtual FeVideoFlags get_video_flags() const;
	virtual int get_video_duration() const;
	virtual int get_video_time() const;

	virtual void load_from_archive( const char *a, const char *n );
	virtual const char *get_file_name() const;
	virtual void set_trigger( int );
	virtual int get_trigger() const;

	virtual void transition_swap( FeBaseTextureContainer *o );
	virtual bool fix_masked_image();

	virtual void set_smooth( bool )=0;
	virtual bool get_smooth() const=0;

	virtual void set_mipmap( bool )=0;
	virtual bool get_mipmap() const=0;

	virtual void set_clear( bool );
	virtual bool get_clear() const;

	virtual void set_repeat( bool );
	virtual bool get_repeat() const;

	virtual bool is_swf() const;
	virtual float get_sample_aspect_ratio() const;

	// function for use with surface objects
	//
	virtual FePresentableParent *get_presentable_parent();

	void register_image( FeImage * );

	virtual void release_audio( bool );
	virtual void on_redraw_surfaces();

protected:
	FeBaseTextureContainer();
	FeBaseTextureContainer( const FeBaseTextureContainer & );
	FeBaseTextureContainer &operator=( const FeBaseTextureContainer & );

	//
	// Return a pointer to the FeTextureContainer if this is that type of container
	//
	virtual FeTextureContainer *get_derived_texture_container();

	// call this to notify registered images that the texture has changed
	void notify_texture_change();

private:
	std::vector< FeImage * > m_images;

	friend class FeTextureContainer;
};

class FeTextureContainer : public FeBaseTextureContainer
{
public:
	FeTextureContainer( bool is_artwork, const std::string &name="" );

	~FeTextureContainer();

	const sf::Texture &get_texture();
	bool get_visible() const;

	void on_new_selection( FeSettings *feSettings );
	void on_end_navigation( FeSettings *feSettings );
	void on_new_list( FeSettings *, bool );

	bool tick( FeSettings *feSettings, bool play_movies ); // returns true if redraw required
	void set_play_state( bool play );
	bool get_play_state() const;
	void set_vol( float vol );

	void set_index_offset( int io, bool do_update );
	int get_index_offset() const;
	void set_filter_offset( int fo, bool do_update );
	int get_filter_offset() const;

	void set_video_flags( FeVideoFlags );
	FeVideoFlags get_video_flags() const;
	int get_video_duration() const;
	int get_video_time() const;

	void load_from_archive( const char *a, const char *n );
	const char *get_file_name() const;
	void set_trigger( int );
	int get_trigger() const;

	void transition_swap( FeBaseTextureContainer *o );

	bool fix_masked_image();

	void set_smooth( bool );
	bool get_smooth() const;

	void release_audio( bool );

	void set_mipmap( bool );
	bool get_mipmap() const;

	void set_repeat( bool );
	bool get_repeat() const;

	bool is_swf() const;
	float get_sample_aspect_ratio() const;

protected:
	FeTextureContainer *get_derived_texture_container();

private:

#ifndef NO_MOVIE
	bool load_with_ffmpeg(
		const std::string &path,
		const std::string &filename,
		bool is_image );
#endif

	bool try_to_load(
		const std::string &path,
		const std::string &filename,
		bool is_image=false );

	void internal_update_selection( FeSettings *feSettings );
	void clear();

	sf::Texture m_texture;

	std::string m_art_name; // artwork label/template name (dynamic images)
	std::string m_file_name; // the name of the loaded file
	int m_index_offset;
	int m_filter_offset;
	int m_current_rom_index;
	int m_current_filter_index;

	enum Type { IsArtwork, IsDynamic, IsStatic };
	Type m_type;
	int m_art_update_trigger;
	FeMedia *m_movie;
	FeSwf *m_swf;
	int m_movie_status; // 0=no play, 1=ready to play, >=PLAY_COUNT=playing
	FeVideoFlags m_video_flags;
	bool m_mipmap;
	bool m_smooth;
	bool m_frame_displayed;
	FeImageLoaderEntry *m_entry;
};

class FeSurfaceTextureContainer : public FeBaseTextureContainer, public FePresentableParent
{
public:

	FeSurfaceTextureContainer( int width, int height );
	~FeSurfaceTextureContainer();

	const sf::Texture &get_texture();

	void on_new_selection( FeSettings *feSettings );
	void on_end_navigation( FeSettings *feSettings );
	void on_new_list( FeSettings *, bool );

	void on_redraw_surfaces();

	void set_smooth( bool );
	bool get_smooth() const;

	void set_mipmap( bool );
	bool get_mipmap() const;

	void set_clear( bool );
	bool get_clear() const;

	void set_repeat( bool );
	bool get_repeat() const;

	FePresentableParent *get_presentable_parent();

private:
	sf::RenderTexture m_texture;
	bool m_clear;
};

class FeImage : public sf::Drawable, public FeBasePresentable
{
protected:
	FeBaseTextureContainer *m_tex;
	FeSprite m_sprite;
	sf::Vector2f m_pos;
	sf::Vector2f m_size;
	sf::Vector2f m_origin;
	FeBlend::Mode m_blend_mode;
	bool m_preserve_aspect_ratio;

	void scale();

	// Override from base class:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeImage( FePresentableParent &p, FeBaseTextureContainer *,
		float x, float y, float w, float h );

	FeImage( FeImage * ); // clone the given image (texture is not copied)
	~FeImage();

	const sf::Texture *get_texture();

	const sf::Vector2f &getSize() const;
	void setSize( const sf::Vector2f &s );
	void setSize( int w, int h ) { setSize( sf::Vector2f( w, h ) ); };
	const sf::Vector2f &getPosition() const;
	void setPosition( const sf::Vector2f & );
	void setPosition( int x, int y ) { setPosition( sf::Vector2f( x, y ));};
	float getRotation() const;
	void setRotation( float );
	const sf::Color &getColor() const;
	void setColor( const sf::Color & );
	int getIndexOffset() const;
	void setIndexOffset(int);
	int getFilterOffset() const;
	void setFilterOffset(int);
	const sf::Vector2u getTextureSize() const;
	const sf::IntRect &getTextureRect() const;
	void setTextureRect( const sf::IntRect &);
	int getVideoFlags() const;
	void setVideoFlags( int f );
	bool getVideoPlaying() const;
	void setVideoPlaying( bool );
	int getVideoDuration() const;
	int getVideoTime() const;
	const char *getFileName() const;
	void setFileName( const char * );
	void loadFromArchive( const char *, const char * );
	int getTrigger() const;
	void setTrigger( int );

	// deprecated as of 1.3, use video_flags instead:
	bool getMovieEnabled() const;
	void setMovieEnabled( bool );

	// Overrides from base class:
	//
	const sf::Drawable &drawable() const { return (const sf::Drawable &)*this; };

	bool get_visible() const;

	void texture_changed( FeBaseTextureContainer *new_tex=NULL );

	float get_origin_x() const;
	float get_origin_y() const;
	int get_skew_x() const;
	int get_skew_y() const;
	int get_pinch_x() const ;
	int get_pinch_y() const;
	int get_texture_width() const;
	int get_texture_height() const;
	int get_subimg_x() const;
	int get_subimg_y() const;
	int get_subimg_width() const;
	int get_subimg_height() const;
	float get_sample_aspect_ratio() const;
	bool get_preserve_aspect_ratio() const;
	bool get_mipmap() const;
	bool get_smooth() const;
	int get_blend_mode() const;
	bool get_clear() const;
	bool get_repeat() const;

	void set_origin_x( float x );
	void set_origin_y( float y );
	void set_skew_x( int x );
	void set_skew_y( int y );
	void set_pinch_x( int x );
	void set_pinch_y( int y );
	void set_subimg_x( int x );
	void set_subimg_y( int y );
	void set_subimg_width( int w );
	void set_subimg_height( int h );
	void set_preserve_aspect_ratio( bool p );
	void set_mipmap( bool m );
	void set_smooth( bool );
	void set_clear( bool );
	void set_repeat( bool );
	void set_blend_mode( int b );

	void transition_swap( FeImage * );

	void rawset_index_offset( int io );
	void rawset_filter_offset( int fo );
	bool fix_masked_image();

	//
	// Callback functions for use with surface objects
	//
	FeImage *add_image(const char *,int, int, int, int);
	FeImage *add_image(const char *, int, int);
	FeImage *add_image(const char *);
	FeImage *add_artwork(const char *,int, int, int, int);
	FeImage *add_artwork(const char *, int, int);
	FeImage *add_artwork(const char *);
	FeImage *add_clone(FeImage *);
	FeText *add_text(const char *,int, int, int, int);
	FeListBox *add_listbox(int, int, int, int);
	FeImage *add_surface(int, int);
};

#endif
