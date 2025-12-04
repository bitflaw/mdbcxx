#include "../include/mdbcxx/field.hpp"
#include <string>

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

Field::Field(MYSQL_FIELD* field, char* raw, ulong len)
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
      value = std::vector<std::byte>{
        reinterpret_cast<std::byte*>(raw),
        reinterpret_cast<std::byte*>(raw) + len
      };
      break;
    default:
      if (raw) { value = std::string {raw, len}; }
      else { value = std::string{}; }
      break;
  }
}

template <typename T>
T Field::as () const { return static_cast<T>(value); }

template <>
short Field::as<short> () const { return std::stoi(std::get<std::string>(value)); }

template <>
int Field::as<int> () const { return std::stoi(std::get<std::string>(value)); }

template <>
float Field::as<float> () const { return std::stof(std::get<std::string>(value)); }

template <>
double Field::as<double> () const { return std::stod(std::get<std::string>(value)); }

template <>
long Field::as<long> () const { return std::stol(std::get<std::string>(value)); }

template <>
unsigned long Field::as<unsigned long> () const { return std::stoul(std::get<std::string>(value)); }

template <>
long long Field::as<long long> () const { return std::stoll(std::get<std::string>(value)); }

template <>
unsigned long long Field::as<unsigned long long> () const { return std::stoull(std::get<std::string>(value)); }

template <>
bool Field::as<bool> () const { return (!std::get<std::string>(value).empty() ? true:false); }

bool Field::is_null () { return std::holds_alternative<std::monostate>(value); }
const std::string& Field::as_string () { return std::get<std::string>(value); }
const std::vector<std::byte>& Field::as_blob () {return std::get<std::vector<std::byte>>(value); }
