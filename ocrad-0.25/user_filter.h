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

class User_filter
  {
public:
       // default action: discard, leave unmodified, mark as unrecognized
  enum Default { d_discard = 0, d_leave, d_mark };

private:
  struct Entry
    {
    int code;
    int new_code;
    Entry( const int c, const int nc ) : code( c ), new_code( nc ) {}
    };

  std::vector< int > table1;	// -1 or new_code of first 256 UCS chars
  std::vector< Entry > table2;		// codes of UCS chars >= 256
  std::string error_;
  int retval_;
  Default default_;

  bool enable_char( const int code, int new_code );
  int parse_char( const std::string & line, unsigned &i ) const;
  void set_file_error( const char * const file_name, const int linenum );

public:
  explicit User_filter( const char * const file_name );
  const std::string & error() const { return error_; }
  int retval() const { return retval_; }
  int get_new_code( const int code ) const;	// -1 means disabled
  bool discard() const { return default_ == d_discard; }
  bool mark() const { return default_ == d_mark; }
  };
