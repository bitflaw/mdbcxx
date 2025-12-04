#include <catch2/catch_test_macros.hpp>
#include "../include/mdbcxx/params.hpp"

TEST_CASE("Params testing")
{
  REQUIRE_NOTHROW(params {sqlstringT{sql_string_types::VARCHAR, "alice"}, uint8_t {34}});

  params p {};
  p.reserve(2);
  REQUIRE(p.raw().capacity() >= 2);

  REQUIRE_NOTHROW(p.append({sql_string_types::TEXT, "alice"}));
  REQUIRE_NOTHROW(p.append(43));
  REQUIRE(p.size() == 2);
  p.clear();
  REQUIRE_NOTHROW(p.append_multi(sqlstringT {sql_string_types::VARCHAR, "alice"}, uint8_t {43}));
  REQUIRE(p.size() == 2);
  p.clear();

  params p1 {sqlstringT{sql_string_types::VARCHAR, "alice"}, uint8_t {34}};
  p.reserve(3);
  REQUIRE_NOTHROW(p.append_multi(p1, 23.32));
  REQUIRE(p.size() == 3);
}
