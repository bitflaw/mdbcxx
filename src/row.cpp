#include "../include/mdbcxx/row.hpp"
#include <cstring>
#include <stdexcept>

Row::Row(const Row& row):
fields(row.fields), col_names(row.col_names)
{}

Row::Row(Row&& row):
fields(std::move(row.fields)), col_names(std::move(row.col_names))
{}

Row::Row (MYSQL_FIELD* farray, std::size_t nfarray, MYSQL_ROW row, ulong* lengths)
{
  if (lengths == NULL)
  {
    throw std::runtime_error("'lengths' array is null.");
  }
  for (std::size_t i = 0; i < nfarray; i++)
  {
    fields.emplace_back(&farray[i], row[i], lengths[i]);
    col_names.emplace_back(farray[i].name);
  }
}

Row::Row (MYSQL_FIELD* farray, std::vector<MYSQL_BIND>& bound_row)
{
  std::size_t n = bound_row.size();
  for (std::size_t i = 0; i<n; i++)
  {
    MYSQL_BIND bound_field = bound_row[i];
    fields.emplace_back(&farray[i], (char*)bound_field.buffer, *bound_field.length);
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
