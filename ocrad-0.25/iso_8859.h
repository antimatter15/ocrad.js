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

namespace ISO_8859 {

// 'seq[i]' begins a escape sequence (the characters following a '\').
// Returns the corresponding code and, in '*lenp', the characters read.
// Returns -1 if error.
//
int escape( const std::string & seq, const unsigned i, int *lenp = 0 );

inline bool isodigit( const unsigned char ch )	// is 'ch' an octal digit?
  { return ( ch <= '7' && ch >= '0' ); }

int xvalue( const unsigned char ch );		// value of hex digit 'ch' or -1

} // end namespace ISO_8859
