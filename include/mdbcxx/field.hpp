#pragma once
#include <mysql.h>
#include <variant>
#include "field_metadata.hpp"

using field_value_t = std::variant<std::monostate, std::string>;

namespace mcxx {
class Field {

public:

  Field(MYSQL_FIELD*, char*, ulong);
  Field(const Field&);
  Field(Field&&);

  Field& operator= (const Field&);
  Field& operator= (Field&&);

  template<typename T> T as () const;
  bool is_null ();
  const std::string& as_string ();

  ~Field() = default;

private:
  field_value_t value {std::monostate {}};
  FieldMetadata metadata {};
};

};//namespace mcxx
