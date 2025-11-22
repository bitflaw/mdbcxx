#include "../include/mdbcxx/result.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>

Result::Result(const Result& result):
  result_set(result.result_set),
  rmetadata(result.rmetadata)
{}

Result::Result(Result&& result):
  result_set(std::move(result.result_set)),
  rmetadata(std::move(result.rmetadata))
{}

Result::Result (MYSQL_RES* res)
{
  if (!res) return;
  ResultMetadata rmeta {res};
  rmetadata = std::move(rmeta);
  std::size_t res_size {mysql_num_rows(res)};
  std::cout<<"Number of rows(raw): "<<mysql_num_rows(res)
    <<"\nNumber of rows(size_t): "<<res_size<<std::endl;
  result_set.reserve(res_size);
  MYSQL_ROW row;
  for (std::size_t i = 0; i < res_size; i++) {
    row = mysql_fetch_row(res);
    if (!row) break;
    result_set.emplace_back(res,row);
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

std::vector<Row>::iterator Result::begin () { return result_set.begin(); }
std::vector<Row>::const_iterator Result::cbegin () { return result_set.cbegin(); }
std::vector<Row>::reverse_iterator Result::rbegin () { return result_set.rbegin(); }
std::vector<Row>::const_reverse_iterator Result::crbegin() { return result_set.crbegin(); }
std::vector<Row>::iterator Result::end () { return result_set.end(); }
std::vector<Row>::const_iterator Result::cend () { return result_set.cend(); }
std::vector<Row>::reverse_iterator Result::rend () { return result_set.rend(); }
std::vector<Row>::const_reverse_iterator Result::crend () { return result_set.crend(); }

std::size_t Result::size () const { return result_set.size(); }

void Result::append (Row row) { result_set.push_back(row); }
