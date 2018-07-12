/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2017 Andrew Mickelson
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

extern "C"
{
#include <libavutil/hwcontext_d3d11va.h>
}

enum AVPixelFormat get_format_d3d11( AVCodecContext *codec_ctx, const enum AVPixelFormat *pix_fmts )
{
	const enum AVPixelFormat *p;

	for ( p = pix_fmts; *p != -1; p++ )
	{
		if ( *p == AV_PIX_FMT_D3D11 )
		{
			AVBufferRef *device_ctx=NULL;
			int ret = av_hwdevice_ctx_create( &device_ctx, AV_HWDEVICE_TYPE_D3D11VA, "", NULL, 0 );

			if ( ret < 0 )
			{
				FeLog() << "Error creating d3d11va hardware device context" << std::endl;
				continue;
			}

			AVBufferRef *frames_ctx = av_hwframe_ctx_alloc( device_ctx );

			if ( !frames_ctx )
			{
				av_buffer_unref( &device_ctx );
				continue;
			}

			AVHWFramesContext *fc = (AVHWFramesContext *)frames_ctx->data;
			AVD3D11VAFramesContext *f_hwc = (AVD3D11VAFramesContext *)fc->hwctx;

			fc->pool = NULL;
			fc->format = AV_PIX_FMT_D3D11;
			fc->sw_format = AV_PIX_FMT_NV12;
			fc->initial_pool_size = 2;

			f_hwc->texture = NULL;
			f_hwc->BindFlags = D3D11_BIND_DECODER;

			float aspect_ratio = 1.0;
			if ( codec_ctx->sample_aspect_ratio.num != 0 )
				aspect_ratio = av_q2d( codec_ctx->sample_aspect_ratio );

			fc->width = codec_ctx->width * aspect_ratio;
			fc->height = codec_ctx->height;

			ret = av_hwframe_ctx_init( frames_ctx );

			if ( ret < 0 )
			{
				FeLog() << "Error initializing d3d11va hardware frame context" << std::endl;
				av_buffer_unref( &device_ctx );
				av_buffer_unref( &frames_ctx );
				continue;
			}

			av_buffer_unref( &device_ctx );
			codec_ctx->hw_frames_ctx = frames_ctx;
			break;
		}
	}

	return *p;
}
