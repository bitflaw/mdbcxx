#include <optional>
#include <mysql/mysql.h>
#include "field.hpp"

class Row {
public:
  Row(const Row&);
  Row(Row&&);
  Row (MYSQL_RES* res, MYSQL_ROW row);

  Field& operator[] (std::size_t);
  std::optional<Field> operator[] (std::string);

  /*
   template <typename... Types>
   std::tuple<Types> as_tuple(Row& row);
   * */

private:
  std::vector<Field> fields {};
  std::vector<std::string> col_names {};
};
