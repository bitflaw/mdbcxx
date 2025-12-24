#include <catch2/catch_test_macros.hpp>
#include "../include/mdbcxx/params.hpp"

TEST_CASE("Params testing")
{
  REQUIRE_NOTHROW(mcxx::params {mcxx::sqlstringT{mcxx::sql_string_types::VARCHAR, "alice"}, uint8_t {34}});

  mcxx::params p {};
  p.reserve(2);
  REQUIRE(p.raw().capacity() >= 2);

  REQUIRE_NOTHROW(p.append({mcxx::sql_string_types::TEXT, "alice"}));
  REQUIRE_NOTHROW(p.append(43));
  REQUIRE(p.size() == 2);
  p.clear();
  REQUIRE_NOTHROW(p.append_multi(mcxx::sqlstringT {mcxx::sql_string_types::VARCHAR, "alice"}, uint8_t {43}));
  REQUIRE(p.size() == 2);
  p.clear();

  mcxx::params p1 {mcxx::sqlstringT{mcxx::sql_string_types::VARCHAR, "alice"}, uint8_t {34}};
  p.reserve(3);
  REQUIRE_NOTHROW(p.append_multi(p1, 23.32));
  REQUIRE(p.size() == 3);
}
