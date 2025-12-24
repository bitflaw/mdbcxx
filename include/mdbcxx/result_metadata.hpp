#include <cstdint>
#include <mysql/mysql.h>
#include <string>
#include <vector>

namespace mcxx
{
class ResultMetadata
{
public:
  std::string db_name {};
  uint32_t num_rows {};
  uint32_t num_cols {};
  std::vector<MYSQL_FIELD> columns {};

  ResultMetadata () = default;
  ResultMetadata (const ResultMetadata&);
  ResultMetadata (ResultMetadata&&);
  ResultMetadata (MYSQL_RES*);

  ResultMetadata& operator= (ResultMetadata&);
  ResultMetadata& operator= (ResultMetadata&&);

  ~ResultMetadata() = default;
};
};
