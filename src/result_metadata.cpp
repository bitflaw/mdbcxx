#include "../include/mdbcxx/result_metadata.hpp"
#include <cstddef>
#include <mysql/mysql.h>
#include <utility>

ResultMetadata::ResultMetadata (const ResultMetadata& rmeta):
  db_name(rmeta.db_name), num_rows(rmeta.num_rows),
  num_cols(rmeta.num_cols), columns(rmeta.columns)
{}

ResultMetadata::ResultMetadata (ResultMetadata&& rmeta):
  db_name(std::move(rmeta.db_name)), num_rows(std::move(rmeta.num_rows)),
  num_cols(std::move(rmeta.num_cols)), columns(std::move(rmeta.columns))
{}

ResultMetadata::ResultMetadata (MYSQL_RES* res):
  db_name(mysql_fetch_field_direct(res, 1)->db),
  num_rows(mysql_num_rows(res)),
  num_cols(mysql_num_fields(res))
{
  MYSQL_FIELD* fields = mysql_fetch_fields(res);
  columns.reserve(num_cols);
  for (std::size_t i = 0; i < num_cols; i++) columns.emplace_back(fields[i]);
}

void ResultMetadata::operator() (ResultMetadata& rmeta)
{
  db_name = rmeta.db_name;
  num_rows = rmeta.num_rows;
  num_cols = rmeta.num_cols;
  std::swap(columns, rmeta.columns);
}

void ResultMetadata::operator() (ResultMetadata&& rmeta)
{
  if (this != &rmeta)
  {
    db_name = std::move(rmeta.db_name);
    num_rows = std::move(rmeta.num_rows);
    num_cols = std::move(rmeta.num_cols);
    columns = std::move(rmeta.columns);
  }
}

ResultMetadata& ResultMetadata::operator= (ResultMetadata& rmeta)
{
  db_name = rmeta.db_name;
  num_rows = rmeta.num_rows;
  num_cols = rmeta.num_cols;
  std::swap(columns, rmeta.columns);
  return *this;
}

ResultMetadata& ResultMetadata::operator= (ResultMetadata&& rmeta)
{
  if (this != &rmeta)
  {
    db_name = std::move(rmeta.db_name);
    num_rows = std::move(rmeta.num_rows);
    num_cols = std::move(rmeta.num_cols);
    columns = std::move(rmeta.columns);
  }
  return *this;
}
