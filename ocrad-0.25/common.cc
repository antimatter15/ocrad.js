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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "common.h"
#include "user_filter.h"


namespace {

const int charsets = 3;

const Charset::Value charset_value[charsets] =
  { Charset::ascii, Charset::iso_8859_9, Charset::iso_8859_15 };

const char * const charset_name[charsets] =
  { "ascii", "iso-8859-9", "iso-8859-15" };

struct F_entry
  {
  const char * const name;
  Filter::Type type;
  };

const F_entry F_table[] =
  {
  { "letters",        Filter::letters },
  { "letters_only",   Filter::letters_only },
  { "numbers",        Filter::numbers },
  { "numbers_only",   Filter::numbers_only },
  { "same_height",    Filter::same_height },
  { "text_block",     Filter::text_block },
  { "upper_num",      Filter::upper_num },
  { "upper_num_mark", Filter::upper_num_mark },
  { "upper_num_only", Filter::upper_num_only },
  { 0, Filter::user }				// not a valid name
  };

struct T_entry
  {
  const char * const name;
  Transformation::Type type;
  };

const T_entry T_table[] =
  {
  { "none",      Transformation::none },
  { "rotate90",  Transformation::rotate90 },
  { "rotate180", Transformation::rotate180 },
  { "rotate270", Transformation::rotate270 },
  { "mirror_lr", Transformation::mirror_lr },
  { "mirror_tb", Transformation::mirror_tb },
  { "mirror_d1", Transformation::mirror_d1 },
  { "mirror_d2", Transformation::mirror_d2 },
  { 0, Transformation::none }
  };

} // end namespace


int verbosity = 0;

void Ocrad::internal_error( const char * const msg )
  {
  std::fprintf( stderr, "ocrad: internal error: %s\n", msg );
  std::exit( 3 );
  }


bool Ocrad::similar( const int a, const int b,
                     const int percent_dif, const int abs_dif )
  {
  int difference = std::abs( a - b );
  if( percent_dif > 0 && difference <= abs_dif ) return true;
  int max_abs = std::max( std::abs( a ), std::abs( b ) );
  return ( difference * 100 <= max_abs * percent_dif );
  }


bool Charset::enable( const char * const name )
  {
  for( int i = 0; i < charsets; ++i )
    if( std::strcmp( name, charset_name[i] ) == 0 )
      { charset_ |= charset_value[i]; return true; }
  return false;
  }


bool Charset::enabled( const Value cset ) const
  {
  if( !charset_ ) return cset == iso_8859_15;		// default charset
  return charset_ & cset;
  }


bool Charset::only( const Value cset ) const
  {
  if( !charset_ ) return cset == iso_8859_15;		// default charset
  return charset_ == cset;
  }


void Charset::show_error( const char * const program_name,
                          const char * const arg ) const
  {
  if( verbosity >= 0 )
    {
    if( arg && std::strcmp( arg, "help" ) != 0 )
      std::fprintf( stderr,"%s: bad charset '%s'\n", program_name, arg );
    std::fputs( "Valid charset names:", stderr );
    for( int i = 0; i < charsets; ++i )
      std::fprintf( stderr, "  %s", charset_name[i] );
    std::fputs( "\n", stderr );
    }
  }


bool Transformation::set( const char * const name )
  {
  for( int i = 0; T_table[i].name != 0; ++i )
    if( std::strcmp( name, T_table[i].name ) == 0 )
      { type_ = T_table[i].type; return true; }
  return false;
  }


void Transformation::show_error( const char * const program_name,
                                 const char * const arg ) const
  {
  if( verbosity >= 0 )
    {
    if( arg && std::strcmp( arg, "help" ) != 0 )
      std::fprintf( stderr,"%s: bad bitmap trasformation '%s'\n", program_name, arg );
    std::fputs( "Valid transformation names:", stderr );
    for( int i = 0; T_table[i].name != 0; ++i )
      std::fprintf( stderr, "  %s", T_table[i].name );
    std::fputs( "\nRotations are made counter-clockwise.\n", stderr );
    }
  }


Control::~Control()
  {
  for( unsigned f = filters.size(); f > 0; --f )
    if( filters[f-1].user_filterp )
      delete filters[f-1].user_filterp;
  }


bool Control::add_filter( const char * const program_name,
                          const char * const name )
  {
  for( int i = 0; F_table[i].name != 0; ++i )
    if( std::strcmp( name, F_table[i].name ) == 0 )
      { filters.push_back( Filter( F_table[i].type ) ); return true; }
  if( verbosity >= 0 )
    {
    if( name && std::strcmp( name, "help" ) != 0 )
      std::fprintf( stderr,"%s: bad filter '%s'\n", program_name, name );
    std::fputs( "Valid filter names:", stderr );
    for( int i = 0; F_table[i].name != 0; ++i )
      std::fprintf( stderr, "  %s", F_table[i].name );
    std::fputs( "\n", stderr );
    }
  return false;
  }


int Control::add_user_filter( const char * const program_name,
                              const char * const file_name )
  {
  const User_filter * const user_filterp = new User_filter( file_name );
  const int retval = user_filterp->retval();
  if( retval == 0 )
    filters.push_back( Filter( user_filterp ) );
  else
    {
    if( verbosity >= 0 )
      std::fprintf( stderr,"%s: user filter: %s\n",
                    program_name, user_filterp->error().c_str() );
    delete user_filterp;
    }
  return retval;
  }


bool Control::set_format( const char * const name )
  {
  if( std::strcmp( name, "byte" ) == 0 ) { utf8 = false; return true; }
  if( std::strcmp( name, "utf8" ) == 0 ) { utf8 = true; return true; }
  return false;
  }
