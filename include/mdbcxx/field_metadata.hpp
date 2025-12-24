#include <mysql/mysql.h>
#include <cstdint>
#include <string>

namespace mcxx {
class FieldMetadata {
public:
  std::string name {};
  std::string default_value {};
  uint32_t length {};
  uint32_t max_length {};
  uint32_t flags {};
  uint32_t decimals {};
  enum_field_types sql_type {};

  FieldMetadata () = default;
  FieldMetadata (const FieldMetadata&);
  FieldMetadata (FieldMetadata&&);
  FieldMetadata (MYSQL_FIELD* fmeta);

  FieldMetadata& operator= (const FieldMetadata&);
  FieldMetadata& operator= (FieldMetadata&&);

  ~FieldMetadata ()= default;
};

};//namespace mcxx
