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
#include <vector>
#include <stdint.h>

#include "common.h"
#include "rectangle.h"
#include "segment.h"
#include "ucs.h"
#include "bitmap.h"
#include "blob.h"
#include "profile.h"
#include "feats.h"


Features::Features( const Blob & b_ )
  : b( b_ ), hbar_initialized( false ), vbar_initialized( false ),
    lp( b, Profile::left ),
    tp( b, Profile::top ), rp( b, Profile::right ), bp( b, Profile::bottom ),
    hp( b, Profile::height ), wp( b, Profile::width )
  {}


void Features::row_scan_init() const
  {
  int l = -1;			// begin of segment. -1 means no segment
  row_scan.resize( b.height() );

  for( int row = b.top(); row <= b.bottom(); ++row )
    for( int col = b.left(); col <= b.right(); ++col )
      {
      bool black = b.get_bit( row, col );
      if( l < 0 && black ) l = col;			// begin of segment
      if( l >= 0 && ( !black || col == b.right() ) )	// end of segment
        { row_scan[row-b.top()].push_back( Csegment( l, col - !black ) );
          l = -1; }
      }
  }


void Features::col_scan_init() const
  {
  int t = -1;			// begin of segment. -1 means no segment
  col_scan.resize( b.width() );

  for( int col = b.left(); col <= b.right(); ++col )
    for( int row = b.top(); row <= b.bottom(); ++row )
      {
      bool black = b.get_bit( row, col );
      if( t < 0 && black ) t = row;			// begin of segment
      if( t >= 0 && ( !black || row == b.bottom() ) )	// end of segment
        { col_scan[col-b.left()].push_back( Csegment( t, row - !black ) );
          t = -1; }
      }
  }


int Features::hbars() const
  {
  if( !hbar_initialized )
    {
    hbar_initialized = true;
    if( row_scan.empty() ) row_scan_init();
    std::vector< Csegment > segv;
    segv.reserve( b.height() );

    for( unsigned i = 0; i < row_scan.size(); ++i )
      {
      if( row_scan[i].size() == 1 )
        { segv.push_back( row_scan[i][0] ); continue; }
      int maxsize = 0, jmax = -1;
      for( unsigned j = 0; j < row_scan[i].size(); ++j )
        {
        const int size = row_scan[i][j].size();
        if( maxsize < size ) { maxsize = size; jmax = j; }
        }
      if( jmax >= 0 ) segv.push_back( row_scan[i][jmax] );
      else segv.push_back( Csegment() );
      }

    const int limit = ( wp.max() + 1 ) / 2;
    int state = 0, begin = 0, l = 0, r = 0;
    for( int i = 0; i < b.height(); ++i )
      {
      Csegment & seg = segv[i];
      switch( state )
        {
        case 0: if( seg.size() <= limit ) break;
                state = 1; begin = i; l = seg.left; r = seg.right;
                if( i < b.height() - 1 ) break;
        case 1: if( seg.size() > limit &&
                    ( i <= begin || seg.overlaps( segv[i-1] ) ) )
                  {
                  if( seg.left < l ) l = seg.left;
                  if( seg.right > r ) r = seg.right;
                  if( i < b.height() - 1 ) break;
                  }
                state = 0;
                int end = ( seg.size() <= limit ) ? i - 1 : i;
                const int width = r - l + 1;
                while( begin <= end && 3 * segv[begin].size() < 2 * width )
                  ++begin;
                while( begin <= end && 3 * segv[end].size() < 2 * width )
                  --end;
                const int height = end - begin + 1;
                if( height < 1 || height > width ) break;
                const int margin = std::max( height, ( b.height() / 10 ) + 1 );
                if( begin >= margin )
                  {
                  bool good = false;
                  for( int j = margin; j > 0; --j )
                    if( 3 * segv[begin-j].size() <= 2 * width )
                      { good = true; break; }
                  if( !good ) break;
                  }
                if( end + margin < b.height() )
                  {
                  bool good = false;
                  for( int j = margin; j > 0; --j )
                    if( 3 * segv[end+j].size() <= 2 * width )
                      { good = true; break; }
                  if( !good ) break;
                  }
                hbar_.push_back( Rectangle( l, begin+b.top(), r, end+b.top() ) );
                break;
        }
      }
    while( hbar_.size() > 3 )			// remove noise hbars
      {
      int wmin = hbar_[0].width();
      for( unsigned i = 1; i < hbar_.size(); ++i )
        if( hbar_[i].width() < wmin ) wmin = hbar_[i].width();
      for( int i = hbar_.size() - 1; i >= 0; --i )
        if( hbar_[i].width() == wmin ) hbar_.erase( hbar_.begin() + i );
      }
    }
  return hbar_.size();
  }


int Features::vbars() const		// FIXME small gaps not detected
  {
  if( !vbar_initialized )
    {
    vbar_initialized = true;
    int state = 0, begin = 0, limit = b.height();
    limit -= ( b.height() < 40 ) ? 3 : b.height() / 10;

    for( int col = b.left(); col <= b.right(); ++col )
      {
      int c = 0, c2 = 0, count = 0;
      for( int row = b.top() + 1; row < b.bottom(); ++row )
        {
        if( b.get_bit( row, col ) )
          { ++c; if( row < b.bottom() - 1 ) continue; }
        else if( ( col > b.left() && b.get_bit( row, col - 1 ) ) ||
                 ( col < b.right() && b.get_bit( row, col + 1 ) ) )
          { ++c; ++c2; if( row < b.bottom() - 1 ) continue; }
        if( c > count ) { count = c; } c = 0;
        }
      if( ( count - c2 ) * 3 < limit * 2 ) count = 0;
      switch( state )
        {
        case 0: if( count >= limit ) { state = 3; begin = col; }
                else if( count * 4 >= limit * 3 ) { state = 2; begin = col; }
                else if( count * 3 >= limit * 2 ) { state = 1; begin = col; }
                break;
        case 1: if( count >= limit ) state = 3;
                else if( count * 4 >= limit * 3 ) state = 2;
                else if( count * 3 < limit * 2 ) state = 0;
                else begin = col;
                break;
        case 2: if( count >= limit ) state = 3;
                else if( count * 3 < limit * 2 ) state = 0;
                else if( count * 4 < limit * 3 ) state = 1;
                break;
        case 3: if( count * 3 < limit * 2 || col == b.right() )
                  {
                  int end = ( count * 3 < limit * 2 ) ? col - 1 : col;
                  vbar_.push_back( Rectangle( begin, b.top(), end, b.bottom() ) );
                  state = 0;
                  }
        }
      }
    }
  return vbar_.size();
  }


Csegment Features::v_segment( const int row, const int col ) const
  {
  const int segments = segments_in_col( col );
  for( int i = 0; i < segments; ++i )
    if( col_scan[col-b.left()][i].includes( row ) )
      return col_scan[col-b.left()][i];
  return Csegment();
  }


int Features::test_misc( const Rectangle & charbox ) const
  {
  if( bp.minima() == 1 )
    {
    if( hbars() == 1 && hbar(0).top() <= b.top() + ( b.height() / 10 ) &&
        4 * hbar(0).height() <= b.height() &&
        5 * hbar(0).width() >= 4 * b.width() &&
        rp[hbar(0).bottom()-b.top()+2] - rp[hbar(0).bottom()-b.top()] < b.width() / 4 &&
        rp.increasing( hbar(0).vcenter() - b.top() + 1 ) )
      return '7';

    if( b.height() > b.width() && rp.increasing() && !tp.decreasing() &&
        b.seek_left( b.vcenter(), b.hcenter() ) <= b.left() )
      return '7';
    }

  if( tp.minima( b.height() / 4 ) == 1 && bp.minima( b.height() / 4 ) == 1 )
    {
    if( b.height() > 2 * b.width() && rp.increasing() &&
        tp.decreasing() && lp.iscpit( 25 ) )
      return '1';

    if( hbars() == 1 ||
        ( hbars() == 2 && hbar(1).bottom() >= b.bottom() - 1 &&
        3 * hbar(0).width() > 4 * hbar(1).width() ) )
      if( 3 * hbar(0).height() < b.height() && hbar(0).top() <= b.top() + 1 )
        {
        int i = lp.pos( 40 );
        if( 3 * wp[i] < b.width() && 5 * lp[i] > b.width() &&
            5 * rp[i] > b.width() ) return 'T';
        }

    if( 3 * b.height() > 4 * b.width() &&
        vbars() == 1 && vbar(0).width() >= 2 )
      {
      const int lg = vbar(0).left() - b.left();
      const int rg = b.right() - vbar(0).right();
      if( 2 * lg < b.width() && 2 * rg < b.width() &&
          Ocrad::similar( lg, rg, 40 ) &&
          4 * bp[bp.pos(25)] > 3 * b.height() &&
          4 * tp[tp.pos(75)] > 3 * b.height() )
        return 'l';
      }

    if( 5 * b.height() >= 4 * charbox.height() && b.height() > wp.max() &&
        3 * wp[wp.pos(50)] < b.width() )
      {
      if( hbars() == 1 && hbar(0).bottom() >= b.bottom() - 1 &&
          hbar(0).top() > b.vpos( 75 ) &&
          Ocrad::similar( lp[lp.pos(50)], rp[rp.pos(50)], 20, 2 ) )
        return 'l';
      if( hbars() == 2 && hbar(0).bottom() < b.vpos( 25 ) &&
          hbar(1).top() > b.vpos( 75 ) &&
          hbar(1).bottom() >= b.bottom() - 1 /*&&
          3 * hbar(0).width() < 4 * hbar(1).width()*/ )
        {
        if( hbar(0).right() <= hbar(1).hcenter() ) return 0;
        if( 3 * hbar(0).width() <= 2 * hbar(1).width() ||
            b.height() >= 3 * wp.max() ) return 'l';
        return 'I';
        }
      }

    if( ( hbars() == 2 || hbars() == 3 ) && hbar(0).top() <= b.top() + 1 &&
        hbar(1).includes_vcenter( b ) &&
        3 * hbar(0).width() > 4 * hbar(1).width() &&
        ( hbars() == 2 ||
          ( hbar(2).bottom() >= b.bottom() - 1 &&
            3 * hbar(0).width() > 4 * hbar(2).width() ) ) ) return 'F';

    if( b.height() > 3 * wp.max() )
      {
      if( rp.istip() && lp.ispit() )
        { if( lp.istpit() ) return '{'; else return '('; }
      if( lp.istip() && rp.ispit() )
        { if( rp.istpit() ) return '}'; else return ')'; }
      if( b.width() > 2 * wp.max() && rp.isconvex() ) return ')';
      }

    if( b.height() > 2 * b.width() && 5 * b.height() >= 4 * charbox.height() &&
        lp.max() + rp.max() < b.width() )
      {
      if( 5 * rp[rp.pos(50)] > 2 * b.width() )
        {
        const int row = b.seek_top( b.vpos( 75 ), b.hpos( 75 ) );
        if( ( b.top() < charbox.top() ||
              b.bottom() <= charbox.bottom() + ( b.height() / 5 ) ) &&
            row <= b.top() ) return 'L';
        if( row > b.top() &&
            b.seek_bottom( b.vpos( 75 ), b.hpos( 75 ) ) < b.bottom() )
          return '[';
        }
      return '|';
      }
    }

  return 0;
  }
