#include <stdexcept>
#include <chrono>
#include <cstddef>
#include <variant>
#include <field.hpp>
#include <dyncol.hpp>


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

  if (raw == nullptr) value = std::monostate {};
  else                value = std::string {raw, len};
}


// WARN: probably should use reinterpret_cast since this can fail for types not
// convertible to and fro string.
template <typename T>
T Field::as () const { return static_cast<T>(std::get<std::string>(value)); }

using BLOB_T = std::vector<std::byte>;
template <> BLOB_T Field::as<BLOB_T> () const
{
  std::string v = std::get<std::string>(value);
  const std::byte* bytestream = reinterpret_cast<const std::byte*>(v.data());
  return std::vector<std::byte> {bytestream, bytestream+v.size()};
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

// WARN: might be wrong
template <>
bool Field::as<bool> () const { return (!std::get<std::string>(value).empty() ? true:false); }

template <> std::string Field::as<std::string> () const { return std::get<std::string>(value); }

template <>
DynamicColumn Field::as<DynamicColumn> () const
{
  std::string v = std::get<std::string>(value);
  return DynamicColumn {
    DYNAMIC_COLUMN {
      .str = v.data(),
      .length = v.length(),
      .max_length = v.max_size(),
      .alloc_increment = 0
    }
  };
}

using chrono_timestamp = std::chrono::system_clock::time_point;
template <>
chrono_timestamp Field::as<chrono_timestamp>() const
{
    const std::string& s = std::get<std::string>(value);
    std::istringstream ss{s};

    std::chrono::sys_time<std::chrono::microseconds> tp {};
    if (ss >> std::chrono::parse("%F %T", tp) ||
        (ss.clear(), ss.str(s), ss >> std::chrono::parse("%FT%T", tp))
        )
    {
        return std::chrono::time_point_cast<chrono_timestamp::duration>(tp);
    }

    ss.clear();
    ss.str(s);
    std::chrono::year_month_day date {};
    if (ss >> std::chrono::parse("%F", date))
    {
        auto days = std::chrono::sys_days{date};
        return std::chrono::time_point_cast<chrono_timestamp::duration>(days);
    }

    throw std::runtime_error("Failed to parse timestamp string: " + s);
}

bool Field::is_null () { return std::holds_alternative<std::monostate>(value); }
const std::string& Field::as_string () { return std::get<std::string>(value); }

};//namespace mcxx
