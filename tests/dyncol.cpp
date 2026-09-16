#include <chrono>
#include <cstdint>
#include <dyncol.hpp>
#include <connection.hpp>
#include <catch2/catch_test_macros.hpp>
#include <mariadb_dyncol.h>
#include <stdexcept>
#include <string>
using namespace std::string_literals;

std::string db = getenv("TDB_NAME");
std::string user = getenv("TDB_USR");
std::string pass = getenv("TDB_PASS");
std::string port_str = getenv("TDB_PORT");
uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
const char* host = getenv("TDB_HOST");

mcxx::Properties props {
  .host = host ? host : "127.0.0.1",
  .user = user,
  .passwd = pass,
  .db_name = db,
  .port = port,
};

mcxx::Connection cxn {props};

TEST_CASE("SINGLE ENTRY DYNAMIC COLUMN TESTS")
{
  mcxx::str_kv_pair str_pair {"ulong", mcxx::DynColVal_T {ulonglong (98)}};
  mcxx::num_kv_pair num_pair {1,       mcxx::DynColVal_T {ulonglong (98)}};

  mcxx::DynamicColumn str_db_dc {};
  mcxx::DynamicColumn num_db_dc {};

  // NOTE: INSERT AND COLUMN COUNT CHECK
  //
  REQUIRE(num_db_dc.column_count() == 0);
  REQUIRE_NOTHROW(num_db_dc.insert(num_pair, false, nullptr));
  REQUIRE(num_db_dc.column_count() == 1);

  REQUIRE(str_db_dc.column_count() == 0);
  REQUIRE_NOTHROW(str_db_dc.insert(str_pair, false, nullptr));
  REQUIRE(str_db_dc.column_count() == 1);
  //


  // NOTE: GET CHECK
  //
  REQUIRE(std::get<ulong>(str_db_dc.get("ulong").v) == 98);
  REQUIRE_NOTHROW(str_db_dc.update(mcxx::str_kv_pair{"ulong", mcxx::DynColVal_T {ulonglong (65)}}));
  REQUIRE(std::get<ulong>(str_db_dc.get("ulong").v) == 65);

  REQUIRE(std::get<ulong>(num_db_dc.get(1).v) == 98);
  REQUIRE_NOTHROW(num_db_dc.update(mcxx::num_kv_pair{1, mcxx::DynColVal_T {ulonglong (65)}}));
  REQUIRE(std::get<ulong>(num_db_dc.get(1).v) == 65);
  //


  // NOTE: UPDATE CHECK
  //
  REQUIRE_NOTHROW(str_db_dc.update(mcxx::str_kv_pair{"new_value", mcxx::DynColVal_T {ulonglong (55)}}));
  REQUIRE(str_db_dc.exists("new_value") == true);
  REQUIRE(std::get<ulong>(str_db_dc.get("new_value").v) == 55);
  REQUIRE(str_db_dc.column_count() == 2);

  REQUIRE_NOTHROW(num_db_dc.update(mcxx::num_kv_pair{2, mcxx::DynColVal_T {ulonglong (55)}}));
  REQUIRE(num_db_dc.exists(2) == true);
  REQUIRE(std::get<ulong>(num_db_dc.get(2).v) == 55);
  REQUIRE(num_db_dc.column_count() == 2);
  //


  // NOTE: REMOVE CHECK
  //
  REQUIRE_NOTHROW(str_db_dc.remove("ulong"));
  REQUIRE_FALSE(str_db_dc.exists("ulong"));

  REQUIRE_NOTHROW(num_db_dc.remove(2));
  REQUIRE_FALSE(num_db_dc.exists(2));

}

TEST_CASE("MULTIPLE ENTRIES DYNAMIC COLUMN TESTS")
{
  std::chrono::time_point now = std::chrono::system_clock::now();
  mcxx::Date date = std::chrono::year_month_day( std::chrono::floor<std::chrono::days>(now));
  auto t = std::chrono::duration_cast<std::chrono::microseconds>(now - std::chrono::floor<std::chrono::days>(now));
  mcxx::Time time = std::chrono::hh_mm_ss (t);
  mcxx::DateTime datetime = std::chrono::time_point_cast<std::chrono::seconds>(now);

  mcxx::str_kv_t str_kv_map {
    {"ulong", mcxx::DynColVal_T {ulonglong (98)}},
    {"double", mcxx::DynColVal_T {double (980)}},
    {"date", mcxx::DynColVal_T {date}},
    {"time", mcxx::DynColVal_T {time}},
    {"str", mcxx::DynColVal_T {std::string{"hello"}}},
    {"num_kv_t", mcxx::DynColVal_T { mcxx::num_kv_t { {1, mcxx::DynColVal_T { datetime } } }}},
  };
  uint32_t str_col_count = str_kv_map.size();

  mcxx::num_kv_t num_kv_map {
    {1, mcxx::DynColVal_T {ulonglong (98)}},
    {2, mcxx::DynColVal_T {double (980)}},
    {3, mcxx::DynColVal_T {date}},
    {4, mcxx::DynColVal_T {time}},
    {5, mcxx::DynColVal_T {std::string{"hello"}}},
    {6, mcxx::DynColVal_T { mcxx::num_kv_t { {1, mcxx::DynColVal_T { datetime } } }}},
  };
  uint32_t num_col_count = num_kv_map.size();

  mcxx::DynamicColumn str_db_dc {};
  mcxx::DynamicColumn num_db_dc {};

  // NOTE: INSERTS AND COLUMN COUNT CHECK
  //
  REQUIRE(num_db_dc.column_count() == 0);
  REQUIRE_NOTHROW(num_db_dc.insert(num_kv_map, false, nullptr));
  REQUIRE(num_db_dc.column_count() == num_col_count);

  REQUIRE(str_db_dc.column_count() == 0);
  REQUIRE_NOTHROW(str_db_dc.insert(str_kv_map, false, nullptr));
  REQUIRE(str_db_dc.column_count() == str_col_count);


  // NOTE: KEYS EXISTENCE CHECK
  //
  REQUIRE(str_db_dc.check_format()  == true);
  REQUIRE(str_db_dc.exists("ulong") == true);
  REQUIRE(str_db_dc.exists("long")  == false);

  REQUIRE(num_db_dc.check_format()  == true);
  REQUIRE(num_db_dc.exists(1) == true);
  REQUIRE(num_db_dc.exists(10)  == false);
  //

  // NOTE: SERIALIZATION AND JSON CHECK
  //
  REQUIRE_NOTHROW(str_db_dc.json());
  REQUIRE_NOTHROW(str_db_dc.serialize());
  REQUIRE_NOTHROW(num_db_dc.json());
  REQUIRE_NOTHROW(num_db_dc.serialize());
  //


  // NOTE: KEYS CHECK
  //
  REQUIRE(str_db_dc.uses_named_keys() == true);
  std::vector<std::string> str_likely_keys {"str", "date", "time", "ulong", "double", "num_kv_t"};
  REQUIRE_THROWS(str_db_dc.num_keys());
  REQUIRE_NOTHROW(str_db_dc.str_keys());
  REQUIRE(str_db_dc.str_keys() == str_likely_keys);

  REQUIRE_FALSE(num_db_dc.uses_named_keys());
  std::vector<uint32_t> num_likely_keys {1,2,3,4,5,6};
  // REQUIRE_THROWS(num_db_dc.str_keys());
  REQUIRE_NOTHROW(num_db_dc.num_keys());
  REQUIRE(num_db_dc.num_keys() == num_likely_keys);
  //


  REQUIRE_NOTHROW(str_db_dc.unpack());
  // REQUIRE(db_dc.unpack() == kv);


  // NOTE: GET CHECK
  //
  REQUIRE(std::get<mcxx::Date>(str_db_dc.get("date").v) == date);
  REQUIRE_THROWS_AS(str_db_dc.get("invalid_key"), std::invalid_argument);

  REQUIRE(std::get<mcxx::Date>(num_db_dc.get(3).v) == date);
  REQUIRE_THROWS_AS(str_db_dc.get(10000), std::invalid_argument);

  REQUIRE(str_db_dc.compare_keys("ulong", "ulong") == std::nullopt);
  REQUIRE(str_db_dc.compare_keys("ulong", "str").value() == true);
  REQUIRE_FALSE(str_db_dc.compare_keys("str", "ulong").value());
  //


  // NOTE: UPDATE CHECK
  //
  REQUIRE(std::get<std::string>(str_db_dc.get("str").v) == "hello");
  mcxx::str_kv_t str_kv_updates {
    {"str", mcxx::DynColVal_T {"goodbye"}},
  };
  REQUIRE_NOTHROW(str_db_dc.update(str_kv_updates));
  REQUIRE(std::get<std::string>(str_db_dc.get("str").v) == "goodbye");

  REQUIRE(std::get<std::string>(num_db_dc.get(5).v) == "hello");
  mcxx::num_kv_t num_kv_updates {
    {5, mcxx::DynColVal_T {"goodbye"}},
  };
  REQUIRE_NOTHROW(num_db_dc.update(num_kv_updates));
  REQUIRE(std::get<std::string>(num_db_dc.get(5).v) == "goodbye");
  //


  // NOTE: REMOVE CHECK
  //
  REQUIRE(str_db_dc.exists("str") == true);
  REQUIRE(str_db_dc.exists("double") == true);
  REQUIRE_NOTHROW(str_db_dc.remove(std::vector<std::string>{"str", "double"}));
  REQUIRE_FALSE(str_db_dc.exists("str"));
  REQUIRE_FALSE(str_db_dc.exists("double"));

  REQUIRE(num_db_dc.exists(4) == true);
  REQUIRE(num_db_dc.exists(5) == true);
  REQUIRE_NOTHROW(num_db_dc.remove(std::vector<uint32_t>{4,5}));
  REQUIRE_FALSE(num_db_dc.exists(4));
  REQUIRE_FALSE(num_db_dc.exists(5));
  //

}
