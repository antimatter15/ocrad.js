/*  GNU Ocrad - Optical Character Recognition program
    Copyright (C) 2003-2014 Antonio Diaz Diaz.

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

class Histogram
  {
  unsigned samples_;
  std::vector< int > distrib;

public:
  Histogram() : samples_( 0 ) {}

  int samples() const { return samples_; }
  int size() const { return distrib.size(); }
  bool empty() const { return distrib.empty(); }
  int operator[]( const int i ) const { return distrib[i]; }
  void reset() { samples_ = 0; distrib.clear(); }

  void add_sample( const unsigned sample )
    {
    if( sample < INT_MAX && samples_ < INT_MAX )
      { if( sample >= distrib.size() ) distrib.resize( sample + 1 );
        ++distrib[sample]; ++samples_; }
    }

  int median() const
    {
    unsigned l = 0, cum = 0;
    while( l < distrib.size() )
      { cum += distrib[l]; if( 2 * cum >= samples_ ) break; else ++l; }
    unsigned r = l;
    while( true )
      { if( 2 * cum > samples_ || r >= distrib.size() ) break;
        cum += distrib[r]; ++r; }
    return ( l + r ) / 2;
    }
  };
