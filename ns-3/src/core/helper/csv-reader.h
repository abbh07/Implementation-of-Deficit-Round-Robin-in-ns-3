/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 Lawrence Livermore National Laboratory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathew Bielejeski <bielejeski1@llnl.gov>
 */

#ifndef NS3_CSV_READER_H_
#define NS3_CSV_READER_H_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <istream>
#include <string>
#include <vector>

namespace ns3 {

// *NS_CHECK_STYLE_OFF*  Style checker trims blank lines in code blocks

// *NS_CHECK_STYLE_ON*
class CsvReader
{
public:
  CsvReader (const std::string& filepath, char delimiter = ',');

  CsvReader (std::istream& stream, char delimiter = ',');

  virtual ~CsvReader ();

  std::size_t ColumnCount () const;

  std::size_t RowNumber () const;

  char Delimiter () const;

  bool FetchNextRow ();

  template<class T>
  bool GetValue (std::size_t columnIndex, T& value) const;

  bool IsBlankRow () const;

private:
  bool GetValueAs (std::string input, double& value) const;

  bool GetValueAs (std::string input, float& value) const;

  bool GetValueAs (std::string input, signed char& value) const;

  bool GetValueAs (std::string input, short& value) const;

  bool GetValueAs (std::string input, int& value) const;

  bool GetValueAs (std::string input, long& value) const;

  bool GetValueAs (std::string input, long long& value) const;

  bool GetValueAs (std::string input, std::string& value) const;

  bool GetValueAs (std::string input, unsigned char& value) const;

  bool GetValueAs (std::string input, unsigned short& value) const;

  bool GetValueAs (std::string input, unsigned int& value) const;

  bool GetValueAs (std::string input, unsigned long& value) const;

  bool GetValueAs (std::string input, unsigned long long& value) const;
  bool IsDelimiter (char c) const;

  void ParseLine (const std::string& line);

  std::tuple<std::string, std::string::const_iterator>
  ParseColumn (std::string::const_iterator begin, std::string::const_iterator end);

  typedef std::vector<std::string> Columns;

  char m_delimiter;
  std::size_t m_rowsRead;
  Columns m_columns;
  bool m_blankRow;
  std::ifstream m_fileStream;

  std::istream* m_stream;

};  // class CsvReader


/****************************************************
 *      Template implementations.
 ***************************************************/

template<class T>
bool
CsvReader::GetValue (std::size_t columnIndex, T& value) const
{
  if ( columnIndex >= ColumnCount () )
    {
      return false;
    }

  std::string cell = m_columns[columnIndex];

  return GetValueAs (std::move (cell), value);
}

}   //  namespace ns3

#endif  //  NS3_CSV_READER_H_