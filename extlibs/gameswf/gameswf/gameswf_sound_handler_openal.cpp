// gameswf_sound_handler_openal.cpp	-- Vitaly Alexeev <tishka92@yahoo.com> 2009

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// OpenAL based sound handler for mobile units

#include "gameswf/gameswf_sound_handler_openal.h"

#if TU_USE_OPENAL == 1

namespace gameswf
{
	openal_sound_handler::openal_sound_handler() :
		m_device(NULL),
		m_context(NULL)
	{
		// open default device
		m_device = alcOpenDevice(NULL);
		if (m_device)
		{
			m_context = alcCreateContext(m_device, NULL);
			if (m_context)
			{
				printf("\nopenAL uses '%s' device\n", alcGetString(m_device, ALC_DEVICE_SPECIFIER));
				alcMakeContextCurrent(m_context);
			}
			else
			{
				alcCloseDevice(m_device);
				m_device = NULL;
			}
		}
		else
		{
			log_error("Unable to start openAL sound handler\n");
		}
	}

	openal_sound_handler::~openal_sound_handler()
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(m_context);
		alcCloseDevice(m_device);

		m_sound.clear();
	}

	void openal_sound_handler::advance(float delta_time)
	{
		m_mutex.lock();

		for (hash<int, gc_ptr<sound> >::iterator it = m_sound.begin(); it != m_sound.end(); ++it)
		{
			it->second->advance(delta_time);
		}

		m_mutex.unlock();
	}

	// loads external sound file
	// TODO: load MP3, ...
	int	openal_sound_handler::load_sound(const char* url)
	{
		return -1;	//TODO

	/*	int id = create_sound(
			data,
			data_bytes,
			wav_spec.samples,
			FORMAT_NATIVE16,
			wav_spec.freq, 
			wav_spec.channels < 2 ? false : true);

		return id;*/
	}

	// may be used for the creation stream sound head with data_bytes=0 
	int	openal_sound_handler::create_sound(
		void* data,
		int data_bytes,
		int sample_count,
		format_type format,
		int sample_rate,
		bool stereo)
		// Called to create a sample.  We'll return a sample ID that
		// can be use for playing it.
	{
		m_mutex.lock();

		int sound_id = m_sound.size();
		m_sound[sound_id] = new sound(data_bytes, (Uint8*) data, format,
			sample_count, sample_rate, stereo);

		m_mutex.unlock();
		return sound_id;
	}

	void	openal_sound_handler::set_max_volume(int vol)
	{
		if (vol >= 0 && vol <= 100)
		{
			m_max_volume = (float) vol / 100.0f;
		}
	}

	void	openal_sound_handler::append_sound(int sound_handle, void* data, int data_bytes)
	{
		m_mutex.lock();
		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->append(data, data_bytes, this);
		}
		m_mutex.unlock();
	}

	void openal_sound_handler::pause(int sound_handle, bool paused)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->pause(paused);
		}

		m_mutex.unlock();
	}

	void	openal_sound_handler::play_sound(as_object* listener_obj, int sound_handle, int loops)
	// Play the index'd sample.
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			if (listener_obj)
			{
				// create listener
				if (it->second->m_listeners == NULL)
				{
					it->second->m_listeners = new listener();
				}
				it->second->m_listeners->add(listener_obj);
			}

			it->second->play(loops, this);
		}

		m_mutex.unlock();
	}

	void	openal_sound_handler::stop_sound(int sound_handle)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->stop();
		}

		m_mutex.unlock();
	}

	void	openal_sound_handler::delete_sound(int sound_handle)
	// this gets called when it's done with a sample.
	{
		m_mutex.lock();

		//		stop_sound(sound_handle);
		m_sound.erase(sound_handle);

		m_mutex.unlock();
	}

	void	openal_sound_handler::stop_all_sounds()
	{
		m_mutex.lock();

		for (hash<int, gc_ptr<sound> >::iterator it = m_sound.begin(); it != m_sound.end(); ++it)
		{
			it->second->stop();
		}
		m_mutex.unlock();
	}

	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	int	openal_sound_handler::get_volume(int sound_handle)
	{
		m_mutex.lock();

		int vol = 0;
		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			vol = it->second->get_volume();
		}
		m_mutex.unlock();
		return vol;
	}


	//	A number from 0 to 100 representing a volume level.
	//	100 is full volume and 0 is no volume. The default setting is 100.
	void	openal_sound_handler::set_volume(int sound_handle, int volume)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->set_volume(volume);
		}

		m_mutex.unlock();
	}

	void	openal_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, as_object* netstream)
	{
		assert(netstream);
		assert(ptr);

		m_mutex.lock();
		//TODO
		m_mutex.unlock();
	}

	void openal_sound_handler::detach_aux_streamer(as_object* netstream)
	{
		m_mutex.lock();
		//TODO
		m_mutex.unlock();
	}

	void openal_sound_handler::cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, 
		int size, int channels, int freq)
	{
		//TODO
		*adjusted_data = NULL;
		*adjusted_size = 0;
	}

	int openal_sound_handler::get_position(int sound_handle)
	{
		int ms = 0;
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			ALfloat sec = 0;
			alGetSourcef(it->second->m_uiSource, AL_SEC_OFFSET, &sec);
			ms = sec * 1000;
		}
		m_mutex.unlock();
		return ms;
	}

	sound::sound(int size, Uint8* data, sound_handler::format_type format, int sample_count, 
		int sample_rate, bool stereo):
		m_volume(1.0f),
		m_format(format),
		m_sample_count(sample_count),
		m_sample_rate(sample_rate),
		m_stereo(stereo),
		m_is_paused(false),
		m_loops(0),
		m_size(0),
		m_uiSource(0),
		m_uiBuffer(0),
		m_uiBuffers(NULL)
	{
		m_size = size;
		if (data != NULL && size > 0)
		{
			// Generate AL Buffers for streaming
			alGenBuffers(1, &m_uiBuffer);

			// Generate a Source to playback the Buffers
			alGenSources( 1, &m_uiSource );

			switch (m_format)
			{
				case sound_handler::FORMAT_ADPCM :	// gameswf doesn't pass this through; it uncompresses and sends FORMAT_NATIVE16
					assert(0);

				case sound_handler::FORMAT_MP3 :
					log_error("MP3 requires FFMPEG library\n");
					return;

				case sound_handler::FORMAT_NELLYMOSER:	// Mystery proprietary format; see nellymoser.com
					assert(0);
					return;

				case sound_handler::FORMAT_RAW:		// unspecified format.	Useful for 8-bit sounds???
				case sound_handler::FORMAT_UNCOMPRESSED:	// 16 bits/sample, little-endian
				case sound_handler::FORMAT_NATIVE16:	// gameswf extension: 16 bits/sample, native-endian
					break;
			}

			ALenum al_fmt = alGetEnumValue(m_stereo ? "AL_FORMAT_STEREO16" : "AL_FORMAT_MONO16");
			alBufferData(m_uiBuffer, al_fmt, data, size, m_sample_rate);
		}
	}

	sound::~sound()
	{
		// Clean up buffers and sources
		alDeleteSources(1, &m_uiSource);
		alDeleteBuffers(1, &m_uiBuffer);
	}

	void sound::append(void* data, int size, openal_sound_handler* handler)
	{
		// TODO
		return;
	//	m_data = (Uint8*) realloc(m_data, size + m_size);
	//	memcpy(m_data + m_size, data, size);
	//	m_size += size;
	}

	void sound::pause(bool paused)
	{
		m_is_paused = paused;
		if (paused)
		{
			alSourcePause(m_uiSource);
		}
		else
		{
			// Play, replay, or resume a Source
			alSourcePlay(m_uiSource);
		}
	}

	// returns the duration
	int  sound::get_played_bytes()
	{
		ALint duration = 0;
		alGetSourcei(m_uiSource, AL_BYTE_OFFSET, &duration);
		return duration;
	}

	void sound::play(int loops, openal_sound_handler* handler)
	{
		loops = (loops == 0) ? 1 : loops;
		if (loops > 0)
		{
			m_loops = loops;

			alSourcei(m_uiSource, AL_LOOPING, AL_FALSE);

			// TODO more safe
			m_uiBuffers = new ALuint[loops];
			for (int i = 0; i < loops; i++)
			{
				m_uiBuffers[i] = m_uiBuffer;
			}
			alSourceQueueBuffers(m_uiSource, m_loops, m_uiBuffers);

			// Start playing source
			alSourcePlay(m_uiSource);
		}
		else
		if (loops < 0)
		{
			// infinitive
			alSourcei(m_uiSource, AL_LOOPING, AL_TRUE);

			// Start playing source
			alSourcePlay(m_uiSource);
		}
	}

	void sound::stop()
	{
		alSourceStop(m_uiSource);
	}

	void sound::advance(float delta_time)
	{
		if (m_loops > 0)
		{
			ALint state;
			alGetSourcei(m_uiSource, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED)
			{
				alSourceUnqueueBuffers(m_uiSource, m_loops, m_uiBuffers);
				delete m_uiBuffers;
				m_uiBuffers = NULL;

				m_loops = 0;

				// notify onSoundComplete
				if (m_listeners)
				{
					m_listeners->notify(event_id::ON_SOUND_COMPLETE);
				}
			}
		}
	}

	// Factory.
	sound_handler*	create_sound_handler_openal()
	{
		return new openal_sound_handler();
	}

}

#endif // TU_USE_SDL
