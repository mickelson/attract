// gameswf_sound_handler_sdl.h	-- Vitaly Alexeev <tishka92@yahoo.com> 2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef SOUND_HANDLER_SDL_H
#define SOUND_HANDLER_SDL_H

#include "base/tu_config.h"

#ifdef TU_USE_SDL

#include <SDL_audio.h>
#include <SDL_thread.h>

#include "gameswf/gameswf.h"
#include "base/container.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_mutex.h"
#include "gameswf/gameswf_listener.h"

#if TU_CONFIG_LINK_TO_FFMPEG == 1
#include <ffmpeg/avformat.h>
#endif

// Used to hold the info about active sounds

namespace gameswf
{

	struct sound;
	struct active_sound;

	// Use SDL and ffmpeg to handle sounds.
	struct SDL_sound_handler : public sound_handler
	{

		// NetStream audio callbacks
		hash<as_object* /* netstream */, aux_streamer_ptr /* callback */> m_aux_streamer;
		hash<int, gc_ptr<sound> > m_sound;
		float m_max_volume;
		tu_mutex m_mutex;

#if TU_CONFIG_LINK_TO_FFMPEG == 1
		AVCodec* m_MP3_codec;
#endif

		// SDL_audio specs
		SDL_AudioSpec m_audioSpec;

		// Is sound device opened?
		bool m_is_open;

		SDL_sound_handler();
		virtual ~SDL_sound_handler();

		virtual bool is_open() { return m_is_open; };

		// loads external sound file, only .WAV for now
		virtual int	load_sound(const char* url);

		// Called to create a sample.
		virtual int	create_sound(void* data, int data_bytes,
			int sample_count, format_type format,
			int sample_rate, bool stereo);

		virtual void append_sound(int sound_handle, void* data, int data_bytes);

		// Play the index'd sample.
		virtual void	play_sound(as_object* listener_obj, int sound_handle, int loop_count);

		virtual void	set_max_volume(int vol);

		virtual void	stop_sound(int sound_handle);

		// this gets called when it's done with a sample.
		virtual void	delete_sound(int sound_handle);

		// this will stop all sounds playing.
		virtual void	stop_all_sounds();

		// returns the sound volume level as an integer from 0 to 100.
		virtual int	get_volume(int sound_handle);

		virtual void	set_volume(int sound_handle, int volume);

		virtual void	attach_aux_streamer(gameswf::sound_handler::aux_streamer_ptr ptr,
			as_object* netstream);
		virtual void	detach_aux_streamer(as_object* netstream);

		// Converts input data to the SDL output format.
		virtual void cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, int size, 
			int channels, int freq);

		virtual void pause(int sound_handle, bool paused);
		virtual int get_position(int sound_handle);
	};

	// Used to hold the sounddata
	struct sound: public gameswf::ref_counted
	{
		sound(int size, Uint8* data, sound_handler::format_type format, int sample_count, 
			int sample_rate, bool stereo);
		~sound();

		inline void clear_playlist()
		{
			m_playlist.clear();
		}

		// return value is in [0..100]
		inline int get_volume() const
		{
			return (int) (m_volume * 100.0f);
		}

		// vol is in [0..100]
		inline void set_volume(int vol)
		{
			m_volume = (float) vol / 100.0f;
		}

		void append(void* data, int size, SDL_sound_handler* handler);
		void play(int loops, SDL_sound_handler* handler);
		bool mix(Uint8* stream,	int len, array< gc_ptr<listener> >* listeners, float max_volume);
		void pause(bool paused);
		int  get_played_bytes();

//	private:

		Uint8* m_data;
		int m_size;
		float m_volume;
		gameswf::sound_handler::format_type m_format;
		int m_sample_count;
		int m_sample_rate;
		bool m_stereo;
		array<gc_ptr<active_sound> > m_playlist;
		bool m_is_paused;
		gc_ptr<listener> m_listeners;	// onSoundComplete event listeners
	};


	struct active_sound: public gameswf::ref_counted
	{
		active_sound(sound* parent, int loops):
		m_pos(0),
		m_played_bytes(0),
		m_loops(loops),
		m_size(0),
		m_data(NULL),
		m_parent(parent),
		m_decoded(0)
#if TU_CONFIG_LINK_TO_FFMPEG == 1
		,
		m_cc(NULL),
		m_parser(NULL)
#endif
	{
		m_handler = (SDL_sound_handler*) get_sound_handler();

		if (m_handler == NULL)
		{
			return;
		}
		
#if TU_CONFIG_LINK_TO_FFMPEG == 1
		if (m_parent->m_format == sound_handler::FORMAT_MP3)
		{
			// Init MP3 the parser
			m_parser = av_parser_init(CODEC_ID_MP3);
			m_cc = avcodec_alloc_context();
			if (m_cc == NULL)
			{
				log_error("Could not alloc MP3 codec context\n");
				return;
			}

			if (avcodec_open(m_cc, m_handler->m_MP3_codec) < 0)
			{
				log_error("Could not open MP3 codec\n");
				avcodec_close(m_cc);
				return;
			}
		}
#endif
	}

	~active_sound()
	{

		// memory was allocated in decode()
		free(m_data);

#if TU_CONFIG_LINK_TO_FFMPEG == 1
		if (m_parent->m_format == sound_handler::FORMAT_MP3)
		{
			if (m_cc)
			{
				avcodec_close(m_cc);
				av_free(m_cc);
			}

			if (m_parser)
			{
				av_parser_close(m_parser);
			}
		}
#endif
	}


	// returns the current sound position
	inline int get_played_bytes()
	{
		return m_played_bytes;
	}

	// returns true if the sound is played
	bool mix(Uint8* mixbuf , int mixbuf_len)
	{
		memset(mixbuf, 0, mixbuf_len);
		Uint8* mixbuf_ptr = mixbuf;

		// m_pos ==> current position of decoded sound which should be played
		assert(m_pos <= m_size);
		assert(m_decoded <= m_parent->m_size);

		bool is_playing = true;
		while (mixbuf_ptr - mixbuf < mixbuf_len)
		{
			int free_space = mixbuf_len - int(mixbuf_ptr - mixbuf);
			int n = imin(m_size - m_pos, free_space);
			memcpy(mixbuf_ptr, m_data + m_pos, n);
			mixbuf_ptr += n;
			m_pos += n;
			m_played_bytes += n;

			// there are decoded data
			if (m_pos < m_size)
			{
				continue;
			}

			free(m_data);
			m_data = NULL;
			m_pos = 0;
			m_size = 0;

			// no decoded data & free_space > 0 &
			// there are coded data
			if (m_decoded < m_parent->m_size)
			{
				switch (m_parent->m_format)
				{
					default:
						log_error("unknown sound format %d\n", m_parent->m_format);
						return false;

					case sound_handler::FORMAT_NATIVE16:
					{
						float	dup = float(m_handler->m_audioSpec.freq) / float(m_parent->m_sample_rate);
						int samples = int(float(mixbuf_len) / dup / (m_parent->m_stereo ? 1.0f : 2.0f));
						samples = imin(samples, m_parent->m_size - m_decoded);

						assert(samples > 0);

						int16* cvt_data;
						m_handler->cvt(&cvt_data, &m_size, m_parent->m_data + m_decoded,
							samples, m_parent->m_stereo ? 2 : 1, m_parent->m_sample_rate);

						m_data = (Uint8*) cvt_data;
						m_decoded += samples;
						break;
					}

					case sound_handler::FORMAT_MP3:
					{
#if TU_CONFIG_LINK_TO_FFMPEG == 1
						int asize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;

						int bufsize = 2 * asize;
						Uint8* buf = (Uint8*) malloc(bufsize);
						int buflen = 0;

						while (m_decoded < m_parent->m_size)
						{
							uint8_t* frame;
							int framesize;
							int decoded = av_parser_parse(m_parser, m_cc, &frame, &framesize,
								m_parent->m_data + m_decoded , m_parent->m_size - m_decoded, 0, 0);

							if (decoded < 0)
							{
								log_error("Error while decoding MP3-stream\n");
								free(buf);
								buf = NULL;
								buflen = 0;
								break;
							}

							m_decoded += decoded;

							if (framesize > 0)
							{
								int len = 0;
								if (avcodec_decode_audio(m_cc, (int16_t*) (buf + buflen), &len, frame, framesize) >= 0)
								{
									buflen += len;

									if (buflen > bufsize - asize)
									{
										bufsize += asize;
										buf = (Uint8*) realloc(buf, bufsize);
									}

									// Whether is it time to stop ?
									float	dup = float(m_handler->m_audioSpec.freq) / float(m_parent->m_sample_rate);
									int	output_sample_count = int(buflen * dup * (m_parent->m_stereo ? 1 : 2));
									if (output_sample_count >= mixbuf_len)
									{
										break;	// exit from while()
									}
								}
							}
						}

						assert(buflen > 0);

						int16* cvt_data;
						m_handler->cvt(&cvt_data, &m_size, buf, buflen,
							m_parent->m_stereo ? 2 : 1, m_parent->m_sample_rate);

						free(buf);	// decoded from MP3 data
						m_data = (Uint8*) cvt_data;
		#else
						log_error("MP3 requires FFMPEG library\n");
		#endif
						break;
					}
				}
			}
			else
			{
				m_decoded = 0;
				m_played_bytes = 0;

				// infinitive
				if (m_loops == -1)
				{
					break;
				}

				if (m_loops == 0)
				{
					is_playing = false;
					break;
				}
				else
				{
					m_loops--;
				}
			}
		}	// end while

		return is_playing;
	}

	private:

		int m_pos;
		int m_played_bytes;	// total played bytes, it's used in get_position()
		int m_loops;
		int m_size;
		Uint8* m_data;
		sound* m_parent;
		int m_decoded_size;

#if TU_CONFIG_LINK_TO_FFMPEG == 1
		AVCodecContext *m_cc;
		AVCodecParserContext* m_parser;
#endif

		int m_bufsize;

		int m_decoded;
		SDL_sound_handler* m_handler;
	};

}

#endif // SOUND_HANDLER_SDL_H

#endif // TU_USE_SDL

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
