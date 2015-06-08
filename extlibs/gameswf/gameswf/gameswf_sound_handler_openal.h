// gameswf_sound_handler_openal.h	-- Vitaly Alexeev <tishka92@yahoo.com> 2009

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// OpenAL based sound handler for mobile units

#ifndef SOUND_HANDLER_OPENAL_H
#define SOUND_HANDLER_OPENAL_H

#include "base/tu_config.h"

#if TU_USE_OPENAL == 1

#include "gameswf/gameswf.h"
#include "base/container.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_mutex.h"
#include "gameswf/gameswf_listener.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

namespace gameswf
{

	struct sound;

	struct openal_sound_handler : public sound_handler
	{
		ALCdevice* m_device;
		ALCcontext* m_context;

		tu_mutex m_mutex;
		hash<int, gc_ptr<sound> > m_sound;
		float m_max_volume;

		openal_sound_handler();
		virtual ~openal_sound_handler();

		virtual bool is_open() { return true; };

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

		// Converts input data to the output format.
		virtual void cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, int size, 
			int channels, int freq);

		virtual void pause(int sound_handle, bool paused);
		virtual int get_position(int sound_handle);
		virtual	void advance(float delta_time);
	};


	// Used to hold the sounddata
	struct sound: public gameswf::ref_counted
	{
		sound(int size, Uint8* data, sound_handler::format_type format, int sample_count, 
			int sample_rate, bool stereo);
		~sound();

		void stop();

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

		void append(void* data, int size, openal_sound_handler* handler);
		void play(int loops, openal_sound_handler* handler);
		void pause(bool paused);
		int  get_played_bytes();
		void advance(float delta_time);

//	private:

		float m_volume;
		gameswf::sound_handler::format_type m_format;
		int m_sample_count;
		int m_sample_rate;
		bool m_stereo;
		bool m_is_paused;
		gc_ptr<listener> m_listeners;	// onSoundComplete event listeners
		int m_loops;
		int m_size;
		ALuint		m_uiSource;
		ALuint		m_uiBuffer;
		ALuint*		m_uiBuffers;
	};

}

#endif

#endif //TU_USE_SDL
