#include <cstddef>
#include <mysql/mysql.h>
#include <variant>
#include <vector>
#include "field_metadata.hpp"

using field_value_t = std::variant<std::monostate, std::string, std::vector<std::byte>>;

class Field {

public:

  Field(MYSQL_FIELD*, char*, ulong);
  Field(const Field&);
  Field(Field&&);

  Field& operator= (Field&);
  Field& operator= (Field&&);
  void operator() (Field& field);
  void operator() (Field&& field);

  template<typename T> T as () const;
  bool is_null ();
  const std::string& as_string ();
  const std::vector<std::byte>& as_blob ();

  ~Field() = default;

private:
  field_value_t value {std::monostate {}};
  FieldMetadata metadata {};
};
