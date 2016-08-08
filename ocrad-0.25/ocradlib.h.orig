/*  Ocradlib - Optical Character Recognition library
    Copyright (C) 2009-2015 Antonio Diaz Diaz.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef __cplusplus
extern "C" {
#endif

const char * const OCRAD_version_string = "0.25";


/* OCRAD_Pixmap.data is a pointer to image data formed by "height" rows
   of "width" pixels each.
   The format for each pixel depends on mode like this:
   OCRAD_bitmap   --> 1 byte  per pixel;  0 = white, 1 = black
   OCRAD_greymap  --> 1 byte  per pixel;  256 level greymap (0 = black)
   OCRAD_colormap --> 3 bytes per pixel;  16777216 colors RGB (0,0,0 = black) */

enum OCRAD_Pixmap_Mode { OCRAD_bitmap, OCRAD_greymap, OCRAD_colormap };

struct OCRAD_Pixmap
  {
  const unsigned char * data;
  int height;
  int width;
  enum OCRAD_Pixmap_Mode mode;
  };


enum OCRAD_Errno { OCRAD_ok = 0, OCRAD_bad_argument, OCRAD_mem_error,
                   OCRAD_sequence_error, OCRAD_library_error };

struct OCRAD_Descriptor;


const char * OCRAD_version( void );


/*--------------------- Functions ---------------------*/

struct OCRAD_Descriptor * OCRAD_open( void );

int OCRAD_close( struct OCRAD_Descriptor * const ocrdes );

enum OCRAD_Errno OCRAD_get_errno( struct OCRAD_Descriptor * const ocrdes );

int OCRAD_set_image( struct OCRAD_Descriptor * const ocrdes,
                     const struct OCRAD_Pixmap * const image,
                     const bool invert );

int OCRAD_set_image_from_file( struct OCRAD_Descriptor * const ocrdes,
                               const char * const filename,
                               const bool invert );

int OCRAD_set_utf8_format( struct OCRAD_Descriptor * const ocrdes,
                           const bool utf8 );		// 0 = byte, 1 = utf8

int OCRAD_set_threshold( struct OCRAD_Descriptor * const ocrdes,
                         const int threshold );		// 0..255, -1 = auto

int OCRAD_scale( struct OCRAD_Descriptor * const ocrdes, const int value );

int OCRAD_recognize( struct OCRAD_Descriptor * const ocrdes,
                     const bool layout );

int OCRAD_result_blocks( struct OCRAD_Descriptor * const ocrdes );

int OCRAD_result_lines( struct OCRAD_Descriptor * const ocrdes,
                        const int blocknum );		// 0..blocks-1

int OCRAD_result_chars_total( struct OCRAD_Descriptor * const ocrdes );

int OCRAD_result_chars_block( struct OCRAD_Descriptor * const ocrdes,
                              const int blocknum );	// 0..blocks-1

int OCRAD_result_chars_line( struct OCRAD_Descriptor * const ocrdes,
                             const int blocknum,	// 0..blocks-1
                             const int linenum );	// 0..lines(block)-1

const char * OCRAD_result_line( struct OCRAD_Descriptor * const ocrdes,
                                const int blocknum,	// 0..blocks-1
                                const int linenum );	// 0..lines(block)-1

int OCRAD_result_first_character( struct OCRAD_Descriptor * const ocrdes );

#ifdef __cplusplus
}
#endif
