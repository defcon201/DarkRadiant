#include "PNGLoader.h"

#include <png.h>
#include <cstring>
#include "RGBAImage.h"
#include "stream/ScopedArchiveBuffer.h"

typedef unsigned char byte;

namespace image
{

#define png_infopp_NULL (png_infopp)nullptr
#define int_p_NULL (int*)nullptr

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	rError() << "libpng warning: " << warning_msg << std::endl;
}

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	rError() << "libpng error: " << error_msg << std::endl;
	longjmp(png_jmpbuf(png_ptr), 1);
}

void user_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
	png_bytep* p_p_fbuffer = (png_bytep*)png_get_io_ptr(png_ptr);
	std::memcpy(data, *p_p_fbuffer, length);
	*p_p_fbuffer += length;
}

RGBAImagePtr LoadPNGBuff(unsigned char* fbuffer)
{
	png_byte** row_pointers;
	png_bytep p_fbuffer;

	p_fbuffer = fbuffer;

	// the reading glue
	// http://www.libpng.org/pub/png/libpng-manual.html

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp)NULL, user_error_fn, user_warning_fn);

	if (!png_ptr)
	{
		rError() << "libpng error: png_create_read_struct\n";
		return RGBAImagePtr();
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		rError() << "libpng error: png_create_info_struct (info_ptr)" << std::endl;
		return RGBAImagePtr();
	}

	png_infop end_info = png_create_info_struct(png_ptr);

	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

		rError() << "libpng error: png_create_info_struct (end_info)" << std::endl;
		return RGBAImagePtr();
	}

	// configure the read function
	png_set_read_fn(png_ptr, (png_voidp)&p_fbuffer, (png_rw_ptr)&user_read_data);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return RGBAImagePtr();
	}

	png_read_info(png_ptr, info_ptr);

	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);

	// we want to treat all images the same way
	//   The following code transforms grayscale images of less than 8 to 8 bits,
	//   changes paletted images to RGB, and adds a full alpha channel if there is
	//   transparency information in a tRNS chunk.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
#if PNG_LIBPNG_VER < 10400
		png_set_gray_1_2_4_to_8(png_ptr);
#else
		png_set_expand_gray_1_2_4_to_8(png_ptr);
#endif 
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}

	if (!(color_type & PNG_COLOR_MASK_ALPHA))
	{
		// Set the background color to draw transparent and alpha images over.
		png_color_16 my_background, * image_background;

		if (png_get_bKGD(png_ptr, info_ptr, &image_background))
		{
			png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
		}
		else
		{
			png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}

		// Add alpha byte after each RGB triplet
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}

	// read the sucker in one chunk
	png_read_update_info(png_ptr, info_ptr);

	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);

	// allocate the pixel buffer, and the row pointers
	RGBAImagePtr image(new RGBAImage(width, height));

	row_pointers = (png_byte**)malloc((height) * sizeof(png_byte*));

	for (int i = 0; i < height; i++)
	{
		row_pointers[i] = (png_byte*)(image->getPixels()) + i * 4 * (width);
	}

	// actual read
	png_read_image(png_ptr, row_pointers);

	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* free up the memory structure */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	free(row_pointers);

	return image;
}

ImagePtr PNGLoader::load(ArchiveFile& file) const
{
    archive::ScopedArchiveBuffer buffer(file);

    return LoadPNGBuff(buffer.buffer);
}

ImageTypeLoader::Extensions PNGLoader::getExtensions() const
{
    Extensions extensions;
    extensions.push_back("png");
    return extensions;
}

}
