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

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

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
#include "media.hpp"

void print_ffmpeg_version_info()
{
	std::cout << "Using "
		<< (( LIBAVCODEC_VERSION_MICRO >= 100 ) ? "FFmpeg" : "Libav" )
		<< " for Audio and Video." << std::endl

		<< "avcodec " << LIBAVCODEC_VERSION_MAJOR
		<< '.' << LIBAVCODEC_VERSION_MINOR
		<< '.' << LIBAVCODEC_VERSION_MICRO

		<< " / avformat " << LIBAVFORMAT_VERSION_MAJOR
		<< '.' << LIBAVFORMAT_VERSION_MINOR
		<< '.' << LIBAVFORMAT_VERSION_MICRO

		<< " / swscale " << LIBSWSCALE_VERSION_MAJOR
		<< '.' << LIBSWSCALE_VERSION_MINOR
		<< '.' << LIBSWSCALE_VERSION_MICRO;

#ifdef DO_RESAMPLE
	std::cout << RESAMPLE_LIB_STR << RESAMPLE_VERSION_MAJOR
		<< '.' << RESAMPLE_VERSION_MINOR
		<< '.' << RESAMPLE_VERSION_MICRO;
#endif
	std::cout << std::endl;
}

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
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
	sf::Mutex m_packetq_mutex;

public:
	bool at_end;					// set when at the end of our input
	AVCodecContext *codec_ctx;
	AVCodec *codec;
	int stream_id;

	FeBaseStream();
	void close();
	void stop();
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
	sf::Int16 *buffer;
	sf::Mutex buffer_mutex;

	FeAudioImp();
	~FeAudioImp();
	void close();
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
	sf::Thread m_video_thread;
	FeMedia *m_parent;

public:
	bool run_video_thread;
	sf::Time time_base;
	sf::Texture display_texture;
	sf::Clock video_timer;

	//
	// The video thread sets display_frame and display_frame_ready when
	// the next image is ready for display.  The main thread then copies
	// the image data into the corresponding sf::Texture
	//
	sf::Mutex image_swap_mutex;
	sf::Uint8 *display_frame;

	FeVideoImp( FeMedia *parent );
	void play();
	void stop();
	void close();

	void preload();
	void video_thread();
};

FeBaseStream::FeBaseStream()
	: at_end( false ),
	codec_ctx( NULL ),
	codec( NULL ),
	stream_id( -1 )
{
}

void FeBaseStream::close()
{
	if ( codec_ctx )
	{
		avcodec_close( codec_ctx );
		codec_ctx = NULL;
	}

	clear_packet_queue();

	codec = NULL;
	at_end = false;
	stream_id = -1;
}

void FeBaseStream::stop()
{
	clear_packet_queue();
	at_end=false;
}

AVPacket *FeBaseStream::pop_packet()
{
	sf::Lock l( m_packetq_mutex );

	if ( m_packetq.empty() )
		return NULL;

	AVPacket *p = m_packetq.front();
	m_packetq.pop();
	return p;
}

void FeBaseStream::clear_packet_queue()
{
	sf::Lock l( m_packetq_mutex );

	while ( !m_packetq.empty() )
	{
		AVPacket *p = m_packetq.front();
		m_packetq.pop();
		free_packet( p );
	}
}

void FeBaseStream::push_packet( AVPacket *pkt )
{
	sf::Lock l( m_packetq_mutex );
	m_packetq.push( pkt );
}

void FeBaseStream::free_packet( AVPacket *pkt )
{
	av_free_packet( pkt );
	av_free( pkt );
}

void FeBaseStream::free_frame( AVFrame *frame )
{
#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT( 54, 28, 0 ))
	avcodec_free_frame( &frame );
#else
	av_free( frame );
#endif
}

FeAudioImp::FeAudioImp()
	: FeBaseStream(),
#ifdef DO_RESAMPLE
	resample_ctx( NULL ),
#endif
	buffer( NULL )
{
}

FeAudioImp::~FeAudioImp()
{
	close();
}

void FeAudioImp::close()
{
#ifdef DO_RESAMPLE
	if ( resample_ctx )
	{
		sf::Lock l( buffer_mutex );
		resample_free( &resample_ctx );
		resample_ctx = NULL;
	}
#endif

	if ( buffer )
	{
		sf::Lock l( buffer_mutex );

		av_free( buffer );
		buffer=NULL;
	}

	FeBaseStream::close();
}

FeVideoImp::FeVideoImp( FeMedia *p )
		: FeBaseStream(),
		m_video_thread( &FeVideoImp::video_thread, this ),
		m_parent( p ),
		run_video_thread( false ),
		display_frame( NULL )
{
}

void FeVideoImp::play()
{
	run_video_thread = true;
	video_timer.restart();
	m_video_thread.launch();
}

void FeVideoImp::stop()
{
	if ( run_video_thread )
	{
		run_video_thread = false;
		m_video_thread.wait();
	}

	FeBaseStream::stop();
}

void FeVideoImp::close()
{
	stop();
	FeBaseStream::close();
}

namespace
{
	void set_avdiscard_from_qscore( AVCodecContext *c, int qscore )
	{
		AVDiscard d = AVDISCARD_DEFAULT;

		if ( qscore <= -40 )
		{
			if ( qscore <= -120 )
				d = AVDISCARD_ALL;
			else if ( qscore <= -100 )
				d = AVDISCARD_NONKEY;
			else
				d = AVDISCARD_BIDIR;
		}
		else if ( qscore <= 0 )
			d = AVDISCARD_NONREF;

		c->skip_loop_filter = d;
		c->skip_idct = d;
		c->skip_frame = d;
	}
}

void FeVideoImp::preload()
{
	bool keep_going = true;
	while ( keep_going )
	{
		AVPacket *packet = pop_packet();
		if ( packet == NULL )
		{
			if ( !m_parent->end_of_file() )
				m_parent->read_packet();
			else
				keep_going = false;
		}
		else
		{
			//
			// decompress packet and put it in our frame queue
			//
			int got_frame = 0;
			AVFrame *raw_frame = avcodec_alloc_frame();

			int len = avcodec_decode_video2( codec_ctx, raw_frame,
									&got_frame, packet );
			if ( len < 0 )
			{
				std::cout << "error decoding video" << std::endl;
				free_frame( raw_frame );
				free_packet( packet );
				return;
			}

			if ( got_frame )
			{
				AVPicture *my_pict = (AVPicture *)av_malloc( sizeof( AVPicture ) );
				avpicture_alloc( my_pict, PIX_FMT_RGBA,
										codec_ctx->width,
										codec_ctx->height );

				if ( !my_pict )
				{
					std::cerr << "Error allocating AVPicture during preload" << std::endl;
					free_frame( raw_frame );
					free_packet( packet );
					return;
				}

				SwsContext *sws_ctx = sws_getContext(
								codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
								codec_ctx->width, codec_ctx->height, PIX_FMT_RGBA,
								SWS_FAST_BILINEAR, NULL, NULL, NULL );

				if ( !sws_ctx )
				{
					std::cerr << "Error allocating SwsContext during preload" << std::endl;
					avpicture_free( my_pict );
					av_free( my_pict );
					free_frame( raw_frame );
					free_packet( packet );
					return;
				}

				sws_scale( sws_ctx, raw_frame->data, raw_frame->linesize,
							0, codec_ctx->height, my_pict->data,
							my_pict->linesize );

				sf::Lock l( image_swap_mutex );
				display_texture.update( my_pict->data[0] );

				keep_going = false;

				sws_freeContext( sws_ctx );
				avpicture_free( my_pict );
				av_free( my_pict );
			}

			free_frame( raw_frame );
			free_packet( packet );
		}
	}
}

void FeVideoImp::video_thread()
{
	const unsigned int MAX_QUEUE( 4 ), MIN_QUEUE( 0 );

	bool exhaust_queue( false );
	sf::Time max_sleep = time_base / (sf::Int64)2;

	int qscore( 100 ), qadjust( 10 ); // quality scoring
	int displayed( 0 ), discarded( 0 ), qscore_accum( 0 );

	std::queue<AVFrame *> frame_queue;

	AVPicture *my_pict = (AVPicture *)av_malloc( sizeof( AVPicture ) );
	avpicture_alloc( my_pict, PIX_FMT_RGBA,
							codec_ctx->width,
							codec_ctx->height );

	SwsContext *sws_ctx = sws_getContext(
					codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
					codec_ctx->width, codec_ctx->height, PIX_FMT_RGBA,
					SWS_FAST_BILINEAR, NULL, NULL, NULL );

	if ((!sws_ctx) || (!my_pict) )
	{
		std::cout << "error initializing video thread" << std::endl;
		goto the_end;
	}

	while ( run_video_thread )
	{
		bool do_process = true;
		bool discard_frames = false;

		//
		// First, display queued frames if they are coming up
		//
		if (( frame_queue.size() > MIN_QUEUE )
			|| ( exhaust_queue && !frame_queue.empty() ))
		{
			sf::Time wait_time = (sf::Int64)frame_queue.front()->pts * time_base
										- m_parent->get_video_time();

			if ( wait_time < max_sleep )
			{
				if ( wait_time < -time_base )

				{
					// If we are falling behind, we may need to start discarding
					// frames to catch up
					//
					qscore -= qadjust;
					set_avdiscard_from_qscore( codec_ctx, qscore );
					discard_frames = ( codec_ctx->skip_frame == AVDISCARD_ALL );
				}
				else if ( wait_time >= sf::Time::Zero )
				{
					if ( discard_frames )
					{
						//
						// Only stop discarding frames once we have caught up and are
						// time_base ahead of the desired presentation time
						//
						if ( wait_time >= time_base )
							discard_frames = false;
					}
					else
					{
						//
						// Otherwise, we are ahead and can sleep until presentation time
						//
						sf::sleep( wait_time );
					}
				}

				AVFrame *detached_frame = frame_queue.front();
				frame_queue.pop();

				qscore_accum += qscore;
				if ( discard_frames )
				{
					discarded++;
					continue;
				}

				sf::Lock l( image_swap_mutex );
				displayed++;

				sws_scale( sws_ctx, detached_frame->data, detached_frame->linesize,
							0, codec_ctx->height, my_pict->data,
							my_pict->linesize );

				display_frame = my_pict->data[0];
				free_frame( detached_frame );

				do_process = false;
			}
			//
			// if we didn't do anything above, then we go into the queue
			// management process below
			//
		}

		if ( do_process )
		{
			if ( frame_queue.size() < MAX_QUEUE )
			{
				//
				// get next packet
				//
				AVPacket *packet = pop_packet();
				if ( packet == NULL )
				{
					if ( !m_parent->end_of_file() )
						m_parent->read_packet();
					else if ( frame_queue.empty() )
						goto the_end;
					else
						exhaust_queue=true;
				}
				else
				{
					//
					// decompress packet and put it in our frame queue
					//
					int got_frame = 0;
					AVFrame *raw_frame = avcodec_alloc_frame();

					int len = avcodec_decode_video2( codec_ctx, raw_frame,
											&got_frame, packet );
					if ( len < 0 )
						std::cout << "error decoding video" << std::endl;

					if ( got_frame )
					{
						raw_frame->pts = raw_frame->pkt_pts;

						if ( raw_frame->pts == AV_NOPTS_VALUE )
							raw_frame->pts = packet->dts;

						frame_queue.push( raw_frame );
					}
					else
						free_frame( raw_frame );

					free_packet( packet );
				}
			}
			else
			{
				// Adjust our quality scoring, increasing it if it is down
				//
				if (( codec_ctx->skip_frame != AVDISCARD_DEFAULT )
						&& ( qadjust > 1 ))
					qadjust--;

				if ( qscore <= -100 ) // we stick at the lowest rate if we are actually discarding frames
					qscore = -100;
				else if ( qscore < 100 )
					qscore += qadjust;

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
	if (sws_ctx) sws_freeContext( sws_ctx );

	if ( my_pict )
	{
		sf::Lock l( image_swap_mutex );

		avpicture_free( my_pict );
		av_free( my_pict );
		display_frame=NULL;
	}

	while ( !frame_queue.empty() )
	{
		AVFrame *f=frame_queue.front();
		frame_queue.pop();

		if (f)
			free_frame( f );
	}

#ifdef FE_DEBUG

	int total_shown = displayed + discarded;
	int average = ( total_shown == 0 ) ? qscore_accum : ( qscore_accum / total_shown );

	std::cout << "End Video Thread - " << m_parent->m_format_ctx->filename << std::endl
				<< " - bit_rate=" << codec_ctx->bit_rate
				<< ", width=" << codec_ctx->width << ", height=" << codec_ctx->height << std::endl
				<< " - displayed=" << displayed << ", discarded=" << discarded << std::endl
				<< " - average qscore=" << average
				<< std::endl;
#endif
}

FeMedia::FeMedia( Type t )
	: sf::SoundStream(),
		m_type( t ),
		m_format_ctx( NULL ),
		m_loop( true ),
		m_read_eof( false ),
		m_audio( NULL ),
		m_video( NULL )
{
}

FeMedia::~FeMedia()
{
	close();
}

void FeMedia::init_av()
{
	static bool do_init=true;

	if ( do_init )
	{
		avcodec_register_all();
		av_register_all();

#ifndef FE_DEBUG
		av_log_set_level(AV_LOG_FATAL);
#endif

		do_init=false;
	}
}

sf::Texture *FeMedia::get_texture()
{
	if ( m_video )
		return &(m_video->display_texture);
	return NULL;
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

void FeMedia::stop()
{
	if ( m_audio )
	{
		sf::SoundStream::stop();
		m_audio->stop();

		av_seek_frame( m_format_ctx, m_audio->stream_id, 0,
							AVSEEK_FLAG_BACKWARD );

		avcodec_flush_buffers( m_audio->codec_ctx );
	}

	if ( m_video )
	{
		m_video->stop();

		av_seek_frame( m_format_ctx, m_video->stream_id, 0,
							AVSEEK_FLAG_BACKWARD );

		avcodec_flush_buffers( m_video->codec_ctx );
	}

	m_read_eof = false;
}

void FeMedia::close()
{
	stop();

	if (m_audio)
	{
		m_audio->close();
		delete m_audio;
		m_audio=NULL;
	}

	if (m_video)
	{
		m_video->close();
		delete m_video;
		m_video=NULL;
	}

	if (m_format_ctx)
	{
#if (LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT( 53, 17, 0 ))
		avformat_close_input( &m_format_ctx );
#else
		av_close_input_file( m_format_ctx );
#endif
		m_format_ctx=NULL;
	}

	m_read_eof=false;
}

bool FeMedia::is_playing()
{
	if ((m_video) && (!m_video->at_end))
		return true;

	return ((m_audio) && (sf::SoundStream::getStatus() == sf::SoundStream::Playing));
}

void FeMedia::setLoop( bool l )
{
	m_loop=l;
}

bool FeMedia::openFromFile( const std::string &name )
{
	close();
	init_av();

	if ( avformat_open_input( &m_format_ctx, name.c_str(), NULL, NULL ) < 0 )
	{
		std::cout << "Error opening input file: " << name << std::endl;
		return false;
	}

	if ( avformat_find_stream_info( m_format_ctx, NULL ) < 0 )
	{
		std::cout << "Error finding stream information in input file: "
					<< name << std::endl;
		return false;
	}

	if ( m_type & Audio )
	{
		int stream_id( -1 );
		AVCodec *dec;
		stream_id = av_find_best_stream( m_format_ctx, AVMEDIA_TYPE_AUDIO,
											-1, -1, &dec, 0 );

		if ( stream_id >= 0 )
		{
			m_format_ctx->streams[stream_id]->codec->request_sample_fmt = AV_SAMPLE_FMT_S16;

			if ( avcodec_open2( m_format_ctx->streams[stream_id]->codec,
										dec, NULL ) < 0 )
			{
				std::cout << "Could not open audio decoder for file: "
						<< name << std::endl;
			}
			else
			{
				m_audio = new FeAudioImp();
				m_audio->stream_id = stream_id;
				m_audio->codec_ctx = m_format_ctx->streams[stream_id]->codec;
				m_audio->codec = dec;

				//
				// TODO: Fix buffer sizing, we allocate way
				// more than we use
				//
				m_audio->buffer = (sf::Int16 *)av_malloc(
					MAX_AUDIO_FRAME_SIZE
					+ FF_INPUT_BUFFER_PADDING_SIZE
					+ m_audio->codec_ctx->sample_rate );

				sf::SoundStream::initialize(
					m_audio->codec_ctx->channels,
					m_audio->codec_ctx->sample_rate );

				sf::SoundStream::setLoop( false );

#ifndef DO_RESAMPLE
				if ( m_audio->codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16 )
				{
					std::cerr << "Warning: Attract-Mode was compiled without an audio resampler (libswresample or libavresample)." << std::endl
						<< "The audio format in " << name << " appears to need resampling.  It will likely sound like garbage." << std::endl;
				}
#endif
			}
		}
	}

	if ( m_type & Video )
	{
		int stream_id( -1 );
		AVCodec *dec;
		stream_id = av_find_best_stream( m_format_ctx, AVMEDIA_TYPE_VIDEO,
					-1, -1, &dec, 0 );

		if ( stream_id < 0 )
		{
			std::cout << "No video stream found, file: " << name << std::endl;
		}
		else
		{
			m_format_ctx->streams[stream_id]->codec->workaround_bugs = FF_BUG_AUTODETECT;

			if ( avcodec_open2( m_format_ctx->streams[stream_id]->codec,
										dec, NULL ) < 0 )
			{
				std::cout << "Could not open video decoder for file: "
						<< name << std::endl;
			}
			else
			{
				m_video = new FeVideoImp( this );
				m_video->stream_id = stream_id;
				m_video->codec_ctx = m_format_ctx->streams[stream_id]->codec;
				m_video->codec = dec;
				m_video->time_base = sf::seconds(
						av_q2d(m_format_ctx->streams[stream_id]->time_base) );

				m_video->display_texture = sf::Texture();
				m_video->display_texture.create( m_video->codec_ctx->width,
											m_video->codec_ctx->height );
				m_video->display_texture.setSmooth( true );
				m_video->preload();
			}
		}
	}

	if ( (!m_video) && (!m_audio) )
		return false;

	return true;
}

bool FeMedia::end_of_file()
{
	sf::Lock l(m_read_mutex);
	return ( m_read_eof );
}

bool FeMedia::read_packet()
{
	sf::Lock l(m_read_mutex);

	if ( m_read_eof )
		return false;

	AVPacket *pkt = (AVPacket *)av_malloc( sizeof( *pkt ) );

	int r = av_read_frame( m_format_ctx, pkt );
	if ( r < 0 )
	{
		m_read_eof=true;
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
		sf::Lock l( m_video->image_swap_mutex );
		if ( m_video->display_frame )
		{
			m_video->display_texture.update( m_video->display_frame );
			m_video->display_frame = NULL;
			return true;
		}
	}

	// restart if we are looping and done
	//
	if ( (m_loop) && (!is_playing()) )
	{
		stop();
		play();
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

#if (LIBAVCODEC_VERSION_INT < AV_VERSION_INT( 53, 25, 0 ))
		{
			sf::Lock l( m_audio->buffer_mutex );

			int bsize = MAX_AUDIO_FRAME_SIZE;
			if ( avcodec_decode_audio3(
						m_audio->codec_ctx,
						(m_audio->buffer + offset),
						&bsize, packet) < 0 )
			{
				std::cout << "Error decoding audio." << std::endl;
				FeBaseStream::free_packet( packet );
				return false;
			}
			else
			{
				offset += bsize / sizeof( sf::Int16 );
				data.sampleCount += bsize / sizeof(sf::Int16);
				data.samples = m_audio->buffer;
			}
		}
#else
		AVFrame *frame = avcodec_alloc_frame();

		//
		// TODO: avcodec_decode_audio4() can return multiple frames per packet depending on the codec.
		// We don't deal with this appropriately...
		//
		int got_frame( 0 );
		int len = avcodec_decode_audio4( m_audio->codec_ctx, frame, &got_frame, packet );
		if ( len < 0 )
		{
			std::cerr << "Error decoding audio." << std::endl;
			FeBaseStream::free_packet( packet );
			FeBaseStream::free_frame( frame );
			return false;
		}

		if ( got_frame )
		{
			int data_size = av_samples_get_buffer_size(
				NULL,
				m_audio->codec_ctx->channels,
				frame->nb_samples,
				m_audio->codec_ctx->sample_fmt, 1);

#ifdef DO_RESAMPLE
			if ( m_audio->codec_ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
#endif
			{
				sf::Lock l( m_audio->buffer_mutex );

				memcpy( (m_audio->buffer + offset), frame->data[0], data_size );
				offset += data_size / sizeof( sf::Int16 );
				data.sampleCount += data_size / sizeof(sf::Int16);
				data.samples = m_audio->buffer;
			}
#ifdef DO_RESAMPLE
			else
			{
				sf::Lock l( m_audio->buffer_mutex );

				if ( !m_audio->resample_ctx )
				{
					m_audio->resample_ctx = resample_alloc();
					if ( !m_audio->resample_ctx )
					{
						std::cerr << "Error allocating audio format converter." << std::endl;
						FeBaseStream::free_packet( packet );
						FeBaseStream::free_frame( frame );
						return false;
					}

					int64_t channel_layout = frame->channel_layout;
					if ( !channel_layout )
					{
						channel_layout = av_get_default_channel_layout(
								m_audio->codec_ctx->channels );
					}

					av_opt_set_int( m_audio->resample_ctx, "in_channel_layout", channel_layout, 0 );
					av_opt_set_int( m_audio->resample_ctx, "in_sample_fmt", frame->format, 0 );
					av_opt_set_int( m_audio->resample_ctx, "in_sample_rate", frame->sample_rate, 0 );
					av_opt_set_int( m_audio->resample_ctx, "out_channel_layout", channel_layout, 0 );
					av_opt_set_int( m_audio->resample_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0 );
					av_opt_set_int( m_audio->resample_ctx, "out_sample_rate", frame->sample_rate, 0 );

#ifdef FE_DEBUG
					std::cout << "Initializing resampler: in_sample_fmt="
						<< av_get_sample_fmt_name( (AVSampleFormat)frame->format )
						<< ", in_sample_rate=" << frame->sample_rate
						<< ", out_sample_fmt=" << av_get_sample_fmt_name( AV_SAMPLE_FMT_S16 )
						<< ", out_sample_rate=" << frame->sample_rate << std::endl;
#endif
					if ( resample_init( m_audio->resample_ctx ) < 0 )
					{
						std::cerr << "Error initializing audio format converter, input format="
							<< av_get_sample_fmt_name( (AVSampleFormat)frame->format )
							<< ", input sample rate=" << frame->sample_rate << std::endl;
						FeBaseStream::free_packet( packet );
						FeBaseStream::free_frame( frame );
						resample_free( &m_audio->resample_ctx );
						m_audio->resample_ctx = NULL;
						return false;
					}
				}
				if ( m_audio->resample_ctx )
				{
					int out_linesize;
					av_samples_get_buffer_size(
						&out_linesize,
						m_audio->codec_ctx->channels,
						frame->nb_samples,
						AV_SAMPLE_FMT_S16, 0 );

					uint8_t *tmp_ptr = (uint8_t *)(m_audio->buffer + offset);

#ifdef USE_SWRESAMPLE
					int out_samples = swr_convert(
								m_audio->resample_ctx,
								&tmp_ptr,
								frame->nb_samples,
								(const uint8_t **)frame->data,
								frame->nb_samples );
#else // USE_AVRESAMPLE
					int out_samples = avresample_convert(
								m_audio->resample_ctx,
								&tmp_ptr,
								out_linesize,
								frame->nb_samples,
								frame->data,
								frame->linesize[0],
								frame->nb_samples );
#endif
					if ( out_samples < 0 )
					{
						std::cerr << "Error performing audio conversion." << std::endl;
						FeBaseStream::free_packet( packet );
						FeBaseStream::free_frame( frame );
						break;
					}
					offset += out_samples * m_audio->codec_ctx->channels;
					data.sampleCount += out_samples * m_audio->codec_ctx->channels;
					data.samples = m_audio->buffer;
				}
			}
#endif
		}
		FeBaseStream::free_frame( frame );

#endif

		FeBaseStream::free_packet( packet );
	}

	return true;
}

void FeMedia::onSeek( sf::Time timeOffset )
{
	// Not implemented
}

bool FeMedia::is_supported_media_file( const std::string &filename )
{
	init_av();
	return ( av_guess_format(
					NULL,
					filename.c_str(),
					NULL ) != NULL ) ? true : false;
}
