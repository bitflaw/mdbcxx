#include "../include/mdbcxx/row.hpp"
#include <cstddef>
#include <cstring>

Row::Row(const Row& row):
fields(row.fields), col_names(row.col_names)
{}

Row::Row(Row&& row):
fields(std::move(row.fields)), col_names(std::move(row.col_names))
{}

Row& Row::operator= (const Row& old)
{
  fields = old.fields;
  col_names = old.col_names;
  return *this;
}

Row& Row::operator= (Row&& old)
{
  if (this != &old)
  {
    fields = std::move(old.fields);
    col_names = std::move(old.col_names);
  }
  return *this;
}

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

std::vector<Field>::iterator Row::begin ()                {return fields.begin();}
std::vector<Field>::const_iterator Row::cbegin ()         {return fields.cbegin();}
std::vector<Field>::reverse_iterator Row::rbegin()        {return fields.rbegin();}
std::vector<Field>::const_reverse_iterator Row::crbegin() {return fields.rbegin();}
std::vector<Field>::iterator Row::end ()                  {return fields.end();}
std::vector<Field>::const_iterator Row::cend ()           {return fields.cend();}
std::vector<Field>::reverse_iterator Row::rend ()         {return fields.rend();}
std::vector<Field>::const_reverse_iterator Row::crend ()  {return fields.crend();}

Field& Row::operator[] (std::size_t index)
{
  if (fields.size() < index)
  {
    throw std::length_error("Index out of bounds");
  }
  return fields[index];
}

Field& Row::operator[] (std::string column_name)
{
  std::size_t fields_size {fields.size()};

  for (std::size_t index = 0; index < fields_size; index++)
  {
    if(col_names[index] == column_name) return fields[index];
  }
  throw std::invalid_argument("No field found for the column name passed!");
}

std::size_t Row::size() const { return fields.size(); }
