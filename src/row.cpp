#include "../include/mdbcxx/row.hpp"
#include <cstddef>
#include <cstring>
#include <mysql/mysql.h>
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

static char empty_str = '\0';

MYSQL_FIELD construct_f (MYSQL_BIND* bound_field)
{
  return {
    .name = &empty_str,
    .org_name = &empty_str,
    .table = &empty_str,
    .org_table = &empty_str,
    .db = &empty_str,
    .catalog = &empty_str,
    .def = &empty_str,
    .length = bound_field->length? *bound_field->length:bound_field->length_value,
    .max_length = bound_field->length? *bound_field->length:bound_field->length_value,
    .name_length = 0,
    .org_name_length = 0,
    .table_length = 0,
    .org_table_length = 0,
    .db_length = 0,
    .catalog_length = 0,
    .def_length = 0,
    .flags = 0,
    .decimals = 0,
    .type = bound_field->buffer_type
  };
}

Row::Row (std::vector<MYSQL_BIND>& bound_row)
{
  for (MYSQL_BIND bound_field : bound_row)
  {
    MYSQL_FIELD field = construct_f(&bound_field);
    fields.emplace_back(&field, (char*)bound_field.buffer);
    col_names.push_back("");
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
