#include <mysql/mysql.h>
#include <variant>
#include <vector>
#include "field_metadata.hpp"

using field_value_t = std::variant<std::monostate, std::string, std::vector<uint8_t>>;

class Field {

public:

  Field(MYSQL_FIELD*, char*);
  Field(const Field&);
  Field(Field&&);

  Field& operator= (Field&);
  Field& operator= (Field&&);
  void operator() (Field& field);
  void operator() (Field&& field);

  template<typename T> T& as ();
  bool is_null ();
  bool is_string ();
  bool is_blob ();
  const std::string& as_string ();
  const std::vector<uint8_t>& as_blob ();

  ~Field() = default;

private:
  field_value_t value {std::monostate {}};
  FieldMetadata metadata {};
};
