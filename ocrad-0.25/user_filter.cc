/*  GNU Ocrad - Optical Character Recognition program
    Copyright (C) 2014, 2015 Antonio Diaz Diaz.

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

#include <cstdio>
#include <string>
#include <vector>

#include "common.h"
#include "iso_8859.h"
#include "ucs.h"
#include "user_filter.h"


namespace {

class Set
  {
  std::vector< unsigned char > data;		// faster than bool
  int parsed_len_;
  bool in;

public:
  explicit Set( const std::string & regex, const unsigned i0 = 0 );
  bool includes( const unsigned char ch ) const
    { return ( parsed_len_ > 0 && in == data[ch] ); }
  int parsed_len() const { return parsed_len_; }
  };


Set::Set( const std::string & regex, const unsigned i0 )
  : data( 256, false ), parsed_len_( 0 ), in( true )
  {
  if( i0 + 2 >= regex.size() || regex[i0] != '[' ) return;
  unsigned i = i0 + 1;
  bool fail = true;
  if( regex[i] == '^' ) { ++i; in = false; }
  if( regex[i] == ']' ) data[regex[i++]] = true;

  for( ; i < regex.size(); ++i )
    {
    unsigned char ch1 = regex[i];
    if( ch1 == ']' ) { ++i; fail = false; break; }
    if( ch1 == '\\' )
      {
      int len, cht = ISO_8859::escape( regex, i + 1, &len );
      if( cht < 0 ) break;
      ch1 = cht; i += len;
      }
    if( i + 2 >= regex.size() || regex[i+1] != '-' || regex[i+2] == ']' )
      data[ch1] = true;
    else					// range
      {
      i += 2;
      unsigned char ch2 = regex[i];
      if( ch2 == '\\' )
        {
        int len, cht = ISO_8859::escape( regex, i + 1, &len );
        if( cht < 0 ) break;
        ch2 = cht; i += len;
        }
      for( int c = ch1; c <= ch2; ++c ) data[c] = true;
      }
    }
  if( !fail ) parsed_len_ = i - i0;
  }


int my_fgetc( FILE * const f, const bool allow_comment = true )
  {
  int ch = std::fgetc( f );
  if( ch == '#' && allow_comment )			// comment
    { do ch = std::fgetc( f ); while( ch != '\n' && ch != EOF ); }
  return ch;
  }


// Read a line discarding comments, leading and trailing whitespace and
// blank lines.
// Returns the size of 'line', or 0 if at EOF.
//
int my_fgets( FILE * const f, std::string & line, int & linenum )
  {
  line.clear();
  int ch;

  do { ch = my_fgetc( f ); if( ch == '\n' ) ++linenum; }
  while( UCS::isspace( ch ) );	// discard leading whitespace and blank lines
  while( ch != EOF )
    {
    line += ch;
    ch = my_fgetc( f, UCS::isspace( ch ) );
    if( ch == '\n' ) { ++linenum; break; }
    }
  if( ch == EOF && !line.empty() ) ++linenum;
  unsigned i = line.size();		// remove trailing whitespace
  while( i > 0 && UCS::isspace( (unsigned char)line[i-1] ) ) --i;
  if( i < line.size() ) line.resize( i );
  return line.size();
  }

} // end namespace


bool User_filter::enable_char( const int code, int new_code )
  {
  if( code < 0 || code > 0xFFFF ) return false;
  if( new_code < 0 || new_code > 0xFFFF ) new_code = code;
  if( code < 256 ) table1[code] = new_code;
  else
    {
    unsigned i = table2.size();
    while( i > 0 && table2[i-1].code > code ) --i;
    if( i == 0 || table2[i-1].code < code )
      table2.insert( table2.begin() + i, Entry( code, new_code ) );
    else table2[i-1].new_code = new_code;
    }
  return true;
  }


int User_filter::parse_char( const std::string & line, unsigned &i ) const
  {
  int code = -1;
  const unsigned len = line.size();
  while( i < len && std::isspace( line[i] ) ) ++i;	// strip spaces
  if( i + 2 < len && line[i] == '\'' )
    {
    if( line[i+1] == '\\' )					// '\X'
      {
      int l, cht = ISO_8859::escape( line, i + 2, &l );
      if( cht >= 0 && i + 2 + l < len && line[i+2+l] == '\'' )
        { code = UCS::map_to_ucs( cht ); i += 3 + l; }
      }
    else if( line[i+2] == '\'' )				// 'X'
      { code = UCS::map_to_ucs( line[i+1] ); i += 3; }
    }
  else if( i + 4 < len && line[i] == 'U' )			// UXXXX
    {
    code = 0;
    for( int j = 1; j <= 4; ++j )
      {
      const int d = ISO_8859::xvalue( line[i+j] );
      if( d < 0 ) { code = -1; ; break; }
      code = ( code << 4 ) + d;
      }
    if( code >= 0 ) i += 5;
    }
  if( code >= 0 )
    {
    while( i < len && std::isspace( line[i] ) ) ++i;	// strip spaces
    if( i < len && line[i] != ',' && line[i] != '-' ) code = -1;
    }
  return code;
  }


void User_filter::set_file_error( const char * const file_name, const int linenum )
  {
  char buf[80];
  snprintf( buf, sizeof buf, "error in file '%s', line %d.", file_name, linenum );
  error_ = buf;
  }


User_filter::User_filter( const char * const file_name )
  : table1( 256, -1 ), retval_( 0 ), default_( d_discard )
  {
  FILE * const f = std::fopen( file_name, "r" );
  if( !f )
    {
    error_ = "Can't open file '"; error_ += file_name; error_ += '\'';
    retval_ = 1;
    return;
    }

  std::string line;
  int linenum = 0;
  while( retval_ == 0 )
    {
    if( my_fgets( f, line, linenum ) <= 0 ) break;
    if( line == "leave" )
      { if( !default_ ) default_ = d_leave; else retval_ = 2; continue; }
    else if( line == "mark" )
      { if( !default_ ) default_ = d_mark; else retval_ = 2; continue; }

    int new_code = -1;			// parse new_code first (if any)
    for( unsigned equ = line.size(), j = 0; j < 2; ++j )
      {
      equ = line.rfind( '=', equ - 1 );
      if( equ >= line.size() ) break;
      unsigned i = equ + 1;
      const int tmp = parse_char( line, i );
      if( tmp >= 0 && i == line.size() && enable_char( tmp, tmp ) )
        { new_code = tmp; line.resize( equ ); break; }
      }

    unsigned i = 0;					// index in line
    while( retval_ == 0 )				// parse
      {
      while( i < line.size() && std::isspace( line[i] ) ) ++i;	// strip spaces
      if( i >= line.size() ) break;		// no more chars in line
      const unsigned char ch = line[i];
      if( ch == '[' )				// parse set
        {
        Set set( line, i );		// Set of characters to recognize
        if( !set.parsed_len() ) { retval_ = 2; break; }
        i += set.parsed_len();
        for( int c = 0; c < 256; ++c )
          if( set.includes( c ) && !enable_char( c, new_code ) )
            { retval_ = 2; break; }
        }
      else if( ch == '\'' || ch == 'U' )	// parse char or range
        {
        const int code = parse_char( line, i );
        if( code < 0 || !enable_char( code, new_code ) )
          { retval_ = 2; break; }
        if( i < line.size() && line[i] == '-' )
          {
          ++i;
          const int code2 = parse_char( line, i );
          if( code2 <= code ) { retval_ = 2; break; }
          for( int j = code + 1; j <= code2; ++j )
            if( !enable_char( j, new_code ) ) { retval_ = 2; break; }
          }
        }
      else { retval_ = 2; break; }		// no set, char or range
      if( i < line.size() && line[i] == ',' ) ++i;
      }
    }
  if( std::fclose( f ) != 0 && retval_ == 0 ) retval_ = 1;
  if( retval_ != 0 ) set_file_error( file_name, linenum );
  }


int User_filter::get_new_code( const int code ) const
  {
  int result = -1;
  if( code >= 0 )
    {
    if( code < 256 ) result = table1[code];
    else
      {
      for( unsigned i = 0; i < table2.size(); ++i )
        if( code == table2[i].code ) { result = table2[i].new_code; break; }
      }
    }
  if( result < 0 && default_ == d_leave ) result = code;
  return result;
  }
