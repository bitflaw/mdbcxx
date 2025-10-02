#include "../include/mdbcxx/row.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

Row::Row(const Row& row):
fields(row.fields), col_names(row.col_names)
{}

Row::Row(Row&& row):
fields(std::move(row.fields)), col_names(std::move(row.col_names))
{}

Row::Row (MYSQL_RES* res, MYSQL_ROW row)
{
  std::size_t cols = mysql_num_fields(res);
  MYSQL_FIELD* farray = mysql_fetch_fields(res);

  for (std::size_t i = 0; i < cols; i++) {
    fields.emplace_back(&farray[i], row[i]);
    col_names.emplace_back(farray[i].name);
  }
}

Field& Row::operator[] (std::size_t index)
{
  if (fields.size() < index)
  {
    throw std::length_error("[ERROR: in 'Row::operator[]'] Index out of bounds");
  }
  return fields[index];
}

std::optional<Field> Row::operator[] (std::string column_name)
{
  std::size_t index {0};
  std::size_t fields_size {fields.size()};

  for (; index < fields_size; index++) {
    if(col_names[index] == column_name) return fields[index];
  }
  return std::nullopt;
}
