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

#ifndef MEDIA_HPP
#define MEDIA_HPP

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <Audio/SoundStream.hpp>

class FeAudioImp;
class FeVideoImp;
class AVFormatContext;
class AVIOContext;

class FeMedia : private sf::SoundStream
{
friend class FeVideoImp;

public:
	enum Type
	{
		Audio=0x01,
		Video=0x02,
		AudioVideo=0x03
	};

	FeMedia( Type t );
	~FeMedia();

	bool openFromFile( const std::string &name,
			sf::Texture *out_texture=NULL );

	bool openFromArchive( const std::string &archive,
			const std::string &name,
			sf::Texture *out_texture=NULL );

	using sf::SoundStream::setPosition;
	using sf::SoundStream::getPosition;
	using sf::SoundStream::setPitch;
	using sf::SoundStream::getPitch;
	using sf::SoundStream::getStatus;

	void play();
	void stop();
	void close();

	// tick() needs to be called regularly on video media to update the display
	// texture. Returns true if display refresh required.  false if no update
	//
	bool tick();

	void setLoop( bool );
	bool getLoop() const;

	void setVolume(float volume);

	bool is_playing();
	bool is_multiframe() const;

	sf::Time get_video_time();
	sf::Time get_duration() const;

	const char *get_metadata( const char *tag );

	//
	// return true if the given filename is a media file that can be opened
	//	by FeMedia
	//
	static bool is_supported_media_file( const std::string &filename );

protected:
	// overrides from base class
	//
	bool onGetData( Chunk &data );
	void onSeek( sf::Time timeOffset );

	bool read_packet();
	bool end_of_file();

	bool internal_open( sf::Texture *outt );

private:
	Type m_type;
	AVFormatContext *m_format_ctx;
	AVIOContext *m_io_ctx;
	sf::Mutex m_read_mutex;
	bool m_loop;
	bool m_read_eof;

	FeAudioImp *m_audio;
	FeVideoImp *m_video;

	FeMedia( const FeMedia & );
	FeMedia &operator=( const FeMedia & );
	static void init_av();
};

#endif
