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

#include "media.hpp"
#include "zip.hpp"
#include "fe_base.hpp"
#include "fe_file.hpp"
#include <SFML/Graphics.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#define FE_HWACCEL (LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( 55, 78, 100 ))

#if FE_HWACCEL
#include <libavutil/hwcontext.h>
#endif

#if USE_SWRESAMPLE
 #include <libswresample/swresample.h>
 #include <libavutil/opt.h>
 #define DO_RESAMPLE
 #define RESAMPLE_LIB_STR " / swresample "
 #define RESAMPLE_VERSION_MAJOR LIBSWRESAMPLE_VERSION_MAJOR
 #define RESAMPLE_VERSION_MINOR LIBSWRESAMPLE_VERSION_MINOR
 #define RESAMPLE_VERSION_MICRO LIBSWRESAMPLE_VERSION_MICRO
 typedef SwrContext ResampleContext;
 inline void resample_free( ResampleContext **ctx ) { swr_free( ctx ); }
 inline ResampleContext *resample_alloc() { return swr_alloc(); }
 inline int resample_init( ResampleContext *ctx ) { return swr_init( ctx ); }
#else
 #if USE_AVRESAMPLE
  #include <libavresample/avresample.h>
  #include <libavutil/opt.h>
  #define DO_RESAMPLE
  #define RESAMPLE_LIB_STR " / avresample "
  #define RESAMPLE_VERSION_MAJOR LIBAVRESAMPLE_VERSION_MAJOR
  #define RESAMPLE_VERSION_MINOR LIBAVRESAMPLE_VERSION_MINOR
  #define RESAMPLE_VERSION_MICRO LIBAVRESAMPLE_VERSION_MICRO
  typedef AVAudioResampleContext ResampleContext;
  inline void resample_free( ResampleContext **ctx ) { avresample_free( ctx ); }
  inline ResampleContext *resample_alloc() { return avresample_alloc_context(); }
  inline int resample_init( ResampleContext *ctx ) { return avresample_open( ctx ); }
 #endif
#endif

}

#include <queue>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#if (LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT( 59, 0, 100 ))
typedef const AVCodec FeAVCodec;
#else
typedef AVCodec FeAVCodec;
#endif

void try_hw_accel( AVCodecContext *&codec_ctx, FeAVCodec *&dec );

std::string g_decoder;

//
// As of Nov, 2017 RetroPie's default version of avcodec is old enough
// that it doesn't define AV_INPUT_PADDING_SIZE
//
#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE FF_INPUT_BUFFER_PADDING_SIZE
#endif

void print_ffmpeg_version_info()
{
	FeLog() << "avcodec " << LIBAVCODEC_VERSION_MAJOR
		<< '.' << LIBAVCODEC_VERSION_MINOR
		<< '.' << LIBAVCODEC_VERSION_MICRO

		<< " / avformat " << LIBAVFORMAT_VERSION_MAJOR
		<< '.' << LIBAVFORMAT_VERSION_MINOR
		<< '.' << LIBAVFORMAT_VERSION_MICRO

		<< " / swscale " << LIBSWSCALE_VERSION_MAJOR
		<< '.' << LIBSWSCALE_VERSION_MINOR
		<< '.' << LIBSWSCALE_VERSION_MICRO

		<< " / avutil " << LIBAVUTIL_VERSION_MAJOR
		<< '.' << LIBAVUTIL_VERSION_MINOR
		<< '.' << LIBAVUTIL_VERSION_MICRO;

#ifdef DO_RESAMPLE
	FeLog() << RESAMPLE_LIB_STR << RESAMPLE_VERSION_MAJOR
		<< '.' << RESAMPLE_VERSION_MINOR
		<< '.' << RESAMPLE_VERSION_MICRO;
#endif

	FeLog() << std::endl;
}

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//
// Container for our general implementation
//
class FeMediaImp
{
public:
	FeMediaImp( FeMedia::Type t );
	void close();

	FeMedia::Type m_type;
	AVFormatContext *m_format_ctx;
	AVIOContext *m_io_ctx;
	std::recursive_mutex m_read_mutex;
	bool m_read_eof;
};

//
// Base class for our implementation of the audio and video components
//
class FeBaseStream
{
private:
	//
	// Queue containing the next packet to process for this stream
	//
	std::queue <AVPacket *> m_packetq;
	std::recursive_mutex m_packetq_mutex;

public:
	virtual ~FeBaseStream();

	bool at_end;					// set when at the end of our input
	bool far_behind;
	AVCodecContext *codec_ctx;
	FeAVCodec *codec;
	int stream_id;

	FeBaseStream();
	virtual void stop();
	AVPacket *pop_packet();
	void push_packet( AVPacket *pkt );
	void clear_packet_queue();

	// Utility functions to free AV stuff...
	//
	static void free_packet( AVPacket *pkt );
	static void free_frame( AVFrame *frame );
};

//
// Container for our implementation of the audio component
//
class FeAudioImp : public FeBaseStream
{
public:
#ifdef DO_RESAMPLE
	ResampleContext *resample_ctx;
#endif
	sf::Int16 *audio_buff;
	std::recursive_mutex audio_buff_mutex;

	FeAudioImp();
	~FeAudioImp();

	bool process_frame( AVFrame *frame, sf::SoundStream::Chunk &data, int &offset );
};

//
// Container for our implementation of the video component
//
class FeVideoImp : public FeBaseStream
{
private:
	//
	// Video decoding and colour conversion runs on a dedicated thread.
	// Loading the result into an sf::Texture and displaying it is done
	// on the main thread.
	//
	std::thread m_video_thread;
	FeMedia *m_parent;
	sf::Uint8 *rgba_buffer[4];
	int rgba_linesize[4];

#if FE_HWACCEL
	AVPixelFormat hwaccel_output_format;
	bool hw_retrieve_data( AVFrame *f );
#endif

public:
	std::atomic<bool> run_video_thread;
	sf::Time time_base;
	sf::Time max_sleep;
	sf::Clock video_timer;
	sf::Texture *display_texture;
	int disptex_width;
	int disptex_height;

	//
	// The video thread sets display_frame when the next image frame is decoded.
	// The main thread then copies the image into the corresponding sf::Texture.
	//
	std::recursive_mutex image_swap_mutex;
	sf::Uint8 *display_frame;

	FeVideoImp( FeMedia *parent );
	~FeVideoImp();

	void play();
	void stop();

	void signal_stop(); // signal the bg thread we are stopping, without blocking

	void init_rgba_buffer();
	void video_thread();
};

FeMediaImp::FeMediaImp( FeMedia::Type t )
	: m_type( t ),
	m_format_ctx( NULL ),
	m_io_ctx( NULL ),
	m_read_eof( false )
{
}

void FeMediaImp::close()
{
	if (m_format_ctx)
		avformat_close_input( &m_format_ctx );

	if (m_io_ctx)
	{
		if ( m_io_ctx->opaque )
			delete (sf::InputStream *)(m_io_ctx->opaque);

		av_free( m_io_ctx->buffer );
		av_free( m_io_ctx );
		m_io_ctx=NULL;
	}


	m_read_eof=false;
}

FeBaseStream::FeBaseStream()
	: at_end( false ),
	far_behind( false ),
	codec_ctx( NULL ),
	codec( NULL ),
	stream_id( -1 )
{
}

FeBaseStream::~FeBaseStream()
{
	if ( codec_ctx )
		avcodec_free_context( &codec_ctx );

	clear_packet_queue();

	codec = NULL;
	at_end = false;
	far_behind = false;
	stream_id = -1;
}

void FeBaseStream::stop()
{
	clear_packet_queue();
	at_end=false;
	far_behind = false;
}

AVPacket *FeBaseStream::pop_packet()
{
	std::lock_guard<std::recursive_mutex> l( m_packetq_mutex );

	if ( m_packetq.empty() )
		return NULL;

	AVPacket *p = m_packetq.front();
	m_packetq.pop();
	return p;
}

void FeBaseStream::clear_packet_queue()
{
	std::lock_guard<std::recursive_mutex> l( m_packetq_mutex );

	while ( !m_packetq.empty() )
	{
		AVPacket *p = m_packetq.front();
		m_packetq.pop();
		free_packet( p );
	}
}

void FeBaseStream::push_packet( AVPacket *pkt )
{
	std::lock_guard<std::recursive_mutex> l( m_packetq_mutex );
	m_packetq.push( pkt );
}

void FeBaseStream::free_packet( AVPacket *pkt )
{
	av_packet_unref( pkt );
	av_free( pkt );
}

void FeBaseStream::free_frame( AVFrame *frame )
{
	av_frame_unref( frame );
	av_frame_free( &frame );
}

FeAudioImp::FeAudioImp()
	: FeBaseStream(),
#ifdef DO_RESAMPLE
	resample_ctx( NULL ),
#endif
	audio_buff( NULL )
{
}

FeAudioImp::~FeAudioImp()
{
	std::lock_guard<std::recursive_mutex> l( audio_buff_mutex );

#ifdef DO_RESAMPLE
	if ( resample_ctx )
	{
		resample_free( &resample_ctx );
		resample_ctx = NULL;
	}
#endif

	if ( audio_buff )
	{
		av_free( audio_buff );
		audio_buff=NULL;
	}
}

// This function frees the frame
bool FeAudioImp::process_frame( AVFrame *frame, sf::SoundStream::Chunk &data, int &offset )
{
	int data_size = av_samples_get_buffer_size(
		NULL,
		codec_ctx->channels,
		frame->nb_samples,
		codec_ctx->sample_fmt, 1);

#ifdef DO_RESAMPLE
	if ( codec_ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
#endif
	{
		std::lock_guard<std::recursive_mutex> l( audio_buff_mutex );

		memcpy( (audio_buff + offset), frame->data[0], data_size );
		offset += data_size / sizeof( sf::Int16 );
		data.sampleCount += data_size / sizeof(sf::Int16);
		data.samples = audio_buff;
	}
#ifdef DO_RESAMPLE
	else
	{
		std::lock_guard<std::recursive_mutex> l( audio_buff_mutex );

		if ( !resample_ctx )
		{
			resample_ctx = resample_alloc();
			if ( !resample_ctx )
			{
				FeLog() << "Error allocating audio format converter." << std::endl;
				free_frame( frame );
				return false;
			}

			int64_t channel_layout = frame->channel_layout;
			if ( !channel_layout )
			{
				channel_layout = av_get_default_channel_layout(
						codec_ctx->channels );
			}

			av_opt_set_int( resample_ctx, "in_channel_layout", channel_layout, 0 );
			av_opt_set_int( resample_ctx, "in_sample_fmt", frame->format, 0 );
			av_opt_set_int( resample_ctx, "in_sample_rate", frame->sample_rate, 0 );
			av_opt_set_int( resample_ctx, "out_channel_layout", channel_layout, 0 );
			av_opt_set_int( resample_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0 );
			av_opt_set_int( resample_ctx, "out_sample_rate", frame->sample_rate, 0 );

			FeDebug() << "Initializing resampler: in_sample_fmt="
				<< av_get_sample_fmt_name( (AVSampleFormat)frame->format )
				<< ", in_sample_rate=" << frame->sample_rate
				<< ", out_sample_fmt=" << av_get_sample_fmt_name( AV_SAMPLE_FMT_S16 )
				<< ", out_sample_rate=" << frame->sample_rate << std::endl;

			if ( resample_init( resample_ctx ) < 0 )
			{
				FeLog() << "Error initializing audio format converter, input format="
					<< av_get_sample_fmt_name( (AVSampleFormat)frame->format )
					<< ", input sample rate=" << frame->sample_rate << std::endl;
				free_frame( frame );
				resample_free( &resample_ctx );
				resample_ctx = NULL;
				return false;
			}
		}
		if ( resample_ctx )
		{
			int out_linesize;
			av_samples_get_buffer_size(
				&out_linesize,
				codec_ctx->channels,
				frame->nb_samples,
				AV_SAMPLE_FMT_S16, 0 );

			uint8_t *tmp_ptr = (uint8_t *)(audio_buff + offset);

#ifdef USE_SWRESAMPLE
			int out_samples = swr_convert(
				resample_ctx,
				&tmp_ptr,
				frame->nb_samples,
				(const uint8_t **)frame->data,
				frame->nb_samples );
#else // USE_AVRESAMPLE
			int out_samples = avresample_convert(
				resample_ctx,
				&tmp_ptr,
				out_linesize,
				frame->nb_samples,
				frame->data,
				frame->linesize[0],
				frame->nb_samples );
#endif
			if ( out_samples < 0 )
			{
				FeLog() << "Error performing audio conversion." << std::endl;
				free_frame( frame );
				return false;
			}
			offset += out_samples * codec_ctx->channels;
			data.sampleCount += out_samples * codec_ctx->channels;
			data.samples = audio_buff;
		}
	}
#endif

	free_frame( frame );
	return true;
}


FeVideoImp::FeVideoImp( FeMedia *p )
		: FeBaseStream(),
		m_video_thread(),
		m_parent( p ),
		rgba_buffer(),
		rgba_linesize(),
#if FE_HWACCEL
		hwaccel_output_format( AV_PIX_FMT_NONE ),
#endif
		run_video_thread( false ),
		display_texture( NULL ),
		disptex_width( 0 ),
		disptex_height( 0 ),
		display_frame( NULL )
{
}

FeVideoImp::~FeVideoImp()
{
	stop();

	if (rgba_buffer[0])
		av_freep(&rgba_buffer[0]);
}

#if FE_HWACCEL

enum AVPixelFormat hw_get_output_format( AVBufferRef *hw_frames_ctx )
{
	enum AVPixelFormat retval = AV_PIX_FMT_NONE;

	enum AVPixelFormat *p=NULL;
	int e = av_hwframe_transfer_get_formats(
			hw_frames_ctx,
			AV_HWFRAME_TRANSFER_DIRECTION_FROM,
			&p, 0 );

	if ( e < 0 )
		FeLog() << "Error getting supported hardware formats." << std::endl;
	else
		retval = *p;

	av_free( p );

	return retval;
}

bool FeVideoImp::hw_retrieve_data( AVFrame *f )
{
	if ( f->format == AV_PIX_FMT_NONE )
		return false;

	if ( !(av_pix_fmt_desc_get( (AVPixelFormat)f->format )->flags & AV_PIX_FMT_FLAG_HWACCEL) )
		return false;

	AVFrame *sw_frame = av_frame_alloc();
	if ( hwaccel_output_format == AV_PIX_FMT_NONE )
	{
		hwaccel_output_format = hw_get_output_format( codec_ctx->hw_frames_ctx );

		FeDebug() << "HWAccel output pixel format: "
			<< av_pix_fmt_desc_get( hwaccel_output_format )->name << std::endl;
	}

	sw_frame->format = hwaccel_output_format;

	int err = av_hwframe_transfer_data( sw_frame, f, 0 );
	if ( err < 0 )
		FeLog() << "Error transferring hardware frame data." << std::endl;

	err = av_frame_copy_props( sw_frame, f );
	if ( err < 0 )
		FeLog() << "Error copying hardware frame properties." << std::endl;

	av_frame_unref( f );
	av_frame_move_ref( f, sw_frame );
	av_frame_free( &sw_frame );

	return true;
}

#endif

void FeVideoImp::play()
{
	run_video_thread = true;
	video_timer.restart();
	m_video_thread = std::thread( &FeVideoImp::video_thread, this );
}

void FeVideoImp::stop()
{
	if ( run_video_thread )
		run_video_thread = false;

	if ( m_video_thread.joinable() )
		m_video_thread.join();

	FeBaseStream::stop();
}

void FeVideoImp::signal_stop()
{
	if ( run_video_thread )
		run_video_thread = false;
}

namespace
{
	void set_avdiscard_from_qscore( AVCodecContext *c, int qscore )
	{
		AVDiscard d = AVDISCARD_DEFAULT;

		// Note: we aren't ever setting AVDISCARD_ALL
		if ( qscore <= 2 )
				d = AVDISCARD_NONKEY;
		else if ( qscore <= 4 )
				d = AVDISCARD_BIDIR;
		else if ( qscore <= 8 )
			d = AVDISCARD_NONREF;

		c->skip_loop_filter = d;
		c->skip_idct = d;
		c->skip_frame = d;
	}
}

void FeVideoImp::init_rgba_buffer()
{
	std::lock_guard<std::recursive_mutex> l( image_swap_mutex );

	if (rgba_buffer[0])
		av_freep(&rgba_buffer[0]);

	int ret = av_image_alloc(rgba_buffer, rgba_linesize,
			disptex_width, disptex_height,
			AV_PIX_FMT_RGBA, 1);

	if (ret < 0)
		FeLog() << "Error allocating rgba buffer" << std::endl;
}

void FeVideoImp::video_thread()
{
	const int QMAX = 16;
	const int QMIN = 0;
	int qscore( 10 ); // quality scoring
	int displayed( 0 ), qscore_accum( 0 );

	AVFrame *detached_frame = NULL;
	bool degrading = false;
	bool do_flush = false;

	int64_t prev_pts = 0;
	int64_t prev_duration = 0;
	SwsContext *sws_ctx = NULL;

	sf::Time wait_time;

	if (!rgba_buffer[0])
	{
		FeLog() << "Error initializing video thread" << std::endl;
		goto the_end;
	}

	while ( run_video_thread )
	{
		bool do_process = true;

		//
		// If we are falling behind for more than 2 seconds
		// it can only mean that we are in suspend/hibernation state,
		// so we flag the video to be restarted on the next tick.
		// This prevents displaying only keyframes for several seconds on wake.
		//
		if ( wait_time < sf::seconds( -5.0f ) )
		{
			wait_time = sf::seconds( 0 );
			far_behind = true;
			run_video_thread = false;
		}

		//
		// First, display queued frame
		//
		if ( detached_frame )
		{
			wait_time = (sf::Int64)detached_frame->pts * time_base
					- m_parent->get_video_time();

			if ( wait_time < max_sleep )
			{
				if ( wait_time < -time_base )

				{
					// If we are falling behind, we may need to start discarding
					// frames to catch up
					//
					if ( qscore > QMIN )
						qscore--;

					set_avdiscard_from_qscore( codec_ctx, qscore );
					degrading = true;
				}
				else if ( wait_time >= sf::Time::Zero )
				{
					//
					// We are ahead and can sleep until presentation time
					//
					sf::sleep( wait_time );
					degrading = false;

				}

				qscore_accum += qscore;

#if FE_HWACCEL
				hw_retrieve_data( detached_frame );
#endif

				if ( !sws_ctx )
				{
					enum AVPixelFormat pfmt = codec_ctx->pix_fmt;
#if FE_HWACCEL
					if ( hwaccel_output_format != AV_PIX_FMT_NONE )
						pfmt = hwaccel_output_format;
#endif
					int sws_flags( SWS_BILINEAR );
					if ( (codec_ctx->width & 0x7) || (codec_ctx->height & 0x7) )
						sws_flags |= SWS_ACCURATE_RND;

					sws_ctx = sws_getCachedContext( NULL,
						codec_ctx->width, codec_ctx->height, pfmt,
						disptex_width, disptex_height, AV_PIX_FMT_RGBA,
						sws_flags, NULL, NULL, NULL );

					if ( !sws_ctx )
					{
						FeLog() << "Error allocating SwsContext" << std::endl;
						goto the_end;
					}
				}

				std::lock_guard<std::recursive_mutex> l( image_swap_mutex );
				displayed++;

				sws_scale( sws_ctx, detached_frame->data, detached_frame->linesize,
							0, codec_ctx->height, rgba_buffer,
							rgba_linesize );

				display_frame = rgba_buffer[0];

				free_frame( detached_frame );
				detached_frame = NULL;

				do_process = false;
			}
			//
			// if we didn't do anything above, then we go into the queue
			// management process below
			//
		}

		if ( do_flush )
		{
			// flushed last time we did do_process branch below, so this time we
			// exit
			goto the_end;
		}

		if ( do_process )
		{
			if ( !detached_frame )
			{
				//
				// get next packet
				//
				AVPacket *packet = pop_packet();
				if ( packet == NULL )
				{
					if ( !m_parent->end_of_file() )
						m_parent->read_packet();
					else
						do_flush = true; // NULL packet will be fed to avcodec_send_packet()
				}

				if (( packet != NULL ) || ( do_flush ))
				{
					//
					// decompress packet and put it in our frame queue
					//
					int r = avcodec_send_packet( codec_ctx, packet );
					if (( r < 0 ) && ( r != AVERROR(EAGAIN) ))
					{
						char buff[256];
						av_strerror( r, buff, 256 );
						FeLog() << "Error decoding video (sending packet): "
							<< buff << std::endl;
					}

					AVFrame *raw_frame = av_frame_alloc();
					r = avcodec_receive_frame( codec_ctx, raw_frame );

					if ( r != 0 )
					{
						if ( r != AVERROR( EAGAIN ))
						{
							char buff[256];
							av_strerror( r, buff, 256 );
							FeLog() << "Error decoding video (receiving frame): "
								<< buff << std::endl;
						}
						free_frame( raw_frame );
					}
					else
					{
						raw_frame->pts = raw_frame->best_effort_timestamp;

						if ( raw_frame->pts == AV_NOPTS_VALUE )
							raw_frame->pts = packet->dts;

// This only works on FFmpeg, exclude libav (it doesn't have pkt_duration
#if (LIBAVUTIL_VERSION_MICRO >= 100 )
						// Correct for out of bounds pts
						if ( raw_frame->pts < prev_pts )
							raw_frame->pts = prev_pts + prev_duration;

						// Track pts and duration if we need to correct next frame
						prev_pts = raw_frame->pts;
						prev_duration = raw_frame->pkt_duration;
#endif

						detached_frame = raw_frame;

					}

					if ( packet )
						free_packet( packet );
				}
			}
			else if ( !degrading )
			{
				if ( qscore < QMAX )
					qscore++;

				set_avdiscard_from_qscore( codec_ctx, qscore );

				//
				// full frame queue and nothing to display yet, so sleep
				//
				sf::sleep( max_sleep );
			}
		}
	}

the_end:
	//
	// shutdown the thread
	//
	at_end=true;

	{
		std::lock_guard<std::recursive_mutex> l( image_swap_mutex );
		if (display_frame)
			display_frame=NULL;
	}

	if ( detached_frame )
		free_frame( detached_frame );

	if ( sws_ctx )
		sws_freeContext(sws_ctx);

	int average = ( displayed == 0 ) ? qscore_accum : ( qscore_accum / displayed );

	FeDebug() << "End Video Thread - " << m_parent->m_imp->m_format_ctx->url << std::endl
				<< " - bit_rate=" << codec_ctx->bit_rate
				<< ", width=" << codec_ctx->width << ", height=" << codec_ctx->height << std::endl
				<< " - displayed=" << displayed << std::endl
				<< " - average qscore=" << average
				<< std::endl;
}

FeMedia::FeMedia( Type t )
	: sf::SoundStream(),
	m_audio( NULL ),
	m_video( NULL ),
	m_aspect_ratio( 1.0 )
{
	m_imp = new FeMediaImp( t );
}

FeMedia::~FeMedia()
{
	close();

	delete m_imp;
}

sf::Time FeMedia::get_video_time()
{
	//
	// TODO: would like to sync movie time to audio, however using
	// getPlayingOffset() here noticably slows things down on my system.
	//
	if ( m_video )
		return m_video->video_timer.getElapsedTime();
	else
		return sf::Time::Zero;
}

void FeMedia::play()
{
	if ( m_video )
		m_video->play();

	if ( m_audio )
		sf::SoundStream::play();
}

void FeMedia::signal_stop()
{
	if ( m_audio )
		sf::SoundStream::signal_stop();

	if ( m_video )
		m_video->signal_stop();
}

void FeMedia::stop()
{
	if ( m_audio )
	{
		sf::SoundStream::stop();
		m_audio->stop();

		av_seek_frame( m_imp->m_format_ctx, m_audio->stream_id, 0,
							AVSEEK_FLAG_BACKWARD );

		avcodec_flush_buffers( m_audio->codec_ctx );
	}

	if ( m_video )
	{
		m_video->stop();

		av_seek_frame( m_imp->m_format_ctx, m_video->stream_id, 0,
							AVSEEK_FLAG_BACKWARD );

		avcodec_flush_buffers( m_video->codec_ctx );
	}

	m_imp->m_read_eof = false;
}

void FeMedia::close()
{
	stop();

	if (m_audio)
	{
		delete m_audio;
		m_audio=NULL;
	}

	if (m_video)
	{
		delete m_video;
		m_video=NULL;
	}

	m_imp->close();
}

bool FeMedia::is_playing()
{
	if ((m_video) && (m_video->far_behind))
		return false;

	if ((m_video) && (!m_video->at_end))
		return (m_video->run_video_thread);

	return ((m_audio) && (sf::SoundStream::getStatus() == sf::SoundStream::Playing));
}

void FeMedia::setVolume(float volume)
{
	if ( m_audio )
	{
		AVDiscard d =( volume <= 0.f ) ? AVDISCARD_ALL : AVDISCARD_DEFAULT;

		m_audio->codec_ctx->skip_loop_filter = d;
		m_audio->codec_ctx->skip_idct = d;
		m_audio->codec_ctx->skip_frame = d;
	}

	sf::SoundStream::setVolume( volume );
}

int fe_media_read( void *opaque, uint8_t *buff, int buff_size )
{
	sf::InputStream *z = (sf::InputStream *)opaque;

	sf::Int64 bytes_read = z->read( buff, buff_size );

	if ( bytes_read == 0 )
		return AVERROR_EOF;

	return bytes_read;
}

// whence: SEEK_SET, SEEK_CUR, SEEK_END, and AVSEEK_SIZE
int64_t fe_media_seek( void *opaque, int64_t offset, int whence )
{
	sf::InputStream *z = (sf::InputStream *)opaque;

	switch ( whence )
	{
	case AVSEEK_SIZE:
		return z->getSize();

	case SEEK_CUR:
		return z->seek( z->tell() + offset );

	case SEEK_END:
		return z->seek( z->getSize() + offset );

	case SEEK_SET:
	default:
		return z->seek( offset );
	}
}

bool FeMedia::open( const std::string &archive,
	const std::string &name, sf::Texture *outt )
{
	close();

	sf::InputStream *s = NULL;

	if ( !archive.empty() )
	{
		FeZipStream *z = new FeZipStream( archive );

		if ( !z->open( name ) )
		{
			// Error opening specified filename. Try to correct
			// in case filname is in a subdir of the archive
			std::string temp;
			if ( get_archive_filename_with_base( temp, archive, name ) )
			{
				z->open( temp );
			}
			else
			{
				delete s;
				return false;
			}
		}

		s = (sf::InputStream *)z;
	}
	else
		s = new FeFileInputStream( name );

	m_imp->m_format_ctx = avformat_alloc_context();

	size_t avio_ctx_buffer_size = 4096;
	uint8_t *avio_ctx_buffer = (uint8_t *)av_malloc( avio_ctx_buffer_size
			+ AV_INPUT_BUFFER_PADDING_SIZE );

	memset( avio_ctx_buffer + avio_ctx_buffer_size,
		0,
		AV_INPUT_BUFFER_PADDING_SIZE );

	m_imp->m_io_ctx = avio_alloc_context( avio_ctx_buffer,
		avio_ctx_buffer_size, 0, s, &fe_media_read,
		NULL, &fe_media_seek );

	m_imp->m_format_ctx->pb = m_imp->m_io_ctx;

	if ( avformat_open_input( &(m_imp->m_format_ctx), name.c_str(), NULL, NULL ) < 0 )
	{
		FeLog() << "Error opening input file: " << name << std::endl;
		return false;
	}

	if ( avformat_find_stream_info( m_imp->m_format_ctx, NULL ) < 0 )
	{
		FeLog() << "Error finding stream information in input file: "
					<< m_imp->m_format_ctx->url << std::endl;
		return false;
	}

	if ( m_imp->m_type & Audio )
	{
		int stream_id( -1 );
		FeAVCodec *dec;

		stream_id = av_find_best_stream( m_imp->m_format_ctx, AVMEDIA_TYPE_AUDIO,
			-1, -1, &dec, 0 );

		if ( stream_id >= 0 )
		{

			AVCodecContext *codec_ctx;
			codec_ctx = avcodec_alloc_context3( NULL );

			avcodec_parameters_to_context( codec_ctx, m_imp->m_format_ctx->streams[stream_id]->codecpar );

			codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

			if ( avcodec_open2( codec_ctx, dec, NULL ) < 0 )
			{
				FeLog() << "Could not open audio decoder for file: "
						<< m_imp->m_format_ctx->url << std::endl;
				avcodec_free_context( &codec_ctx );
			}
			else
			{
				m_audio = new FeAudioImp();
				m_audio->stream_id = stream_id;
				m_audio->codec_ctx = codec_ctx;
				m_audio->codec = dec;

				//
				// TODO: Fix buffer sizing, we allocate way
				// more than we use
				//
				m_audio->audio_buff = (sf::Int16 *)av_malloc(
					MAX_AUDIO_FRAME_SIZE
					+ AV_INPUT_BUFFER_PADDING_SIZE
					+ codec_ctx->sample_rate );

				sf::SoundStream::initialize(
					codec_ctx->channels,
					codec_ctx->sample_rate );

				sf::SoundStream::setLoop( false );

#ifndef DO_RESAMPLE
				if ( codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16 )
				{
					FeLog() << "Warning: Attract-Mode was compiled without an audio resampler (libswresample or libavresample)." << std::endl
						<< "The audio format in " << m_imp->m_format_ctx->url << " appears to need resampling.  It will likely sound like garbage." << std::endl;
				}
#endif
			}
		}
	}

	if ( m_imp->m_type & Video )
	{
		std::string prev_dec_name;
		int av_result( -1 );
		int stream_id( -1 );
		FeAVCodec *dec;

		stream_id = av_find_best_stream( m_imp->m_format_ctx, AVMEDIA_TYPE_VIDEO,
					-1, -1, &dec, 0 );

		if ( stream_id < 0 )
		{
			FeLog() << "No video stream found, file: "
				<< m_imp->m_format_ctx->url << std::endl;
		}
		else
		{

			AVCodecContext *codec_ctx;
			codec_ctx = avcodec_alloc_context3( NULL );

			avcodec_parameters_to_context( codec_ctx, m_imp->m_format_ctx->streams[stream_id]->codecpar );

			codec_ctx->workaround_bugs = FF_BUG_AUTODETECT;

			// Note also: http://trac.ffmpeg.org/ticket/4404
			codec_ctx->thread_count=1;

			if (dec)
				prev_dec_name = std::string(dec->name);

			try_hw_accel( codec_ctx, dec );

			av_result = avcodec_open2( codec_ctx, dec, NULL );
			if ( av_result < 0 )
			{
				if ( !prev_dec_name.empty() && (g_decoder.compare( "mmal" ) == 0) )
				{
					switch( dec->id )
					{


					case AV_CODEC_ID_VC1:
					case AV_CODEC_ID_MPEG2VIDEO:
					case AV_CODEC_ID_H264:
					case AV_CODEC_ID_MPEG4:
						FeLog() << "mmal video decoding (" << dec->name
							<< ") not supported for file (trying software): "
							<< m_imp->m_format_ctx->url << std::endl;

						dec = avcodec_find_decoder_by_name(prev_dec_name.c_str());

						av_result = avcodec_open2( codec_ctx, dec, NULL );
						break;

					default:
						break;
					}
				}

				if ( av_result < 0 )
				{
					FeLog() << "Could not open video decoder for file: "
							<< m_imp->m_format_ctx->url << std::endl;
					avcodec_free_context( &codec_ctx );
				}
			}

			if ( av_result >=0  )
			{
				m_video = new FeVideoImp( this );

				m_video->stream_id = stream_id;
				m_video->codec_ctx = codec_ctx;

				m_video->codec = dec;
				m_video->time_base = sf::seconds(
						av_q2d(m_imp->m_format_ctx->streams[stream_id]->time_base) );

				m_video->max_sleep = sf::seconds( 0.5 / av_q2d(m_imp->m_format_ctx->streams[stream_id]->r_frame_rate));

				if ( codec_ctx->sample_aspect_ratio.num != 0 )
					m_aspect_ratio = av_q2d( codec_ctx->sample_aspect_ratio );
				if ( m_imp->m_format_ctx->streams[stream_id]->sample_aspect_ratio.num != 0 )
					m_aspect_ratio = av_q2d( m_imp->m_format_ctx->streams[stream_id]->sample_aspect_ratio );

				m_video->disptex_width = codec_ctx->width;
				m_video->disptex_height = codec_ctx->height;

				m_video->display_texture = outt;
				if ( outt->getSize() != sf::Vector2u( m_video->disptex_width, m_video->disptex_height ) )
					m_video->display_texture->create( m_video->disptex_width, m_video->disptex_height );

				m_video->init_rgba_buffer();
			}
		}
	}

	if ( (!m_video) && (!m_audio) )
		return false;

	return true;
}

bool FeMedia::end_of_file()
{
	std::lock_guard<std::recursive_mutex> l( m_imp->m_read_mutex );

	bool retval = ( m_imp->m_read_eof );
	return retval;
}

bool FeMedia::read_packet()
{
	std::lock_guard<std::recursive_mutex> l( m_imp->m_read_mutex );

	if ( m_imp->m_read_eof )
		return false;

	AVPacket *pkt = (AVPacket *)av_malloc( sizeof( *pkt ) );

	int r = av_read_frame( m_imp->m_format_ctx, pkt );
	if ( r < 0 )
	{
		m_imp->m_read_eof=true;
		FeBaseStream::free_packet( pkt );
		return false;
	}

	if ( ( m_audio ) && ( pkt->stream_index == m_audio->stream_id ) )
		m_audio->push_packet( pkt );
	else if ( ( m_video ) && (pkt->stream_index == m_video->stream_id ) )
		m_video->push_packet( pkt );
	else
		FeBaseStream::free_packet( pkt );

	return true;
}

bool FeMedia::tick()
{
	if (( !m_video ) && ( !m_audio ))
		return false;

	if ( m_video )
	{
		std::lock_guard<std::recursive_mutex> l( m_video->image_swap_mutex );
		if ( m_video->display_frame )
		{
			m_video->display_texture->update( m_video->display_frame );
			m_video->display_frame = NULL;
			return true;
		}
	}

	return false;
}


bool FeMedia::onGetData( Chunk &data )
{
	int offset=0;

	data.samples = NULL;
	data.sampleCount = 0;

	if ( (!m_audio) || end_of_file() )
		return false;

	while ( offset < m_audio->codec_ctx->sample_rate )
	{
		AVPacket *packet = m_audio->pop_packet();
		while (( packet == NULL ) && ( !end_of_file() ))
		{
			read_packet();
			packet = m_audio->pop_packet();
		}

		if ( packet == NULL )
		{
			m_audio->at_end=true;
			if ( offset > 0 )
				return true;
			return false;
		}

		int r = avcodec_send_packet( m_audio->codec_ctx, packet );
		if (( r < 0 ) && ( r != AVERROR(EAGAIN) ))
		{
			char buff[256];
			av_strerror( r, buff, 256 );
			FeLog() << "Error decoding audio (sending packet): " << buff << std::endl;
		}

		FeBaseStream::free_packet( packet );

		r = AVERROR(EAGAIN);

		//
		// Note that avcodec_receive_frame() may need to return multiple frames per packet
		// depending on the audio codec.
		//
		do
		{
			AVFrame *frame = av_frame_alloc();
			r = avcodec_receive_frame( m_audio->codec_ctx, frame );

			if ( r == 0 )
			{
				if ( !m_audio->process_frame( frame, data, offset ) )
					return false;
			}
			else
			{
				if ( r != AVERROR(EAGAIN) )
				{
					char buff[256];
					av_strerror( r, buff, 256 );
					FeLog() << "Error decoding audio (receiving frame): " << buff << std::endl;
				}
			}
		} while ( r != AVERROR(EAGAIN) );
	}

	return true;
}

void FeMedia::onSeek( sf::Time timeOffset )
{
	// Not implemented
}

bool FeMedia::is_supported_media_file( const std::string &filename )
{
	// Work around for FFmpeg not recognizing certain file extensions
	// that it supports (xmv reported as of Dec 2015)
	//
	size_t pos = filename.find_last_of( '.' );
	if ( pos != std::string::npos )
	{
		std::string f = filename.substr(pos+1);
		std::transform( f.begin(), f.end(), f.begin(), ::tolower );
		return ( av_guess_format( f.c_str(), filename.c_str(),
			NULL ) != NULL );
	}

	return ( av_guess_format( NULL, filename.c_str(), NULL ) != NULL );
}

bool FeMedia::is_multiframe() const
{
	if ( m_video && m_imp->m_format_ctx )
	{
		AVStream *s = m_imp->m_format_ctx->streams[ m_video->stream_id ];

#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT( 56, 13, 0 ))
		if (( s->nb_frames > 1 )
				|| ( s->id == AV_CODEC_ID_APNG )
				|| ( s->id == AV_CODEC_ID_GIF ))
			return true;
#else
		if ( s->nb_frames > 1 )
			return true;
#endif

	}

	return false;
}

float FeMedia::get_aspect_ratio() const
{
	return m_aspect_ratio;
}

sf::Time FeMedia::get_duration() const
{
	if ( m_video && m_imp->m_format_ctx )
	{
		return sf::seconds(
				av_q2d( m_imp->m_format_ctx->streams[m_video->stream_id]->time_base ) *
							m_imp->m_format_ctx->streams[ m_video->stream_id ]->duration );
	}

	return sf::Time::Zero;
}

const char *FeMedia::get_metadata( const char *tag )
{
	if ( !m_imp->m_format_ctx )
		return "";

	AVDictionaryEntry *entry = NULL;
	entry = av_dict_get( m_imp->m_format_ctx->metadata, tag, NULL, AV_DICT_IGNORE_SUFFIX );

	return ( entry ? entry->value : "" );
}

#if FE_HWACCEL
//
// A list of the 'HWDEVICE' ffmpeg hwaccels that we support
//
enum AVHWDeviceType fe_hw_accels[] =
{
#ifdef FE_HWACCEL_VAAPI
	AV_HWDEVICE_TYPE_VAAPI,
#endif
#ifdef FE_HWACCEL_VDPAU
	AV_HWDEVICE_TYPE_VDPAU,
#endif

#ifdef SFML_SYSTEM_WINDOWS
	AV_HWDEVICE_TYPE_DXVA2,
 #if (LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( 56, 67, 100 ))
	AV_HWDEVICE_TYPE_D3D11VA,
 #endif
#endif

#ifdef SFML_SYSTEM_MACOS
 #if (LIBAVUTIL_VERSION_INT >= AV_VERSION_INT( 57, 63, 100 ))
	AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
 #endif
#endif

	AV_HWDEVICE_TYPE_NONE
};
#endif

void FeMedia::get_decoder_list( std::vector< std::string > &l )
{
	l.clear();

	l.push_back( "software" );

#if defined(USE_GLES) || defined(USE_MMAL)
	//
	// Raspberry Pi specific - check for mmal
	//
	if ( avcodec_find_decoder_by_name( "mpeg4_mmal" ) )
		l.push_back( "mmal" );
#endif

#if FE_HWACCEL
	for ( int i=0; fe_hw_accels[i] != AV_HWDEVICE_TYPE_NONE; i++ )
	{
		const char *name = av_hwdevice_get_type_name( fe_hw_accels[i] );
		if ( name != NULL )
			l.push_back( name );
	}
#endif
}

std::string FeMedia::get_current_decoder()
{
	return g_decoder;
}

void FeMedia::set_current_decoder( const std::string &l )
{
	g_decoder = l;
}

//
// Try to use a hardware accelerated decoder where readily available...
//
void try_hw_accel( AVCodecContext *&codec_ctx, FeAVCodec *&dec )
{
	if ( g_decoder.empty() || ( g_decoder.compare( "software" ) == 0 ))
		return;

#if defined(USE_GLES) || defined(USE_MMAL)
	if ( g_decoder.compare( "mmal" ) == 0 )
	{
		switch( dec->id )
		{

		case AV_CODEC_ID_MPEG4:
			dec = avcodec_find_decoder_by_name( "mpeg4_mmal" );
			return;


		case AV_CODEC_ID_H264:
			dec = avcodec_find_decoder_by_name( "h264_mmal" );
			return;


		case AV_CODEC_ID_MPEG2VIDEO:
			dec = avcodec_find_decoder_by_name( "mpeg2_mmal" );
			return;

		case AV_CODEC_ID_VC1:
			dec = avcodec_find_decoder_by_name( "vc1_mmal" );
			return;

		default:
			break;
		};

		return;
	}
#endif

#if FE_HWACCEL
	for ( int i=0; fe_hw_accels[i] != AV_HWDEVICE_TYPE_NONE; i++ )
	{
		if ( g_decoder.compare( av_hwdevice_get_type_name( fe_hw_accels[i] ) ) != 0 )
			continue;

		AVBufferRef *device_ctx=NULL;
		int ret = av_hwdevice_ctx_create( &device_ctx, fe_hw_accels[i], NULL, NULL, 0 );

		if ( ret < 0 )
		{
			FeLog() << "error creating hw device context: "
				<< av_hwdevice_get_type_name( fe_hw_accels[i] ) << std::endl;
			return;
		}

		codec_ctx->hw_device_ctx = device_ctx; // we are passing our buffer ref on device_ctx to codec_ctx here...
		codec_ctx->hwaccel_flags = AV_HWACCEL_FLAG_IGNORE_LEVEL;

		FeDebug() << "created hw device: "
				<< av_hwdevice_get_type_name( fe_hw_accels[i] ) << std::endl;
	}
#endif
}
