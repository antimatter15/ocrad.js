/*  GNU Ocrad - Optical Character Recognition program
    Copyright (C) 2003-2015 Antonio Diaz Diaz.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

extern int verbosity;


namespace Ocrad {

void internal_error( const char * const msg );
bool similar( const int a, const int b,
              const int percent_dif, const int abs_dif = 1 );

} // end namespace Ocrad


class Charset
  {
  int charset_;

public:
  enum Value { ascii = 1, iso_8859_9 = 2, iso_8859_15 = 4 };

  Charset() : charset_( 0 ) {}
  bool enable( const char * const name );
  bool enabled( const Value cset ) const;
  bool only( const Value cset ) const;
  void show_error( const char * const program_name,
                   const char * const arg ) const;
  };


class Transformation
  {
public:
  enum Type { none, rotate90, rotate180, rotate270,
              mirror_lr, mirror_tb, mirror_d1, mirror_d2 };
private:
  Type type_;

public:
  Transformation() : type_( none ) {}
  bool set( const char * const name );
  Type type() const { return type_; }
  void show_error( const char * const program_name,
                   const char * const arg ) const;
  };


class User_filter;

struct Filter
  {
  enum Type { letters, letters_only, numbers, numbers_only, same_height,
              text_block, upper_num, upper_num_mark, upper_num_only, user };

  const User_filter * user_filterp;
  Type type;

  explicit Filter( const User_filter * p )
    : user_filterp( p ), type( user ) {}
  explicit Filter( const Type t )
    : user_filterp( 0 ), type( t ) {}
  };


struct Control
  {
  Charset charset;
  std::vector< Filter > filters;
  FILE * outfile, * exportfile;
  int debug_level;
  char filetype;
  bool utf8;

  Control()
    : outfile( stdout ), exportfile( 0 ),
      debug_level( 0 ), filetype( '4' ), utf8( false ) {}
  ~Control();

  bool add_filter( const char * const program_name, const char * const name );
  int add_user_filter( const char * const program_name,
                       const char * const file_name );
  bool set_format( const char * const name );
  };
