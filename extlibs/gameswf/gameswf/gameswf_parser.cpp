// gameswf_parser.cpp	-- Willem Kokke 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A program to parse SWF files into text output.

#include "base/tu_file.h"
#include "base/zlib_adapter.h"
#include "base/image.h"
#include "base/jpeg.h"

#include "gameswf/gameswf_stream.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf.h"

#define TWIPS_TO_PIXELS(x) ((x) / 20.f)
#define PIXELS_TO_TWIPS(x) ((x) * 20.f)

using gameswf::stream;
using gameswf::log_msg;

namespace parser
{
	static int ident = 0;
	static int current_frame = 0;
	tu_file* out;

	typedef void (*loader_function)(stream* input, int tag_type);
	static hash<int, loader_function> tag_loaders;

	void register_tag_loader(int tag_type, loader_function lf)
	{
		assert(tag_loaders.get(tag_type, NULL) == false);
		assert(lf != NULL);
		tag_loaders.add(tag_type, lf);
	}

	void log_callback(bool error, const char* message)
	{
		static char buf[512];

		strcpy(buf,"");

		for (int i=0; i<ident; i++)
		{
			strcat(buf,"	");
		}
		strcat(buf,message);
		//out->write_string(buf);
		fputs(buf,stdout);
	}

	// parse a matrix
	struct matrix
	{
		static float m_[2][3];
		static bool has_scale, has_rotate;
		static void parse(stream* in)
		{
			in->align();

			memset(&m_[0], 0, sizeof(m_));
			m_[0][0] = 1;
			m_[1][1] = 1;

			int	has_scale = in->read_uint(1);
			if (has_scale)
			{
				int	scale_nbits = in->read_uint(5);
				m_[0][0] = in->read_sint(scale_nbits) / 65536.0f;
				m_[1][1] = in->read_sint(scale_nbits) / 65536.0f;
			}
			int	has_rotate = in->read_uint(1);
			if (has_rotate)
			{
				int	rotate_nbits = in->read_uint(5);
				m_[1][0] = in->read_sint(rotate_nbits) / 65536.0f;
				m_[0][1] = in->read_sint(rotate_nbits) / 65536.0f;
			}

			int	translate_nbits = in->read_uint(5);
			if (translate_nbits > 0)
			{
				m_[0][2] = (float) in->read_sint(translate_nbits);
				m_[1][2] = (float) in->read_sint(translate_nbits);
			}
		}
		static void write()
		{
			ident++;
			log_msg("has_scale = %d, has_rotate = %d\n", has_scale, has_rotate);
			log_msg("| %4.4f %4.4f %4.4f |\n", m_[0][0], m_[0][1], TWIPS_TO_PIXELS(m_[0][2]));
			log_msg("| %4.4f %4.4f %4.4f |\n", m_[1][0], m_[1][1], TWIPS_TO_PIXELS(m_[1][2]));
			ident--;
		}

	};
	float matrix::m_[2][3];
	bool matrix::has_scale, matrix::has_rotate;

	struct rect
	{
		static Uint32 x_min,x_max,y_min,y_max;
		static void parse(stream* in)
		{
			in->align();
			int	nbits = in->read_uint(5);
			x_min = in->read_sint(nbits);
			x_max = in->read_sint(nbits);
			y_min = in->read_sint(nbits);
			y_max = in->read_sint(nbits);
		}
		static void write()
		{
			ident++;
			log_msg("x_min: %i, x_max: %i,	width: %i twips, %4.0f pixels\n", x_min, x_max, x_max - x_min, TWIPS_TO_PIXELS(x_max - x_min));
			log_msg("y_min: %i, y_max: %i, height: %i twips, %4.0f pixels\n", y_min, y_max, y_max - y_min, TWIPS_TO_PIXELS(y_max - y_min));
			ident--;
		}
	};
	Uint32 rect::x_min;
	Uint32 rect::y_min;
	Uint32 rect::x_max;
	Uint32 rect::y_max;

	struct rgb
	{
		static Uint8 m_r, m_g, m_b;
		static void parse(stream* in)
		{
			m_r = in->read_u8();
			m_g = in->read_u8();
			m_b = in->read_u8();
		}
		static void write()
		{
			ident++;
			log_msg("rgb: %d %d %d \n", m_r, m_g, m_b);
			ident--;
		}
	};
	Uint8 rgb::m_r;
	Uint8 rgb::m_g;
	Uint8 rgb::m_b;

	struct rgba
	{
		static Uint8 m_r, m_g, m_b, m_a;
		static void parse(stream* in)
		{
			m_r = in->read_u8();
			m_g = in->read_u8();
			m_b = in->read_u8();
			m_a = in->read_u8();
		}
		static void write()
		{
			ident++;
			log_msg("rgba: %d %d %d %d\n", m_r, m_g, m_b, m_a);
			ident--;
		}
	};
	Uint8 rgba::m_r;
	Uint8 rgba::m_g;
	Uint8 rgba::m_b;
	Uint8 rgba::m_a;

	struct cxform
	{
		static float m_[4][2];
		static bool has_add, has_mult;

		static void parse_rgb(stream* in)
		{
			in->align();

			int	has_add = in->read_uint(1);
			int	has_mult = in->read_uint(1);
			int	nbits = in->read_uint(4);

			if (has_mult) {
				m_[0][0] = in->read_sint(nbits) / 255.0f;
				m_[1][0] = in->read_sint(nbits) / 255.0f;
				m_[2][0] = in->read_sint(nbits) / 255.0f;
				m_[3][0] = 1;
			}
			else {
				for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
			}
			if (has_add) {
				m_[0][1] = (float) in->read_sint(nbits);
				m_[1][1] = (float) in->read_sint(nbits);
				m_[2][1] = (float) in->read_sint(nbits);
				m_[3][1] = 1;
			}
			else {
				for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
			}
		}
		static void parse_rgba(stream* in)
		{
			in->align();

			int	has_add = in->read_uint(1);
			int	has_mult = in->read_uint(1);
			int	nbits = in->read_uint(4);

			if (has_mult) {
				m_[0][0] = in->read_sint(nbits) / 255.0f;
				m_[1][0] = in->read_sint(nbits) / 255.0f;
				m_[2][0] = in->read_sint(nbits) / 255.0f;
				m_[3][0] = in->read_sint(nbits) / 255.0f;
			}
			else {
				for (int i = 0; i < 4; i++) { m_[i][0] = 1; }
			}
			if (has_add) {
				m_[0][1] = (float) in->read_sint(nbits);
				m_[1][1] = (float) in->read_sint(nbits);
				m_[2][1] = (float) in->read_sint(nbits);
				m_[3][1] = (float) in->read_sint(nbits);
			}
			else {
				for (int i = 0; i < 4; i++) { m_[i][1] = 0; }
			}
		}
		static void write()
		{
			ident++;
			log_msg("cxform:\n");
			log_msg("has_add = %d, has_mult = %d\n", has_add, has_mult);
			log_msg("| %4.4f %4.4f |\n", m_[0][0], m_[0][1]);
			log_msg("| %4.4f %4.4f |\n", m_[1][0], m_[1][1]);
			log_msg("| %4.4f %4.4f |\n", m_[2][0], m_[2][1]);
			log_msg("| %4.4f %4.4f |\n", m_[3][0], m_[3][1]);
			ident--;
		}
	};
	float cxform::m_[4][2];
	bool cxform::has_add;
	bool cxform::has_mult;

	// tag 0
	void parse_end_movie(stream* input, int tag_type)
	{
		assert(tag_type == 0);
		ident--;
		log_msg("\n");
		log_msg("Movie ended\n\n");
	}

	// tag 1
	void parse_show_frame(stream* input, int tag_type)
	{
		assert(tag_type == 1);
		ident--;
		current_frame++;
		log_msg("\n");
		log_msg("show frame %i\n\n", current_frame);
		ident++;
	}

	// tag 2, 22, 32
	void parse_define_shape123(stream* input, int tag_type)
	{
		assert(tag_type == 2 || tag_type == 22 || tag_type == 32);
		if(tag_type == 2)
			log_msg("define_shape:\n");
		if(tag_type == 22)
			log_msg("define_shape2:\n");
		if(tag_type == 32)
			log_msg("define_shape3:\n");
		ident++;
		log_msg("character ID: %i\n", input->read_u16());
		ident--;
	}

	// tag 4, 26
	void parse_place_object12(stream* input, int tag_type)
	{
		assert(tag_type == 4 || tag_type == 26);
		
		if (tag_type == 4)
		{
			log_msg("place_object:\n");
			ident++;
			log_msg("character ID: %i\n",input->read_u16());
			log_msg("depth: %i\n",input->read_u16());
			log_msg("matrix:\n");
			matrix::parse(input);
			matrix::write();
			
			if (input->get_position() < input->get_tag_end_position())
			{
				log_msg("color transform:\n");
				cxform::parse_rgb(input);
				cxform::write();
			}
			ident--;
		}
		else if (tag_type == 26)
		{
			input->align();
			
			log_msg("place_object2:\n");
			ident++;
			
			bool	has_actions = input->read_uint(1) ? true : false;
			bool	has_clip_depth = input->read_uint(1) ? true : false;
			bool	has_name = input->read_uint(1) ? true : false;
			bool	has_ratio = input->read_uint(1) ? true : false;
			bool	has_cxform = input->read_uint(1) ? true : false;
			bool	has_matrix = input->read_uint(1) ? true : false;
			bool	has_char = input->read_uint(1) ? true : false;
			bool	flag_move = input->read_uint(1) ? true : false;

			UNUSED(has_actions);

			log_msg("depth: %i\n",input->read_u16());
			
			if (has_char) {
				log_msg("character ID: %i\n",input->read_u16());
			}
			if (has_matrix) {
				log_msg("matrix:\n");
				matrix::parse(input);
				matrix::write();
			}
			if (has_cxform) {
				log_msg("color transform:");
				cxform::parse_rgba(input);
				cxform::write();
			}			
			if (has_ratio) {
				log_msg("ratio: %i\n",input->read_u16());
			}			
			if (has_name) {
				tu_string name;
				input->read_string(&name);
				log_msg("name: %s\n",name.c_str());
			}
			if (has_clip_depth) {
				log_msg("clipdepth: %i\n",input->read_u16());
			}			
			if (has_clip_depth) {
				log_msg("has_actions: to be implemented\n");
			}
			
			if (has_char == true && flag_move == true)
			{
				log_msg("replacing a character previously at this depth\n");
			}
			else if (has_char == false && flag_move == true)
			{
				log_msg("moving a character previously at this depth\n");
			}
			else if (has_char == true && flag_move == false)
			{
				log_msg("placing a character first time at this depth\n");
			}
			
			ident--;
		}
	}

	// tag 5, 28
	void parse_remove_object12(stream* input, int tag_type)
	{
		assert(tag_type == 5 || tag_type == 28);
		if (tag_type==5)
		{
			log_msg("remove_object\n");
			ident++;
			log_msg("character ID: %i\n", input->read_u16());
			log_msg("depth: %i\n", input->read_u16());
			ident--;
		}
		if (tag_type==28)
		{
			log_msg("remove_object_2\n");
			ident++;
			log_msg("depth: %i\n", input->read_u16());
			ident--;
		}
	}

	// tag 46
	void parse_define_shape_morph(stream *input, int tag_type)
	{
		assert(tag_type == 46);
		log_msg("define_shape_morph\n");
		ident++;
		log_msg("character ID: %i\n", input->read_u16());
		ident--;
	}

	// tag 6
	void parse_define_bits(stream* input, int tag_type)
	{
		assert(tag_type==6);
		log_msg("define jpeg bits\n");
		ident++;
		log_msg("character ID: %i\n", input->read_u16());
		ident--;
	}

	void parse_jpeg_tables(stream* input, int tag_type)
	{
		assert(tag_type==8);
		log_msg("define jpeg table\n\n");
	}	
	
	void parse_set_background_color(stream* input, int tag_type)
	{
		assert(tag_type==9);
		rgb::parse(input);
		log_msg("set background color to:\n");
		rgb::write();		
	}

	void parse_do_action(stream* input, int tag_type)
	{
		assert(tag_type==12);
		log_msg("do action:\n");
		ident++;
		log_msg("to be implemented\n");
		ident--;
	}		

	void parse_define_sprite(stream* input, int tag_type)
	{
		assert(tag_type==39);
		log_msg("define a new sprite:\n");
		ident++;
		int	tag_end = input->get_tag_end_position();
		Uint32 char_id = input->read_u16();
		Uint32 sprite_frame_count = input->read_u16();
		log_msg("character ID: %i\n", char_id);
		log_msg("frame count of sprite: %i\n", sprite_frame_count);
		Uint32 old_current_frame = current_frame;
		current_frame = 0;

		ident++;
		log_msg("\n");		
		log_msg("starting frame 0\n\n");
		ident++;

		while ((Uint32) input->get_position() < (Uint32) tag_end)
		{
			int	tag_type = input->open_tag();
			loader_function lf = NULL;

			if (tag_type == 0)
			{
				ident--;
				ident--;
				ident--;
				log_msg("end of sprite definition\n\n");
			}
			else if (tag_loaders.get(tag_type, &lf))
			{
				(*lf)(input, tag_type);
			}
			else
			{
				log_msg("warning: no tag loader for tag_type %d\n", tag_type);
			}
			input->close_tag();
		}
		current_frame = old_current_frame;
	}	
	
	void parse_set_framelabel(stream* input, int tag_type)
	{
		assert(tag_type==43);
		log_msg("current framelabel:\n");
		ident++;
		tu_string str;
		input->read_string(&str);
		log_msg("%s\n",str.c_str());
		delete str;

		if (input->get_position() < input->get_tag_end_position())
		{
			//TODOm_color_transform.read_rgb(in);
		}
		ident--;
	}	
	
	void parse_swf(tu_file* file)
	{
		register_tag_loader(0,parse_end_movie);		
		register_tag_loader(1,parse_show_frame);
		register_tag_loader(2,parse_define_shape123);
		register_tag_loader(4,parse_place_object12);
		register_tag_loader(5,parse_remove_object12);
		register_tag_loader(6,parse_define_bits);			
		register_tag_loader(8,parse_jpeg_tables);				
		register_tag_loader(9,parse_set_background_color);
		register_tag_loader(12,parse_do_action);	
		register_tag_loader(22,parse_define_shape123);
		register_tag_loader(26,parse_place_object12);
		register_tag_loader(28,parse_remove_object12);
		register_tag_loader(32,parse_define_shape123);
		register_tag_loader(39,parse_define_sprite);	
		register_tag_loader(43,parse_set_framelabel);		
		register_tag_loader(46,parse_define_shape_morph);

		ident = 1;

		Uint32 header = file->read_le32();
		Uint32 file_length = file->read_le32();

		Uint32 version = (header >> 24) & 255;
		if ((header & 0x0FFFFFF) != 0x00535746 && (header & 0x0FFFFFF) != 0x00535743)
		{
			log_msg("\nNo valid SWF file, header is incorrect!\n");
			return;
		}

		bool compressed = (header & 255) == 'C';

		tu_file* original_file = NULL;

		log_msg("\nSWF version %i, file length = %i bytes\n", version, file_length);

		if (compressed)
		{
			log_msg("file is compressed.\n");
			original_file = file;
			file = zlib_adapter::make_inflater(original_file);
			file_length -= 8;
		}

		stream str(file);

		rect::parse(&str);
		float frame_rate = str.read_u16() / 256.0f;
		int frame_count = str.read_u16();

		log_msg("viewport:\n");
		rect::write();
		log_msg("frame rate = %f, number of frames = %d\n", frame_rate, frame_count);

		log_msg("\n");
		log_msg("starting frame 0\n\n");
		ident++;

		while ((Uint32) str.get_position() < file_length)
		{
			int	tag_type = str.open_tag();

			loader_function	lf = NULL;

			if (tag_loaders.get(tag_type, &lf))
			{
				(*lf)(&str, tag_type);

			} else
			{
				log_msg("warning: no tag loader for tag_type %d\n", tag_type);
			}

			str.close_tag();

			if (tag_type == 0)
			{
				if ((unsigned int)str.get_position() != file_length)
				{
					log_msg("warning: end of file tag found, while not at the end of the file, aborting\n");
					break;
				}
			}
		}

		if (out)
		{
			delete out;
		}
		if (original_file)
		{
			delete file;
		}
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	const char* infile = argv[1];
	gameswf::register_log_callback(parser::log_callback);

	if (infile == NULL)
	{
		log_msg("pass a filename as a parameter");
		exit(1);
	}

	tu_file*	in = new tu_file(infile, "rb");
	if (in->get_error())
	{
		log_msg("can't open '%s' for input\n", infile);
		delete in;
		exit(1);
	}
	parser::parse_swf(in);
	return(0);
}

// Local Variables:
// mode: C++
// c-basic-offset: 8
// tab-width: 8
// indent-tabs-mode: t
// End:
