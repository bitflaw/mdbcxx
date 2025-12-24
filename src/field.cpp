#include "../include/mdbcxx/field.hpp"
#include <stdexcept>
#include <chrono>
#include <cstddef>


namespace mcxx
{
Field::Field(const Field& field):
value(field.value), metadata(field.metadata)
{}

Field::Field(Field&& field):
value(std::move(field.value)), metadata(std::move(field.metadata))
{}

Field& Field::operator= (const Field& field)
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

Field::Field(MYSQL_FIELD* field, char* raw, ulong len)
{
  metadata = FieldMetadata {field};

  if (!(field->flags & NOT_NULL_FLAG) ||
    field->type == MYSQL_TYPE_NULL
  )
  {
    value = std::monostate {};
    return;
  }

  if (raw != NULL) value = std::string {raw, len};
  else value = std::string {};
}


template <typename T>
// WARN: probably should use reinterpret_cast since this can fail for types not
// convertible to and fro string.
T Field::as () const { return static_cast<T>(std::get<std::string>(value)); }

using BLOB_T = std::vector<std::byte>;
template <> BLOB_T Field::as<BLOB_T> () const
{
  std::string v = std::get<std::string>(value);
  const std::byte* raw_bytestream = reinterpret_cast<const std::byte*>(v.data());
  return std::vector<std::byte> {raw_bytestream, raw_bytestream+v.size()};
}

template <>
short Field::as<short> () const { return std::stoi(std::get<std::string>(value)); }

template <>
int Field::as<int> () const { return std::stoi(std::get<std::string>(value)); }

template <>
float Field::as<float> () const { return std::stof(std::get<std::string>(value)); }

template <>
double Field::as<double> () const { return std::stod(std::get<std::string>(value)); }

template <>
long Field::as<long> () const
{
  if (metadata.flags & UNSIGNED_FLAG)
    throw std::runtime_error("Field holds unsigned value, use 'as<ulong>()' instead!");

  return std::stol(std::get<std::string>(value));
}

template <>
unsigned long Field::as<unsigned long> () const
{
  if (!(metadata.flags & UNSIGNED_FLAG))
    throw std::runtime_error("Field holds signed value, use 'as<long>()' instead!");

  return std::stoul(std::get<std::string>(value));
}

template <>
bool Field::as<bool> () const { return (!std::get<std::string>(value).empty() ? true:false); }

template <> std::string Field::as<std::string> () const { return std::get<std::string>(value); }

using chrono_timestamp = std::chrono::system_clock::time_point;
template <> chrono_timestamp Field::as<chrono_timestamp> () const
{
  int y, M, d, h, m, sec;
  int micro = 0;
  std::string s = std::get<std::string>(value);

  std::sscanf(s.data(), "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &sec);

  if (auto dot = s.find('.'); dot != std::string_view::npos)
  {
    micro = std::stoi(std::string(s.substr(dot + 1)));
  }

  std::tm tm{};
  tm.tm_year = y - 1900;
  tm.tm_mon  = M - 1;
  tm.tm_mday = d;
  tm.tm_hour = h;
  tm.tm_min  = m;
  tm.tm_sec  = sec;

  auto secs = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  return chrono_timestamp {secs + std::chrono::microseconds(micro)};
}

bool Field::is_null () { return std::holds_alternative<std::monostate>(value); }
const std::string& Field::as_string () { return std::get<std::string>(value); }

};//namespace mcxx
