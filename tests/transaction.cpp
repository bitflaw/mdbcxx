#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include "../include/mdbcxx/transaction.hpp"

TEST_CASE("TRANSACTION TESTS")
{

  std::string db = getenv("TDB_NAME");
  std::string user = getenv("TDB_USR");
  std::string pass = getenv("TDB_PASS");
  std::string port = getenv("TDB_PORT");

  Properties props {};
  props.user = user;
  props.passwd = pass;
  props.db_name = db;
  props.port = (short) std::stoi(port);

  Connection cxn {props};
  REQUIRE_NOTHROW(Transaction {cxn});
  Transaction txn {cxn};
  REQUIRE_NOTHROW(Transaction {txn});

  std::string sql = "select 'alice' as name, 32 as age;";

  SECTION("Normal SQL")
  {
    std::optional<Result> res = txn.exec(sql);
    REQUIRE(res.has_value());
    REQUIRE(res.value().size() == 1);
    REQUIRE(res.value()[0][0].as_string() == "alice");
    REQUIRE(res.value()[0][1].as<int>() == 32);

    Row row = txn.exec1(sql);
    REQUIRE(row[0].as_string() == "alice");
    REQUIRE(row[1].as<int>() == 32);

    REQUIRE_THROWS_AS(txn.exec0(sql), std::length_error);

    std::optional<Result> res2 = txn.execn(1, sql);
    REQUIRE(res2.has_value());
    REQUIRE(res2.value().size() == 1);
    REQUIRE(res2.value()[0][0].as_string() == "alice");
    REQUIRE(res2.value()[0][1].as<int>() == 32);
  }

  std::string prep_sql = "select ? as name, ? as age;";
  cxn.prepare("select_stmt", prep_sql);
  params p {sqlstringT {sql_string_types::VARCHAR, "alice"}, 32};

  SECTION("Prepared SQL")
  {
    std::optional<Result> res = txn.exec(cxn.prepped("select_stmt"), p);
    REQUIRE(res.has_value());
    REQUIRE(res.value().size() == 1);
    REQUIRE(res.value()[0][0].as_string() == "alice");
    REQUIRE(res.value()[0][1].as<int>() == 32);

    Row row = txn.exec1(cxn.prepped("select_stmt"), p);
    REQUIRE(row[0].as_string() == "alice");
    REQUIRE(row[1].as<int>() == 32);

    REQUIRE_THROWS_AS(txn.exec0(cxn.prepped("select_stmt"), p), std::length_error);

    std::optional<Result> res2 = txn.execn(1, cxn.prepped("select_stmt"), p);
    REQUIRE(res2.has_value());
    REQUIRE(res2.value().size() == 1);
    REQUIRE(res2.value()[0][0].as_string() == "alice");
    REQUIRE(res2.value()[0][1].as<int>() == 32);
  }
}
