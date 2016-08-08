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

class Features
  {
  const Blob & b;		// Blob to witch these features belong
  mutable bool hbar_initialized, vbar_initialized;
  mutable std::vector< Rectangle > hbar_, vbar_;
  mutable std::vector< std::vector< Csegment > > row_scan, col_scan;

  Features( const Features & );			// declared as private
  void operator=( const Features & );		// declared as private

  void row_scan_init() const;
  void col_scan_init() const;

public:
  mutable Profile lp, tp, rp, bp, hp, wp;

  explicit Features( const Blob & b_ );

//  const Blob & blob() const { return b; }

  const Rectangle & hbar( const int i ) const { return hbar_[i]; }
  const Rectangle & vbar( const int i ) const { return vbar_[i]; }
  int hbars() const;
  int vbars() const;

      // number of vertical traces crossing every row
  int segments_in_row( const int row ) const
    {
    if( row_scan.empty() ) row_scan_init();
    return row_scan[row-b.top()].size();
    }
      // number of horizontal traces crossing every column
  int segments_in_col( const int col ) const
    {
    if( col_scan.empty() ) col_scan_init();
    return col_scan[col-b.left()].size();
    }

           // vertical segment containing the point (row,col), if any
  Csegment v_segment( const int row, const int col ) const;

  int test_235Esz( const Charset & charset ) const;
  int test_49ARegpq( const Rectangle & charbox ) const;
  int test_4ADQao( const Charset & charset, const Rectangle & charbox ) const;
  int test_6abd( const Charset & charset ) const;
  int test_EFIJLlT( const Charset & charset, const Rectangle & charbox ) const;
  int test_c() const;
  int test_frst( const Rectangle & charbox ) const;
  int test_G() const;
  int test_HKMNUuvwYy( const Rectangle & charbox ) const;
  int test_hknwx( const Rectangle & charbox ) const;
  int test_s_cedilla() const;

  bool test_comma() const;
  int test_easy( const Rectangle & charbox ) const;
  int test_line( const Rectangle & charbox ) const;
  int test_solid( const Rectangle & charbox ) const;
  int test_misc( const Rectangle & charbox ) const;
  };
