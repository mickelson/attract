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

#ifndef FE_IMAGE_HPP
#define FE_IMAGE_HPP

#include <SFML/Graphics.hpp>
#include "sprite.hpp"
#include "fe_presentable.hpp"

class FeSettings;
class FeMedia;
class FeImage;
class FeText;
class FeListBox;

enum FeVideoFlags
{
	VF_Normal			= 0,
	VF_DisableVideo			= 0x01,
	VF_NoLoop			= 0x02,
	VF_NoAutoStart			= 0x04,
	VF_NoAudio			= 0x08
};

class FeBaseTextureContainer
{
public:
	virtual ~FeBaseTextureContainer();

	virtual const sf::Texture &get_texture()=0;

	virtual void on_new_selection( FeSettings *feSettings );
	virtual void on_new_list( FeSettings *, float, float );

	virtual bool tick( FeSettings *feSettings, bool play_movies )=0; // returns true if redraw required
	virtual void set_play_state( bool play );
	virtual bool get_play_state() const;
	virtual void set_vol( float vol );

	virtual void set_index_offset( int io );
	virtual int get_index_offset() const;

	virtual void set_video_flags( FeVideoFlags );
	virtual FeVideoFlags get_video_flags() const;

	virtual void set_file_name( const char *n );
	virtual const char *get_file_name() const;
	//
	// Callback functions for use with surface objects
	//
	virtual FeImage *add_image(const char *,int, int, int, int);
	virtual FeImage *add_artwork(const char *,int, int, int, int);
	virtual FeImage *add_clone(FeImage *);
	virtual FeText *add_text(const char *,int, int, int, int);
	virtual FeListBox *add_listbox(int, int, int, int);
	virtual FeImage *add_surface(int, int);

	void register_image( FeImage * );

protected:
	FeBaseTextureContainer();
	FeBaseTextureContainer( const FeBaseTextureContainer & );
	FeBaseTextureContainer &operator=( const FeBaseTextureContainer & );

	// call this to notify registered images that the texture has changed
	void notify_texture_change();

private:
	std::vector< FeImage * > m_images;
};

class FeTextureContainer : public FeBaseTextureContainer
{
public:
	FeTextureContainer( bool is_artwork, const std::string &art_name="" );
	~FeTextureContainer();

	const sf::Texture &get_texture();

	void on_new_selection( FeSettings *feSettings );
	bool tick( FeSettings *feSettings, bool play_movies ); // returns true if redraw required
	void set_play_state( bool play );
	bool get_play_state() const;
	void set_vol( float vol );

	void set_index_offset( int io );
	int get_index_offset() const;

	void set_video_flags( FeVideoFlags );
	FeVideoFlags get_video_flags() const;

	void set_file_name( const char *n );
	const char *get_file_name() const;

	void load_now( const std::string &filename );

private:

	bool load( const std::vector <std::string> &art_paths,
		const std::string &target_name );
	bool load( const std::string &file_name );

	bool common_load(
		std::vector<std::string> &non_image_names,
		std::vector<std::string> &image_names );

	sf::Texture m_texture;
	std::string m_art_name; // artwork label for artworks
	std::string m_file_name; // the name of the loaded file
	int m_index_offset;
	bool m_is_artwork;
	FeMedia *m_movie;
	int m_movie_status; // 0=no play, 1=ready to play, >=PLAY_COUNT=playing
	FeVideoFlags m_video_flags;
};

class FeSurfaceTextureContainer : public FeBaseTextureContainer
{
public:

	FeSurfaceTextureContainer( int width, int height );
	~FeSurfaceTextureContainer();

	const sf::Texture &get_texture();

	void on_new_selection( FeSettings *feSettings );
	void on_new_list( FeSettings *, float, float );

	bool tick( FeSettings *feSettings, bool play_movies ); // returns true if redraw required

	//
	// Callback functions for use with surface objects
	//
	FeImage *add_image(const char *,int, int, int, int);
	FeImage *add_artwork(const char *,int, int, int, int);
	FeImage *add_clone(FeImage *);
	FeText *add_text(const char *,int, int, int, int);
	FeListBox *add_listbox(int, int, int, int);
	FeImage *add_surface(int, int);

private:
	sf::RenderTexture m_texture;
	std::vector <FeBasePresentable *> m_draw_list;
};

class FeImage : public sf::Drawable, public FeBasePresentable
{
protected:
	FeBaseTextureContainer *m_tex;
	FeSprite m_sprite;
	sf::Vector2f m_pos;
	sf::Vector2f m_size;
	bool m_preserve_aspect_ratio;

	void scale();

	// Override from base class:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
	FeImage( FeBaseTextureContainer *, float x, float y, float w, float h );
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
	const sf::Vector2u getTextureSize() const;
	const sf::IntRect &getTextureRect() const;
	void setTextureRect( const sf::IntRect &);
	int getVideoFlags() const;
	void setVideoFlags( int f );
	bool getVideoPlaying() const;
	void setVideoPlaying( bool );
	const char *getFileName() const;
	void setFileName( const char * );

	// deprecated as of 1.3, use video_flags instead:
	bool getMovieEnabled() const;
	void setMovieEnabled( bool );

	// Overrides from base class:
	//
	const sf::Drawable &drawable() const { return (const sf::Drawable &)*this; };
	void texture_changed();

	int get_skew_x() const ;
	int get_skew_y() const;
	int get_pinch_x() const ;
	int get_pinch_y() const;
	int get_texture_width() const;
	int get_texture_height() const;
	int get_subimg_x() const;
	int get_subimg_y() const;
	int get_subimg_width() const;
	int get_subimg_height() const;
	bool get_preserve_aspect_ratio() const;

	void set_skew_x( int x );
	void set_skew_y( int y );
	void set_pinch_x( int x );
	void set_pinch_y( int y );
	void set_subimg_x( int x );
	void set_subimg_y( int y );
	void set_subimg_width( int w );
	void set_subimg_height( int h );
	void set_preserve_aspect_ratio( bool p );

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

void script_do_update( FeBaseTextureContainer * );

#endif
