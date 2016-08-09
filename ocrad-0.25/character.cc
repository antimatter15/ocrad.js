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
#include <string>
#include <vector>
#include <stdint.h>

#include "common.h"
#include "rectangle.h"
#include "segment.h"
#include "ucs.h"
#include "user_filter.h"
#include "bitmap.h"
#include "blob.h"
#include "character.h"
#include "profile.h"
#include "feats.h"


Character::Character( const Character & c )
  : Rectangle( c ), blobpv( c.blobpv ), gv( c.gv )
  {
  for( unsigned i = 0; i < blobpv.size(); ++i )
    blobpv[i] = new Blob( *c.blobpv[i] );
  }


Character & Character::operator=( const Character & c )
  {
  if( this != &c )
    {
    Rectangle::operator=( c );
    for( unsigned i = 0; i < blobpv.size(); ++i ) delete blobpv[i];
    blobpv = c.blobpv;
    for( unsigned i = 0; i < blobpv.size(); ++i )
      blobpv[i] = new Blob( *c.blobpv[i] );
    gv = c.gv;
    }
  return *this;
  }


Character::~Character()
  {
  for( unsigned i = 0; i < blobpv.size(); ++i ) delete blobpv[i];
  }


// Returns the filled area of the main blobs only (no recursive)
//
int Character::area() const
  {
  int a = 0;
  for( int i = 0; i < blobs(); ++i ) a += blobpv[i]->area();
  return a;
  }


const Blob & Character::blob( const int i ) const
  {
  if( i < 0 || i >= blobs() )
    Ocrad::internal_error( "const blob, index out of bounds" );
  return *blobpv[i];
  }


Blob & Character::blob( const int i )
  {
  if( i < 0 || i >= blobs() )
    Ocrad::internal_error( "blob, index out of bounds" );
  return *blobpv[i];
  }


Blob & Character::main_blob()
  {
  int imax = 0;
  for( int i = 1; i < blobs(); ++i )
    if( blobpv[i]->size() > blobpv[imax]->size() )
      imax = i;
  return *blobpv[imax];
  }


void Character::shift_blobp( Blob * const p )
  {
  add_rectangle( *p );
  int i = blobs() - 1;
  for( ; i >= 0; --i )
    {
    Blob & bi = *blobpv[i];
    if( p->vcenter() > bi.vcenter() ) break;
    if( p->vcenter() == bi.vcenter() && p->hcenter() >= bi.hcenter() ) break;
    }
  blobpv.insert( blobpv.begin() + ( i + 1 ), p );
  }


void Character::insert_guess( const int i, const int code, const int value )
  {
  if( i < 0 || i > guesses() )
    Ocrad::internal_error( "insert_guess, index out of bounds" );
  gv.insert( gv.begin() + i, Guess( code, value ) );
  }


void Character::delete_guess( const int i )
  {
  if( i < 0 || i >= guesses() )
    Ocrad::internal_error( "delete_guess, index out of bounds" );
  gv.erase( gv.begin() + i );
  }


bool Character::set_merged_guess( const int code1, const int right1,
                                  const int code2, const int blob_index )
  {
  if( blob_index < 0 || blob_index >= blobs() ) return false;
  const Blob & b = *blobpv[blob_index];
  if( b.left() <= right1 && right1 < b.right() )
    {
    only_guess( -(blob_index + 1), left() );
    add_guess( code1, right1 );
    add_guess( code2, right() );
    return true;
    }
  return false;
  }


void Character::swap_guesses( const int i, const int j )
  {
  if( i < 0 || i >= guesses() || j < 0 || j >= guesses() )
    Ocrad::internal_error( "swap_guesses, index out of bounds" );
  const int code = gv[i].code;
  gv[i].code = gv[j].code; gv[j].code = code;
  }


const Character::Guess & Character::guess( const int i ) const
  {
  if( i < 0 || i >= guesses() )
    Ocrad::internal_error( "guess, index out of bounds" );
  return gv[i];
  }


bool Character::maybe( const int code ) const
  {
  for( int i = 0; i < guesses(); ++i )
    if( code == gv[i].code ) return true;
  return false;
  }

/*
bool Character::maybe_digit() const
  {
  for( int i = 0; i < guesses(); ++i )
    if( UCS::isdigit( gv[i].code ) ) return true;
  return false;
  }


bool Character::maybe_letter() const
  {
  for( int i = 0; i < guesses(); ++i )
    if( UCS::isalpha( gv[i].code ) ) return true;
  return false;
  }
*/

void Character::join( Character & c )
  {
  for( int i = 0; i < c.blobs(); ++i ) shift_blobp( c.blobpv[i] );
  c.blobpv.clear();
  }


unsigned char Character::byte_result() const
  {
  if( guesses() )
    {
    const unsigned char ch = UCS::map_to_byte( gv[0].code );
    if( ch ) return ch;
    }
  return '_';
  }


const char * Character::utf8_result() const
  {
  if( guesses() )
    {
    const char * s = UCS::ucs_to_utf8( gv[0].code );
    if( *s ) return s;
    }
  return "_";
  }


void Character::print( const Control & control ) const
  {
  if( guesses() )
    {
    if( !control.utf8 )
      {
      unsigned char ch = UCS::map_to_byte( gv[0].code );
      if( ch ) std::putc( ch, control.outfile );
      }
    else if( gv[0].code )
      std::fputs( UCS::ucs_to_utf8( gv[0].code ), control.outfile );
    }
  else std::putc( '_', control.outfile );
  }


void Character::dprint( const Control & control, const Rectangle & charbox,
                        const bool graph, const bool recursive ) const
  {
  if( graph || recursive )
    std::fprintf( control.outfile, "%d guesses    ", guesses() );
  for( int i = 0; i < guesses(); ++i )
    {
    if( gv[i].code == '\t' )
      std::fprintf( control.outfile, "guess '\\t', confidence %d    ",
                             gv[i].value );
    else if( !control.utf8 )
      {
      unsigned char ch = UCS::map_to_byte( gv[i].code );
      if( ch ) std::fprintf( control.outfile, "guess '%c', confidence %d    ",
                             ch, gv[i].value );
      }
    else
      std::fprintf( control.outfile, "guess '%s', confidence %d    ",
                    UCS::ucs_to_utf8( gv[i].code ), gv[i].value );
    if( !graph && !recursive ) break;
    }
  std::fputs( "\n", control.outfile );
  if( graph )
    {
    std::fprintf( control.outfile,
                  "left = %d, top = %d, right = %d, bottom = %d\n",
                  left(), top(), right(), bottom() );
    std::fprintf( control.outfile,
                  "width = %d, height = %d, hcenter = %d, vcenter = %d, black area = %d%%\n",
                  width(), height(), hcenter(), vcenter(), ( area() * 100 ) / size() );
    if( blobs() >= 1 && blobs() <= 3 )
      {
      const Blob & b = blob( blobs() - 1 );
      Features f( b );
      std::fprintf( control.outfile,
                    "hbars = %d, vbars = %d\n", f.hbars(), f.vbars() );
      }
    std::fputs( "\n", control.outfile );

    const int minrow = std::min( top(), charbox.top() );
    const int maxrow = std::max( bottom(), charbox.bottom() );
    for( int row = minrow; row <= maxrow; ++row )
      {
      bool istop = ( row == top() );
      bool isvc = ( row == vcenter() );
      bool isbot = ( row == bottom() );
      bool iscbtop = ( row == charbox.top() );
      bool iscbvc = ( row == charbox.vcenter() );
      bool iscbbot = ( row == charbox.bottom() );

      bool ish1top = false, ish1bot = false, ish2top = false, ish2bot = false;
      if( blobs() == 1 && blobpv[0]->holes() )
        {
        const Blob & b = *blobpv[0];
        ish1top = ( row == b.hole(0).top() );
        ish1bot = ( row == b.hole(0).bottom() );
        if( b.holes() > 1 )
          {
          ish2top = ( row == b.hole(1).top() );
          ish2bot = ( row == b.hole(1).bottom() );
          }
        }

      for( int col = left(); col <= right(); ++col )
        {
        char ch = ( isvc && col == hcenter() ) ? '+' : '.';
        for( int i = 0; i < blobs(); ++i )
          {
          int id = blobpv[i]->id( row, col );
          if( id != 0 )
            {
            if( id > 0 ) ch = (ch == '+') ? 'C' : 'O';
            else ch = (ch == '+') ? '=' : '-';
            break;
            }
          }
        std::fprintf( control.outfile, " %c", ch );
        }
      if( istop ) std::fprintf( control.outfile, "  top(%d)", row );
      if( isvc ) std::fprintf( control.outfile, "  vcenter(%d)", row );
      if( isbot ) std::fprintf( control.outfile, "  bottom(%d)", row );

      if( iscbtop ) std::fprintf( control.outfile, "  box.top(%d)", row );
      if( iscbvc ) std::fprintf( control.outfile, "  box.vcenter(%d)", row );
      if( iscbbot ) std::fprintf( control.outfile, "  box.bottom(%d)", row );

      if( ish1top ) std::fprintf( control.outfile, "  h1.top(%d)", row );
      if( ish1bot ) std::fprintf( control.outfile, "  h1.bottom(%d)", row );
      if( ish2top ) std::fprintf( control.outfile, "  h2.top(%d)", row );
      if( ish2bot ) std::fprintf( control.outfile, "  h2.bottom(%d)", row );

      std::fputs( "\n", control.outfile );
      }
    std::fputs( "\n\n", control.outfile );
    }
  }


void Character::xprint( const Control & control ) const
  {
  std::fprintf( control.exportfile, "%3d %3d %2d %2d; %d",
                left(), top(), width(), height(), guesses() );

  for( int i = 0; i < guesses(); ++i )
    if( !control.utf8 )
      {
      unsigned char ch = UCS::map_to_byte( gv[i].code );
      if( !ch ) ch = '_';
      std::fprintf( control.exportfile, ", '%c'%d", ch, gv[i].value );
      }
    else
      std::fprintf( control.exportfile, ", '%s'%d",
                    UCS::ucs_to_utf8( gv[i].code ), gv[i].value );
  std::fputs( "\n", control.exportfile );
  }


void Character::apply_filter( const Filter::Type filter )
  {
  if( !guesses() ) return;
  const int code = gv[0].code;
  bool remove = false;

  switch( filter )
    {
    case Filter::letters_only:
      remove = true;				// fall through
    case Filter::letters:
      if( !UCS::isalpha( code ) && !UCS::isspace( code ) )
        {
        for( int i = 1; i < guesses(); ++i )
          if( UCS::isalpha( gv[i].code ) ) { swap_guesses( 0, i ); break; }
        if( gv[0].code == '+' && 2 * height() > 3 * width() )
          { gv[0].code = 't'; break; }
        if( !UCS::isalpha( gv[0].code ) )
          gv[0].code = UCS::to_nearest_letter( gv[0].code );
        if( remove && !UCS::isalpha( gv[0].code ) ) clear_guesses();
        }
      break;
    case Filter::numbers_only:
      remove = true;				// fall through
    case Filter::numbers:
      if( !UCS::isdigit( code ) && !UCS::isspace( code ) )
        {
        for( int i = 1; i < guesses(); ++i )
          if( UCS::isdigit( gv[i].code ) ) { swap_guesses( 0, i ); break; }
        if( !UCS::isdigit( gv[0].code ) )
          gv[0].code = UCS::to_nearest_digit( gv[0].code );
        if( remove && !UCS::isdigit( gv[0].code ) ) clear_guesses();
        }
      break;
    case Filter::same_height: break;		// handled at line level
    case Filter::text_block: break;		// handled at block level
    case Filter::upper_num_mark:		// fall through
    case Filter::upper_num_only:
      remove = true;				// fall through
    case Filter::upper_num:
      if( !UCS::isupper( code ) && !UCS::isdigit( code ) &&
          !UCS::isspace( code ) )
        {
        for( int i = 1; i < guesses(); ++i )
          if( UCS::isupper( gv[i].code ) || UCS::isdigit( gv[i].code ) )
            { swap_guesses( 0, i ); break; }
        if( !UCS::isupper( gv[0].code ) && !UCS::isdigit( gv[0].code ) )
          gv[0].code = UCS::to_nearest_upper_num( gv[0].code );
        if( remove && !UCS::isupper( gv[0].code ) && !UCS::isdigit( gv[0].code ) )
          clear_guesses();
        }
      break;
    case Filter::user: break;		// handled by apply_user_filter
    }
  }


void Character::apply_user_filter( const User_filter & user_filter )
  {
  if( !guesses() || UCS::isspace( gv[0].code ) ) return;
  int new_code = user_filter.get_new_code( gv[0].code );

  if( new_code >= 0 ) gv[0].code = new_code;
  else							// disabled
    {
    for( int i = 1; i < guesses(); ++i )
      {
      new_code = user_filter.get_new_code( gv[i].code );
      if( new_code >= 0 )
        { swap_guesses( 0, i ); gv[0].code = new_code; break; }
      }
    if( new_code < 0 ) clear_guesses();
    }
  }
