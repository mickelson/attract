// gameswf_tools.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some optional helper code.


#include "base/tu_file.h"
#include "base/utility.h"
#include "base/zlib_adapter.h"
#include "gameswf/gameswf.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_stream.h"
#include "gameswf/gameswf_types.h"


#if TU_CONFIG_LINK_TO_ZLIB
//#include <zlib.h>	// for compress()
#endif // TU_CONFIG_LINK_TO_ZLIB


namespace gameswf { namespace tools
{
	// This struct tracks an input stream.  When you call
	// do_copy(), it writes all the data that has been read from
	// the input stream into the output stream.  (Basically it
	// goes by the input file position, not by the *actual* read
	// calls.)
	//
	// The copying can be optionally cancelled.
	struct copy_helper
	{
		tu_file*	m_in;
		tu_file*	m_out;
		int	m_initial_in_pos;
		bool	m_done_copy;

		copy_helper(tu_file* in, tu_file* out)
			:
			m_in(in),
			m_out(out),
			m_initial_in_pos(in->get_position()),
			m_done_copy(false)
		{
			assert(m_in && m_in->get_error() == TU_FILE_NO_ERROR);
			assert(m_out && m_out->get_error() == TU_FILE_NO_ERROR);
		}


		bool	do_copy()
		// Copy the data.  Return true on success, false on failure.
		{
			if (m_done_copy)
			{
				assert(0);
				log_error("gameswf::tools::copy_helper() already done copy\n");
				return false;
			}

			m_done_copy = true;

			int	current_in_pos = m_in->get_position();
			int	bytes_to_copy = current_in_pos - m_initial_in_pos;
			if (bytes_to_copy > 0)
			{
				m_in->set_position(m_initial_in_pos);
				int	bytes_copied = m_out->copy_bytes(m_in, bytes_to_copy);

				if (bytes_copied != bytes_to_copy)
				{
					m_in->set_position(current_in_pos);	// fixup
					return false;
				}
				assert(m_in->get_position() == current_in_pos);

				return true;
			}
			else
			{
				log_error("gameswf::tools::copy_helper asked to copy %d bytes\n",
					  bytes_to_copy);
				return false;
			}
		}
	};


	void	write_placeholder_bitmap(tu_file* out, Uint16 character_id)
	// Write a minimal bitmap character tag into the given stream,
	// with the given character_id.
	{
		out->write_le16((20 << 6) | 0x3F);	// tag header: tag type = 20, size = from next u32
		int	tag_size_pos = out->get_position();
		out->write_le32(0);	// placeholder for tag size.

		out->write_le16(character_id);
		out->write_byte(4);	// code for 16 bits/pixel
		out->write_le16(2);	// width, min pitch = 4 bytes/row
		out->write_le16(1);	// height

		// This is zlib-compressed data representing four 0 bytes.
		static const int	COMP_SIZE = 12;
		unsigned char	compressed_data[COMP_SIZE] =
		{
			0x78,
			0x9c,
			0x63,
			0x60,
			0x60,
			0x60,
			0x00,
			0x00,
			0x00,
			0x04,
			0x00,
			0x01,
		};
		out->write_bytes(compressed_data, COMP_SIZE);

// Here's some code to compute that at run-time.
#if 0
#ifdef TU_CONFIG_LINK_TO_ZLIB
		int	buffer_bytes = 4;	// width * height * bytes/pix
		unsigned char	buffer[4] = { 0, 0, 0, 0 };

		static const int	COMPBUFSIZE = 200;
		unsigned char	compressed_buffer[COMPBUFSIZE];

		// Deflate our little dummy bitmap.
		unsigned long	compressed_size = COMPBUFSIZE;
		int err = compress(compressed_buffer, &compressed_size, buffer, sizeof(buffer));
		if (err != Z_OK)
		{
			assert(0);	// There's no good reason for this to fail.
			log_error("write_placeholder_bitmap(): compress() failed.\n");
		}
		else
		{
			// Dump the compressed data into the output.
			out->write_bytes(compressed_buffer, compressed_size);
		}
#endif // TU_CONFIG_LINK_TO_ZLIB
#endif // 0

		// Write the actual tag size in the slot at the beginning.
		int	end_pos = out->get_position();
		int	size = end_pos - tag_size_pos - 4;
		out->set_position(tag_size_pos);
		out->write_le32(size);
		out->set_position(end_pos);
	}

}}	// end namespace gameswf::tools


int	gameswf::tools::process_swf(tu_file* swf_out, tu_file* in, const process_options& options)
{
	assert(in && in->get_error() == TU_FILE_NO_ERROR);
	assert(swf_out && swf_out->get_error() == TU_FILE_NO_ERROR);

	// @@ Copied & adapted from movie_def_impl::read()
	// @@ TODO share this wrapper code somehow (also with gameswf_parser)

	Uint32	file_start_pos = in->get_position();
	Uint32	header = in->read_le32();
	Uint32	file_length = in->read_le32();
	Uint32	file_end_pos = file_start_pos + file_length;

	int	version = (header >> 24) & 255;
	if ((header & 0x0FFFFFF) != 0x00535746
	    && (header & 0x0FFFFFF) != 0x00535743)
	{
		// ERROR
		log_error("gameswf::movie_def_impl::read() -- file does not start with a SWF header!\n");
		return 1;
	}
	bool	compressed = (header & 255) == 'C';

	IF_VERBOSE_PARSE(log_msg("version = %d, file_length = %d\n", version, file_length));

	tu_file*	original_in = NULL;
	if (compressed)
	{
#if TU_CONFIG_LINK_TO_ZLIB == 0
		log_error("gameswf can't read zipped SWF data; TU_CONFIG_LINK_TO_ZLIB is 0!\n");
		return -1;
#endif

		IF_VERBOSE_PARSE(log_msg("file is compressed.\n"));
		original_in = in;

		// Uncompress the input as we read it.
		in = zlib_adapter::make_inflater(original_in);

		// Subtract the size of the 8-byte header, since
		// it's not included in the compressed
		// stream length.
		file_end_pos = file_length - 8;
	}

	stream	str(in);

	if (options.m_zip_whole_file)
	{
		// @@ TODO not implemented yet.
		log_error("gameswf::tools::process_swf(): options.m_zip_whole_file is not implemented!  Output will not be zipped.\n");
	}

	//
	// Start the output file
	//

	int	output_file_start_pos = swf_out->get_position();
	swf_out->write_le32(0x06535746);	// Flash 6 header, uncompressed

	// File length (need to overwrite later with the actual value.
	int	output_file_length_pos = swf_out->get_position();
	swf_out->write_le32(0);

	float	frame_rate = 30.f;
	int	frame_count = 0;
	{
		copy_helper	cp(in, swf_out);	// copies everything that's read in this scope.

		rect	dummy_frame_size;
		dummy_frame_size.read(&str);
		frame_rate = str.read_u16() / 256.0f;
		frame_count = str.read_u16();

		str.align();

		bool	success = cp.do_copy();
		if (!success)
		{
			// Error!
			log_error("gameswf::tools::process_swf() -- unable to copy header data!\n");
			return 1;
		}
	}

//	m_playlist.resize(m_frame_count);

//	IF_VERBOSE_PARSE(m_frame_size.print());
	IF_VERBOSE_PARSE(log_msg("frame rate = %f, frames = %d\n", frame_rate, frame_count));

	while ((Uint32) str.get_position() < file_end_pos)
	{
		copy_helper	cp(in, swf_out);

		int	tag_type = str.open_tag();
		if (options.m_remove_image_data
		    && tag_type == 8)
		{
			// Don't need no stinkin jpeg tables.
			str.close_tag();
		}
		else if (options.m_remove_image_data
			 && (tag_type == 6
			     || tag_type == 20
			     || tag_type == 21
			     || tag_type == 35
			     || tag_type == 36))
		{
			// Some type of bitmap character tag; replace it with a minimal stand-in.
			Uint16	cid = str.read_u16();
			str.close_tag();

			// Insert substitute tag.
			write_placeholder_bitmap(swf_out, cid);
		}
		else
		{
			// Leave this tag as-is.
			str.close_tag();
			str.align();

			// Copy into output.
			bool	success = cp.do_copy();
			if (!success)
			{
				// Error!
				log_error("gameswf::tools::process_swf() -- error copying tag!\n");
				return 1;
			}
		}

		if (tag_type == 0)
		{
			if ((unsigned int) str.get_position() != file_end_pos)
			{
				// Safety break, so we don't read past the end of the
				// movie.
				log_msg("warning: process_swf() hit stream-end tag, but not at the "
					"end of the file yet; stopping for safety\n");
				break;
			}
		}
	}

	if (original_in)
	{
		// Done with the zlib_adapter.
		delete in;
	}
	
	// Go back and write the file size.
	int	current_pos = swf_out->get_position();
	swf_out->set_position(output_file_length_pos);
	swf_out->write_le32(current_pos - output_file_start_pos);
	swf_out->set_position(current_pos);

	return 0;	// OK
}



// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
