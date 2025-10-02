#include "../include/mdbcxx/field.hpp"
#include <mysql/mariadb_com.h>

Field::Field(const Field& field):
value(field.value), metadata(field.metadata)
{}

Field::Field(Field&& field):
value(std::move(field.value)), metadata(std::move(field.metadata))
{}

Field& Field::operator= (Field& field)
{
  value = field.value;
  metadata = field.metadata;
  return *this;
}

Field& Field::operator= (Field&& field)
{
  if (this != &field)
  {
    value = std::move(field.value);
    metadata = std::move(field.metadata);
  }
  return *this;
}

void Field::operator() (Field& field)
{
  value = field.value;
  metadata = field.metadata;
}

void Field::operator() (Field&& field)
{
  value = std::move(field.value);
  metadata = std::move(field.metadata);
}

Field::Field(MYSQL_FIELD* field, char* raw)
{
  metadata = FieldMetadata {field};
  switch (field->type)
  {
    case MYSQL_TYPE_NULL:
      value = std::monostate {};
      break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
      value = std::vector<uint8_t>{reinterpret_cast<uint8_t*>(raw),
                                   reinterpret_cast<uint8_t*>(raw) + field->length
                                   };
      break;
    default:
      if (raw) { value = std::string {raw, field->length}; }
      else { value = std::monostate{}; }
      break;
  }
}

template <typename T>
T& Field::as () { return static_cast<T>(value); }

bool Field::is_null () { return std::holds_alternative<std::monostate>(value); }
bool Field::is_string () { return std::holds_alternative<std::string>(value); }
bool Field::is_blob () { return std::holds_alternative<std::vector<uint8_t>>(value); }

const std::string& Field::as_string () { return std::get<std::string>(value); }
const std::vector<uint8_t>& Field::as_blob () {return std::get<std::vector<uint8_t>>(value); }
