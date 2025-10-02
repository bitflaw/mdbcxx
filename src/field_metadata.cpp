#include "../include/mdbcxx/field_metadata.hpp"

FieldMetadata::FieldMetadata(const FieldMetadata& fmeta):
  name(fmeta.name), default_value(fmeta.default_value),
  length(fmeta.length), max_length(fmeta.max_length),
  flags(fmeta.flags), decimals(fmeta.decimals),
  sql_type(fmeta.sql_type)
{}

FieldMetadata::FieldMetadata(FieldMetadata&& fmeta):
  name(std::move(fmeta.name)), default_value(std::move(fmeta.default_value)),
  length(std::move(fmeta.length)), max_length(std::move(fmeta.max_length)),
  flags(std::move(fmeta.flags)), decimals(std::move(fmeta.decimals)),
  sql_type(std::move(fmeta.sql_type))
{}

FieldMetadata::FieldMetadata (MYSQL_FIELD* fmeta):
  name(fmeta->name), default_value(fmeta->def),
  length(fmeta->length), max_length(fmeta->max_length),
  flags(fmeta->flags), decimals(fmeta->decimals),
  sql_type(fmeta->type)
{}

void FieldMetadata::operator() (FieldMetadata& fmeta)
{
  name = fmeta.name;
  default_value = fmeta.default_value;
  length = fmeta.length;
  max_length = fmeta.max_length;
  flags = fmeta.flags;
  decimals = fmeta.decimals;
  sql_type = fmeta.sql_type;
}

void FieldMetadata::operator() (FieldMetadata&& fmeta)
{
  if (this != &fmeta)
  {
    name = std::move(fmeta.name);
    default_value = std::move(fmeta.default_value);
    length = std::move(fmeta.length);
    max_length = std::move(fmeta.max_length);
    flags = std::move(fmeta.flags);
    decimals = std::move(fmeta.decimals);
    sql_type = std::move(fmeta.sql_type);
  }
}

FieldMetadata& FieldMetadata::operator= (FieldMetadata& fmeta)
{
  name = fmeta.name;
  default_value = fmeta.default_value;
  length = fmeta.length;
  max_length = fmeta.max_length;
  flags = fmeta.flags;
  decimals = fmeta.decimals;
  sql_type = fmeta.sql_type;
  return *this;
}

FieldMetadata& FieldMetadata::operator= (FieldMetadata&& fmeta)
{
  if (this != &fmeta)
  {
    name = std::move(fmeta.name);
    default_value = std::move(fmeta.default_value);
    length = std::move(fmeta.length);
    max_length = std::move(fmeta.max_length);
    flags = std::move(fmeta.flags);
    decimals = std::move(fmeta.decimals);
    sql_type = std::move(fmeta.sql_type);
  }
  return *this;
}
