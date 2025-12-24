#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include "../include/mdbcxx/connection.hpp"

TEST_CASE("CONNECTION TESTS")
{
  std::string db = getenv("TDB_NAME");
  std::string user = getenv("TDB_USR");
  std::string pass = getenv("TDB_PASS");
  std::string port = getenv("TDB_PORT");

  mcxx::Properties props {};
  props.user = user;
  props.passwd = pass;
  props.db_name = db;
  props.port = (short) std::stoi(port);

  REQUIRE_NOTHROW(mcxx::Connection {props});
  REQUIRE_THROWS(mcxx::Connection {user, pass, db});

  mcxx::Connection cxn {};
  REQUIRE_NOTHROW(cxn.connect(props));
  REQUIRE_FALSE(cxn.raw() == NULL);

  REQUIRE(cxn.is_connected() == true);
  REQUIRE(cxn.ping() == true);
  REQUIRE(cxn.reset() == true);

  std::string sql = "select ? as name, ? as age;";
  REQUIRE_NOTHROW(cxn.prepare("select_stmt", sql));
  REQUIRE(cxn.prepped("select_stmt").param_count == 2);
  REQUIRE_THROWS_AS(cxn.prepped("invalid_name"), std::invalid_argument);

  cxn.close();
  REQUIRE(cxn.is_connected() == false);
  REQUIRE(cxn.ping() == false);
}
