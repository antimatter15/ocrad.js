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

#include <cctype>
#include <string>

#include "iso_8859.h"


// 'seq[i]' begins a escape sequence (the characters following a '\').
// Returns the corresponding code and, in '*lenp', the characters read.
// Returns -1 if error.
//
int ISO_8859::escape( const std::string & seq, const unsigned i, int *lenp )
  {
  if( i >= seq.size() ) return -1;
  int len = 1;
  unsigned ch = seq[i];

  switch( ch )
    {
    case 'a': ch = '\a'; break;
    case 'b': ch = '\b'; break;
    case 'e': ch = 27; break;
    case 'f': ch = '\f'; break;
    case 'n': ch = '\n'; break;
    case 'r': ch = '\r'; break;
    case 't': ch = '\t'; break;
    case 'v': ch = '\v'; break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      ch -= '0';
      while( len < 3 && i + len < seq.size() && ISO_8859::isodigit( seq[i+len] ) )
        { ch <<= 3; ch += seq[i+len] - '0'; ++len; }
      if( ch > 255 ) return -1;
      break;
    case 'x':
    case 'X': if( i + 2 >= seq.size() || ISO_8859::xvalue( seq[i+1] ) < 0 ||
                  ISO_8859::xvalue( seq[i+2] ) < 0 ) return -1;
              ch = ( ISO_8859::xvalue( seq[i+1] ) << 4 ) +
                   ISO_8859::xvalue( seq[i+2] );
              len = 3;
              break;
    }
  if( lenp ) *lenp = len;
  return ch;
  }


int ISO_8859::xvalue( const unsigned char ch )
  {
  switch( ch )
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 0x0A;
    case 'b':
    case 'B': return 0x0B;
    case 'c':
    case 'C': return 0x0C;
    case 'd':
    case 'D': return 0x0D;
    case 'e':
    case 'E': return 0x0E;
    case 'f':
    case 'F': return 0x0F;
    default: return -1;
    }
  }
