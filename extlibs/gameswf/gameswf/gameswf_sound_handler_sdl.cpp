// gameswf_sound_handler_sdl.cpp	-- Thatcher Ulrich http://tulrich.com 2003
// gameswf_sound_handler_sdl.cpp	-- Vitaly Alexeev <tishka92@yahoo.com> 2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#include "gameswf/gameswf_sound_handler_sdl.h"

#ifdef TU_USE_SDL

#pragma comment (lib, "SDL.lib")


namespace gameswf
{
	tu_mutex& gameswf_engine_mutex();
	static void sdl_audio_callback(void *udata, Uint8 *stream, int len); // SDL C audio handler

	SDL_sound_handler::SDL_sound_handler():
		m_max_volume(1.0f),
		m_is_open(true)
	{
		// This is our sound settings
		m_audioSpec.freq = 44100;
		m_audioSpec.format = AUDIO_S16SYS;
		m_audioSpec.channels = 2;
		m_audioSpec.callback = sdl_audio_callback;
		m_audioSpec.userdata = this;
		m_audioSpec.samples = 4096;

#if TU_CONFIG_LINK_TO_FFMPEG == 1
		avcodec_init();
		av_log_set_level(-1);		// do not print ffmeg messages
		avcodec_register_all();
		m_MP3_codec = avcodec_find_decoder(CODEC_ID_MP3);
#endif

		if (SDL_OpenAudio(&m_audioSpec, NULL) < 0 )
		{
			log_error("Unable to start sound handler: %s\n", SDL_GetError());
			m_is_open = false;
			return;
		}

		SDL_PauseAudio(1);
	}

	SDL_sound_handler::~SDL_sound_handler()
	{
		if (m_is_open) SDL_CloseAudio();
		m_sound.clear();
	}

	// loads external sound file
	// TODO: load MP3, ...
	int	SDL_sound_handler::load_sound(const char* url)
	{
		SDL_AudioSpec wav_spec;
		Uint32	data_bytes;
		Uint8*	data;

		if (SDL_LoadWAV(url, &wav_spec, &data, &data_bytes) == NULL)
		{
			log_error("loadSound: can't load %s\n", url);
			return -1;
		}

		int id = create_sound(
			data,
			data_bytes,
			wav_spec.samples,
			FORMAT_NATIVE16,
			wav_spec.freq, 
			wav_spec.channels < 2 ? false : true);

		SDL_FreeWAV(data);

		return id;
	}

	// may be used for the creation stream sound head with data_bytes=0 
	int	SDL_sound_handler::create_sound(
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

	void	SDL_sound_handler::set_max_volume(int vol)
	{
		if (vol >= 0 && vol <= 100)
		{
			m_max_volume = (float) vol / 100.0f;
		}
	}

	void	SDL_sound_handler::append_sound(int sound_handle, void* data, int data_bytes)
	{
		m_mutex.lock();
		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->append(data, data_bytes, this);
		}
		m_mutex.unlock();
	}

	void SDL_sound_handler::pause(int sound_handle, bool paused)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->pause(paused);
		}

		m_mutex.unlock();
	}

	void	SDL_sound_handler::play_sound(as_object* listener_obj, int sound_handle, int loops)
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

	void	SDL_sound_handler::stop_sound(int sound_handle)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->clear_playlist();
		}

		m_mutex.unlock();
	}


	void	SDL_sound_handler::delete_sound(int sound_handle)
	// this gets called when it's done with a sample.
	{
		m_mutex.lock();

		//		stop_sound(sound_handle);
		m_sound.erase(sound_handle);

		m_mutex.unlock();
	}

	void	SDL_sound_handler::stop_all_sounds()
	{
		m_mutex.lock();

		for (hash<int, gc_ptr<sound> >::iterator it = m_sound.begin(); it != m_sound.end(); ++it)
		{
			it->second->clear_playlist();
		}
		m_mutex.unlock();
	}

	//	returns the sound volume level as an integer from 0 to 100,
	//	where 0 is off and 100 is full volume. The default setting is 100.
	int	SDL_sound_handler::get_volume(int sound_handle)
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
	void	SDL_sound_handler::set_volume(int sound_handle, int volume)
	{
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			it->second->set_volume(volume);
		}

		m_mutex.unlock();
	}

	void	SDL_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, as_object* netstream)
	{
		assert(netstream);
		assert(ptr);

		m_mutex.lock();

		m_aux_streamer[netstream] = ptr;
		SDL_PauseAudio(0);

		m_mutex.unlock();
	}

	void SDL_sound_handler::detach_aux_streamer(as_object* netstream)
	{
		m_mutex.lock();
		m_aux_streamer.erase(netstream);
		m_mutex.unlock();
	}

	void SDL_sound_handler::cvt(short int** adjusted_data, int* adjusted_size, unsigned char* data, 
		int size, int channels, int freq)
	{
		*adjusted_data = NULL;
		*adjusted_size = 0;

		SDL_AudioCVT  wav_cvt;
		int rc = SDL_BuildAudioCVT(&wav_cvt, AUDIO_S16SYS, channels, freq,
			m_audioSpec.format, m_audioSpec.channels, m_audioSpec.freq);
		if (rc == -1)
		{
			log_error("Couldn't build SDL audio converter\n");
			return;
		}

		// Setup for conversion 
		wav_cvt.buf = (Uint8*) malloc(size * wav_cvt.len_mult);
		wav_cvt.len = size;
		memcpy(wav_cvt.buf, data, size);

		// We can delete to original WAV data now 
		// free(data);

		// And now we're ready to convert
		SDL_ConvertAudio(&wav_cvt);

		*adjusted_data = (int16*) wav_cvt.buf;
		*adjusted_size = size * wav_cvt.len_mult;
	}

	int SDL_sound_handler::get_position(int sound_handle)
	{
		int ms = 0;
		m_mutex.lock();

		hash<int, gc_ptr<sound> >::iterator it = m_sound.find(sound_handle);
		if (it != m_sound.end())
		{
			int bytes = it->second->get_played_bytes();

			// convert to the time in ms
			ms = bytes / (m_audioSpec.freq *(m_audioSpec.channels > 1 ? 4 : 2) / 1000);
		}

		m_mutex.unlock();
		return ms;
	}

	sound::sound(int size, Uint8* data, sound_handler::format_type format, int sample_count, 
		int sample_rate, bool stereo):
		m_data(data),
		m_size(size),
		m_volume(1.0f),
		m_format(format),
		m_sample_count(sample_count),
		m_sample_rate(sample_rate),
		m_stereo(stereo),
		m_is_paused(false)
	{
		if (data != NULL && size > 0)
		{
			m_data = (Uint8*) malloc(size);
			memcpy(m_data, data, size);
		}
	}

	sound::~sound()
	{
		free(m_data);
	}

	void sound::append(void* data, int size, SDL_sound_handler* handler)
	{
		m_data = (Uint8*) realloc(m_data, size + m_size);
		memcpy(m_data + m_size, data, size);
		m_size += size;
	}

	void sound::pause(bool paused)
	{
		m_is_paused = paused;
	}

	// returns the current sound position in ms
	int  sound::get_played_bytes()
	{
		// What to do if m_playlist.size() > 1 ???
		if (m_playlist.size() > 0)
		{
			return m_playlist[0]->get_played_bytes();
		}
		return 0;
	}

	void sound::play(int loops, SDL_sound_handler* handler)
	{
		if (m_size == 0)
		{
			log_error("the attempt to play the empty sound\n");
			return;
		}

#if TU_CONFIG_LINK_TO_FFMPEG == 1
		m_playlist.push_back(new active_sound(this, loops));
		SDL_PauseAudio(0);
#else
		if (m_format != sound_handler::FORMAT_MP3)
		{
			m_playlist.push_back(new active_sound(this, loops));
			SDL_PauseAudio(0);
		}
		else
		{
			log_error("MP3 requires FFMPEG library\n");
		}
#endif
	}

	// called from audio callback
	bool sound::mix(Uint8* stream, int len, array< gc_ptr<listener> >* listeners, float max_volume)
	{
		if (m_is_paused)
		{
			return true;
		}

		Uint8* buf = new Uint8[len];

		bool play = false;
		for (int i = 0; i < m_playlist.size(); )
		{
			play = true;
			if (m_playlist[i]->mix(buf, len))
			{
				i++;
			}
			else
			{
				if (m_listeners != NULL)
				{
					listeners->push_back(m_listeners);
				}

				m_playlist.remove(i);
			}

			int volume = (int) floorf(SDL_MIX_MAXVOLUME * max_volume * m_volume);
			SDL_MixAudio(stream, buf, len, volume);
		}

		delete [] buf;
		return play;
	}

	sound_handler*	create_sound_handler_sdl()
	// Factory.
	{
		return new SDL_sound_handler;
	}

	/// The callback function which refills the buffer with data
	/// We run through all of the sounds, and mix all of the active sounds 
	/// into the stream given by the callback.
	/// If sound is compresssed (mp3) a mp3-frame is decoded into a buffer,
	/// and resampled if needed. When the buffer has been sampled, another
	/// frame is decoded until all frames has been decoded.
	/// If a sound is looping it will be decoded from the beginning again.

	static void	sdl_audio_callback (void *udata, Uint8 *stream, int len)
	{
		// Get the soundhandler
		SDL_sound_handler* handler = static_cast<SDL_sound_handler*>(udata);

		handler->m_mutex.lock();

		int pause = 1;
		memset(stream, 0, len);

		array< gc_ptr<listener> > listeners;

		// mix Flash audio
		for (hash<int, gc_ptr<sound> >::iterator snd = handler->m_sound.begin();
			snd != handler->m_sound.end(); ++snd)
		{
			bool play = snd->second->mix(stream, len, &listeners, handler->m_max_volume);
			pause = play ? 0 : pause;
		}

		// mix audio of video 
		if (handler->m_aux_streamer.size() > 0)
		{
			Uint8* mix_buf = new Uint8[len];
			for (hash< as_object*, gameswf::sound_handler::aux_streamer_ptr>::const_iterator it = handler->m_aux_streamer.begin();
			     it != handler->m_aux_streamer.end();
			     ++it)
			{
				memset(mix_buf, 0, len); 

				gameswf::sound_handler::aux_streamer_ptr aux_streamer = it->second;
				(aux_streamer)(it->first, mix_buf, len);

				int volume = (int) floorf(SDL_MIX_MAXVOLUME * handler->m_max_volume);
				SDL_MixAudio(stream, mix_buf, len, volume);
			}
			pause = 0;
			delete [] mix_buf;
		}
		SDL_PauseAudio(pause);
		handler->m_mutex.unlock();

		// notify onSoundComplete
		gameswf_engine_mutex().lock();
		for (int i = 0, n = listeners.size(); i < n; i++)
		{
			listeners[i]->notify(event_id::ON_SOUND_COMPLETE);
		}
		gameswf_engine_mutex().unlock();

	}

}

#endif  // TU_USE_SDL

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
