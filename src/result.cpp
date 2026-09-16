#include "../include/mdbcxx/result.hpp"
#include <cstddef>
#include <stdexcept>

namespace mcxx
{
Result::Result(const Result& result):
  result_set(result.result_set),
  metadata(result.metadata)
{}

Result::Result(Result&& result):
  result_set(std::move(result.result_set)),
  metadata(std::move(result.metadata))
{}

Result::Result (MYSQL_RES* res)
{
  if (!res) return;
  ResultMetadata rmeta {res};
  metadata = std::move(rmeta);
  std::size_t res_size {mysql_num_rows(res)};
  result_set.reserve(res_size);
  std::size_t ncol {mysql_num_fields(res)};
  MYSQL_FIELD* farray = mysql_fetch_fields(res);
  MYSQL_ROW row;
  for (std::size_t i = 0; i < res_size; i++)
  {
    row = mysql_fetch_row(res);
    if (!row) break;
    ulong* lengths = mysql_fetch_lengths(res);
    result_set.emplace_back(farray, ncol, row, lengths);
  }
  mysql_free_result(res);
}

Row& Result::operator[] (std::size_t index)
{
  if (result_set.size() < index)
  {
    throw std::length_error("[ERROR: in 'Result::operator[]'] Index out of bounds");
  }
  return result_set[index];
}

std::vector<Row>::iterator Result::begin ()                { return result_set.begin(); }
std::vector<Row>::const_iterator Result::cbegin ()         { return result_set.cbegin(); }
std::vector<Row>::reverse_iterator Result::rbegin ()       { return result_set.rbegin(); }
std::vector<Row>::const_reverse_iterator Result::crbegin() { return result_set.crbegin(); }
std::vector<Row>::iterator Result::end ()                  { return result_set.end(); }
std::vector<Row>::const_iterator Result::cend ()           { return result_set.cend(); }
std::vector<Row>::reverse_iterator Result::rend ()         { return result_set.rend(); }
std::vector<Row>::const_reverse_iterator Result::crend ()  { return result_set.crend(); }

std::size_t Result::size () const { return result_set.size(); }
bool Result::empty () const { return result_set.empty(); }

void Result::append (Row&& row) { result_set.push_back(row); }

};//namespace mcxx
