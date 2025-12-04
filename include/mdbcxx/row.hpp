#include <optional>
#include <mysql/mysql.h>
#include "field.hpp"

class Row {
public:
  Row(const Row&);
  Row(Row&&);
  Row (MYSQL_FIELD* farray,std::size_t nfarray, MYSQL_ROW row, unsigned long* lengths);
  Row (MYSQL_FIELD* fields, std::vector<MYSQL_BIND>& bound_row);

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
