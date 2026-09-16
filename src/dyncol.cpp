#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dyncol.hpp>
#include <mariadb_ctype.h>
#include <mariadb_dyncol.h>
#include <mysql.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

namespace mcxx
{

DyncolError translate_dyncol_error(int rc)
{
  switch (rc)
  {
    case ER_DYNCOL_FORMAT:
      return DyncolError::FORMAT;
    case ER_DYNCOL_LIMIT:
      return DyncolError::LIMIT;
    case ER_DYNCOL_RESOURCE:
      return DyncolError::RESOURCE;
    case ER_DYNCOL_DATA:
      return DyncolError::DATA;
    case ER_DYNCOL_UNKNOWN_CHARSET:
      return DyncolError::UNKNOWN_CHARSET;
    default:
      throw std::logic_error("Unexpected dynamic-column error");
  }
}

DynamicColumn::DynamicColumn ()
{
  mariadb_dyncol_init(&dyncol);
}

DynamicColumn::DynamicColumn (DYNAMIC_COLUMN dc)
{
  dyncol.length          = dc.length;
  dyncol.max_length      = dc.max_length;
  dyncol.alloc_increment = dc.alloc_increment;
  dyncol.str = static_cast<char*>(std::malloc(dc.length));
  std::memcpy(dyncol.str, dc.str, dyncol.length);
}

DynamicColumn::~DynamicColumn ()
{
  mariadb_dyncol_free(&dyncol);
}

DynamicColumn::DynamicColumn (DynamicColumn&& other)
{
  dyncol.length          = std::move(other.dyncol.length);
  dyncol.max_length      = std::move(other.dyncol.max_length);
  dyncol.alloc_increment = std::move(other.dyncol.alloc_increment);
  dyncol.str = static_cast<char*>(std::malloc(dyncol.length));
  std::memcpy(dyncol.str, other.dyncol.str, dyncol.length);
  mariadb_dyncol_free(&other.dyncol);
}

DynamicColumn::DynamicColumn (const DynamicColumn& other)
{
  if (other.dyncol.str == nullptr || other.dyncol.length == 0)
  {
    mariadb_dyncol_init(&dyncol);
    return;
  }
  dyncol.length          = other.dyncol.length;
  dyncol.max_length      = other.dyncol.max_length;
  dyncol.alloc_increment = other.dyncol.alloc_increment;
  dyncol.str = static_cast<char*>(std::malloc(dyncol.length));
  std::memcpy(dyncol.str, other.dyncol.str, dyncol.length);
}

DynamicColumn& DynamicColumn::operator=(DynamicColumn&& other)
{
  if (this == &other) return *this;
  mariadb_dyncol_free(&dyncol);
  mariadb_dyncol_init(&dyncol);
  dyncol.length          = std::move(other.dyncol.length);
  dyncol.max_length      = std::move(other.dyncol.max_length);
  dyncol.alloc_increment = std::move(other.dyncol.alloc_increment);
  dyncol.str = static_cast<char*>(std::malloc(dyncol.length));
  std::memcpy(dyncol.str, other.dyncol.str, dyncol.length);
  mariadb_dyncol_free(&other.dyncol);
  return *this;
}

DynamicColumn& DynamicColumn::operator=(const DynamicColumn& other)
{
  if (this == &other) return *this;
  mariadb_dyncol_free(&dyncol);
  if (other.dyncol.str == nullptr || other.dyncol.length == 0)
  {
    mariadb_dyncol_init(&dyncol);
    return *this;
  }
  dyncol.length          = other.dyncol.length;
  dyncol.max_length      = other.dyncol.max_length;
  dyncol.alloc_increment = other.dyncol.alloc_increment;
  dyncol.str = static_cast<char*>(std::malloc(dyncol.length));
  std::memcpy(dyncol.str, other.dyncol.str, dyncol.length);
  return *this;
}

bool DynamicColumn::check_format ()
{
  int retval = mariadb_dyncol_check(&dyncol);
  if (retval != ER_DYNCOL_OK) return false;
  return true;
}

uint32_t DynamicColumn::column_count (DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  uint32_t column_count {0};
  int rc = mariadb_dyncol_column_count(dc_ptr, &column_count);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] column_count(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
  return column_count;
}

bool DynamicColumn::exists (uint32_t key)
{
  int retval = mariadb_dyncol_exists_num(&dyncol, key);
  return (retval == ER_DYNCOL_YES) ? true : false;
}

bool DynamicColumn::exists (std::string key)
{
  MYSQL_LEX_STRING str {
    .str = key.data(),
    .length = key.length()
  };
  int retval = mariadb_dyncol_exists_named(&dyncol, &str);
  return (retval == ER_DYNCOL_YES) ? true : false;
}

bool DynamicColumn::uses_named_keys (DYNAMIC_COLUMN* dc)
{
  if (dc != nullptr)
    return static_cast<bool>(mariadb_dyncol_has_names(dc));
  return static_cast<bool>(mariadb_dyncol_has_names(&dyncol));
}

std::string DynamicColumn::json ()
{
  DYNAMIC_STRING json_str {};
  int rc = mariadb_dyncol_json(&dyncol, &json_str);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] json(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
  return std::string { json_str.str, json_str.length };
}

std::vector<std::string> DynamicColumn::str_keys (DYNAMIC_COLUMN *dc)
{
  DYNAMIC_COLUMN *dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  uint32_t col_count = column_count (dc_ptr);
  std::vector<std::string> keys {}; keys.reserve(col_count);
  MYSQL_LEX_STRING* col_keys = nullptr;
  int rc = mariadb_dyncol_list_named(dc_ptr, &col_count, &col_keys);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] str_keys(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );

  for (uint32_t i{0}; i < col_count; i++)
  {
    MYSQL_LEX_STRING str = col_keys[i];
    if (str.str || str.length > 0)
      keys.emplace_back(std::string {str.str, str.length});
    else
      keys.emplace_back(std::string {});
  }
  if (col_keys) free(col_keys);
  return keys;
}

std::vector<uint32_t> DynamicColumn::num_keys (DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN *dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  uint32_t col_count = column_count (dc_ptr);
  uint32_t* keys_ptr = nullptr;
  int rc = mariadb_dyncol_list_num(dc_ptr, &col_count, &keys_ptr);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] num_keys(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
  std::vector<uint32_t> keys {keys_ptr, keys_ptr + col_count};
  if (keys_ptr) free(keys_ptr);
  return keys;
}

str_kv_t DynamicColumn::unpack (DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;

  uint32_t col_count = column_count (dc_ptr);
  MYSQL_LEX_STRING* keys = nullptr;
  DYNAMIC_COLUMN_VALUE* vals = nullptr;

  int rc = mariadb_dyncol_unpack(dc_ptr, &col_count, &keys, &vals);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] unpack(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );

  str_kv_t kv {};
  for (uint32_t i {0}; i < col_count; i++)
    kv.insert({
        std::string {keys[i].str, keys[i].length},
        to_dyncol_val_t(vals[i])
        });

  if (keys) free(keys);
  if (vals) free(vals);

  return kv;
}

DynColVal_T DynamicColumn::get (std::string key, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  LEX_STRING str { key.data(), key.length() };
  DYNAMIC_COLUMN_VALUE v {}; mariadb_dyncol_value_init(&v);
  int rc = mariadb_dyncol_get_named(dc_ptr, &str, &v);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] get(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
  if (v.type == DYN_COL_NULL)
    throw std::invalid_argument ("[ERROR] get(): Invalid key!");
  return to_dyncol_val_t(v);
}

DynColVal_T DynamicColumn::get (uint32_t key, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  DYNAMIC_COLUMN_VALUE v {}; mariadb_dyncol_value_init(&v);
  int rc = mariadb_dyncol_get_num(dc_ptr, key, &v);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] get(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
  if (v.type == DYN_COL_NULL)
    throw std::invalid_argument ("[ERROR] get(): Invalid key!");
  return to_dyncol_val_t(v);
}

std::optional<bool> DynamicColumn::compare_keys (std::string key1, std::string key2)
{
  LEX_STRING k1 {
    .str = key1.data(),
      .length = key1.length()
  };
  LEX_STRING k2 {
    .str = key2.data(),
      .length = key2.length()
  };
  int retval = mariadb_dyncol_column_cmp_named(&k1, &k2);
  if (retval < 0) return false;
  else if (retval > 0) return true;
  else return std::nullopt;
}

void DynamicColumn::insert (str_kv_pair kv_pair, bool rst, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  // if (rst) mariadb_dyncol_free(dc_ptr);
  MYSQL_LEX_STRING key { const_cast<char*>(kv_pair.first.data()), kv_pair.first.length()};
  DYNAMIC_COLUMN_VALUE val {to_dyncol_val(kv_pair.second)};

  int rc = mariadb_dyncol_create_many_named(dc_ptr, 1, &key, &val, rst);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] insert(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::insert (num_kv_pair kv_pair, bool rst, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  // if (rst) mariadb_dyncol_free(dc_ptr);
  DYNAMIC_COLUMN_VALUE val {to_dyncol_val(kv_pair.second)};
  int rc = mariadb_dyncol_create_many_num(dc_ptr, 1, &kv_pair.first, &val, rst);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] insert(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::insert (str_kv_t kv, bool rst, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  // if (rst) mariadb_dyncol_free(dc_ptr);
  uint32_t col_count = kv.size();
  std::vector<MYSQL_LEX_STRING> keys {}; keys.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> vals {}; vals.reserve(col_count);

  for (auto& [k, v] : kv)
  {
    keys.emplace_back(MYSQL_LEX_STRING { const_cast<char*>(k.data()), k.length()});
    vals.emplace_back(to_dyncol_val(v));
  }
  int rc = mariadb_dyncol_create_many_named(dc_ptr, col_count, keys.data(), vals.data(), rst);

  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] insert(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::insert (num_kv_t kv, bool rst, DYNAMIC_COLUMN* dc)
{
  DYNAMIC_COLUMN* dc_ptr = &dyncol;
  if (dc != nullptr) dc_ptr = dc;
  uint32_t col_count = kv.size();
  std::vector<uint32_t> keys {}; keys.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> vals {}; vals.reserve(col_count);

  for (auto& [k, v] : kv)
  {
    keys.emplace_back(k);
    vals.emplace_back(to_dyncol_val(v));
  }
  int rc = mariadb_dyncol_create_many_num(dc_ptr, col_count, keys.data(), vals.data(), rst);

  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] insert(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::update (str_kv_pair kv_pair)
{
  MYSQL_LEX_STRING key { const_cast<char*>(kv_pair.first.data()), kv_pair.first.length()};
  DYNAMIC_COLUMN_VALUE val {to_dyncol_val(kv_pair.second)};
  int rc = mariadb_dyncol_update_many_named(&dyncol, 1, &key, &val);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] update(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::update (num_kv_pair kv_pair)
{
  DYNAMIC_COLUMN_VALUE val {to_dyncol_val(kv_pair.second)};
  int rc = mariadb_dyncol_update_many_num(&dyncol, 1, &kv_pair.first, &val);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] update(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::update (str_kv_t kv)
{
  uint32_t col_count = kv.size();
  std::vector<MYSQL_LEX_STRING> keys {}; keys.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> vals {}; vals.reserve(col_count);
  for (auto& [k, v] : kv)
  {
    keys.emplace_back(MYSQL_LEX_STRING { const_cast<char*>(k.data()), k.length()});
    vals.emplace_back(to_dyncol_val(v));
  }
  int rc = mariadb_dyncol_update_many_named(&dyncol, col_count, keys.data(), vals.data());
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] update(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::update (num_kv_t kv)
{
  uint32_t col_count = kv.size();
  std::vector<uint32_t> keys {}; keys.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> vals {}; vals.reserve(col_count);
  for (auto& [k, v] : kv)
  {
    keys.emplace_back(k);
    vals.emplace_back(to_dyncol_val(v));
  }
  int rc = mariadb_dyncol_update_many_num(&dyncol, col_count, keys.data(), vals.data());
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] update(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::remove (std::string key)
{
  MYSQL_LEX_STRING k_str { key.data(), key.length()};
  DYNAMIC_COLUMN_VALUE v {}; mariadb_dyncol_value_init(&v);
  int rc = mariadb_dyncol_update_many_named(&dyncol, 1, &k_str, &v);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] remove(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::remove (uint32_t key)
{
  DYNAMIC_COLUMN_VALUE v {}; mariadb_dyncol_value_init(&v);
  int rc = mariadb_dyncol_update_many_num(&dyncol, 1, &key, &v);
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] remove(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::remove (std::vector<std::string> keys)
{
  uint32_t col_count = keys.size();
  std::vector<MYSQL_LEX_STRING> k {}; k.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> v {col_count};
  for (uint32_t i {0}; i < col_count; i++)
  {
    k.emplace_back(MYSQL_LEX_STRING { keys[i].data(), keys[i].length()});
    mariadb_dyncol_value_init(&v[i]);
  }
  int rc = mariadb_dyncol_update_many_named(&dyncol, col_count, k.data(), v.data());
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] remove(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

void DynamicColumn::remove (std::vector<uint32_t> keys)
{
  uint32_t col_count = keys.size();
  std::vector<uint32_t> k {}; k.reserve(col_count);
  std::vector<DYNAMIC_COLUMN_VALUE> v {col_count};
  for (uint32_t i {0}; i < col_count; i++)
  {
    k.emplace_back(keys[i]);
    mariadb_dyncol_value_init(&v[i]);
  }
  int rc = mariadb_dyncol_update_many_num(&dyncol, col_count, keys.data(), v.data());
  if (rc != ER_DYNCOL_OK)
    throw std::runtime_error (
        "[ERROR] remove(): " + dyncol_strerr.at(translate_dyncol_error(rc))
        );
}

std::vector<std::byte> DynamicColumn::serialize ()
{
  std::byte* bytestream = reinterpret_cast<std::byte*>(dyncol.str);
  return std::vector<std::byte> {bytestream, bytestream + dyncol.length};
}


// PRIVATE FUNCTION DEFINITIONS

// WARN: check over the time stuff, for the second_part and neg. might need to increase precision to fill it.

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
DYNAMIC_COLUMN_VALUE DynamicColumn::to_dyncol_val (DynColVal_T& cval)
{
  DYNAMIC_COLUMN_VALUE val {}; mariadb_dyncol_value_init(&val);
  std::visit(overload {
      [&](longlong& v)
      {
        val.type = DYN_COL_INT;
        val.x.long_value = v;
      },
      [&](ulonglong& v)
      {
        val.type = DYN_COL_UINT;
        val.x.ulong_value = v;
      },

      // NOTE: DOCS: `DYN_COL_DECIMAL` type is not supported in MariaDB Connector/C yet,
      // but in MariaDB Server >= 5.3
      //
      // [&](float v)
      // {
      //   std::string x = std::to_string(v);
      //   val.type = DYN_COL_DECIMAL;
      //   val.x.decimal.buffer = x.data();
      // },

      [&](double& v)
      {
        val.type = DYN_COL_DOUBLE;
        val.x.double_value = v;
      },
      [&](std::string& v)
      {
        MYSQL_LEX_STRING x {v.data(), v.length()};
        val.type = DYN_COL_STRING;
        val.x.string = {x, mariadb_get_charset_by_name ("utf8")};
      },
      [&](Date& v)
      {
        MYSQL_TIME ts {
          .year = static_cast<uint32_t>(int32_t (v.year())),
          .month = uint32_t (v.month()),
          .day = uint32_t (v.day()),
          .hour = 0, .minute=0, .second=0, .second_part=0,
          .neg = my_bool(false),
          .time_type = MYSQL_TIMESTAMP_DATE
        };
        val.type = DYN_COL_DATE;
        val.x.time_value = ts;
      },
      [&](Time& v)
      {
        MYSQL_TIME ts {
          .year=0, .month=0, .day=0,
          .hour = static_cast<uint32_t>(v.hours().count()),
          .minute = uint32_t (v.minutes().count()),
          .second = uint32_t (v.seconds().count()),
          .second_part = uint32_t (v.subseconds().count()),
          .neg = my_bool(v.is_negative()),
          .time_type = MYSQL_TIMESTAMP_TIME,
        };
        val.type = DYN_COL_TIME;
        val.x.time_value = ts;
      },
      [&](DateTime& v)
      {
        auto sys_day = std::chrono::floor<std::chrono::days>(v);
        Date d {sys_day};
        Time t {v - sys_day};
        MYSQL_TIME dt {
          .year = static_cast<uint32_t>(int32_t (d.year())),
          .month = uint32_t (d.month()),
          .day = uint32_t (d.day()),
          .hour = static_cast<uint32_t>(t.hours().count()),
          .minute = uint32_t (t.minutes().count()),
          .second = uint32_t (t.seconds().count()),
          .second_part = uint32_t (t.subseconds().count()),
          .neg = my_bool(false),
          .time_type = MYSQL_TIMESTAMP_DATETIME,
        };
        val.type = DYN_COL_DATETIME;
        val.x.time_value = dt;
      },
      [&](str_kv_t& v)
      {
        DYNAMIC_COLUMN dc {}; mariadb_dyncol_init(&dc);
        insert(v, false, &dc);
        val.type = DYN_COL_DYNCOL;
        val.x.string = {
          .value = {.str = dc.str, .length = dc.length },
          .charset = mariadb_get_charset_by_name ("utf8")
        };
      },
      [&](num_kv_t& v)
      {
        DYNAMIC_COLUMN dc {}; mariadb_dyncol_init(&dc);
        insert(v, false, &dc);
        val.type = DYN_COL_DYNCOL;
        val.x.string = {
          .value = {.str = dc.str, .length = dc.length },
          .charset = mariadb_get_charset_by_name ("utf8")
        };
      }
    },
  cval.v);
  return val;
}

DynColVal_T DynamicColumn::to_dyncol_val_t (DYNAMIC_COLUMN_VALUE& val)
{
  DynColVal_T cval {};

  switch (val.type)
  {
    case DYN_COL_INT:
      cval.v = longlong {static_cast<longlong>(val.x.long_value)};
      break;
    case DYN_COL_UINT:
      cval.v = ulonglong {static_cast<ulonglong>(val.x.ulong_value)};
      break;

    // NOTE: DOCS: `DYN_COL_DECIMAL` type is not supported in MariaDB Connector/C yet,
    // but in MariaDB Server >= 5.3
    //
    // case DYN_COL_DECIMAL:
    //   cval.v = float {static_cast<float>(val.x.decimal.value)};
    //   break;

    case DYN_COL_DOUBLE:
      cval.v = double {static_cast<double>(val.x.double_value)};
      break;
    case DYN_COL_STRING:
      {
        auto* p = val.x.string.value.str;
        auto n  = val.x.string.value.length;

        if (n > 0 && p[n - 1] == '\0') --n;
        cval.v = std::string {p, n};
        break;
      }
    case DYN_COL_DATE:
      {
        MYSQL_TIME ts = val.x.time_value;
        std::chrono::day d {ts.day};
        std::chrono::month m {ts.month};
        std::chrono::year y {static_cast<int32_t>(ts.year)};
        cval.v = Date {y,  m, d};
        break;
      }
    case DYN_COL_TIME:
      {
        MYSQL_TIME ts = val.x.time_value;
        auto time_duration = std::chrono::hours {ts.hour} +
          std::chrono::minutes {ts.minute} +
          std::chrono::seconds {ts.second} +
          std::chrono::microseconds {ts.second_part};
        cval.v = Time {(ts.neg) ? -time_duration: time_duration};
        break;
      }
    case DYN_COL_DATETIME:
      {
        MYSQL_TIME ts = val.x.time_value;
        std::chrono::year_month_day date {
          std::chrono::year {static_cast<int32_t>(ts.year)},
            std::chrono::month {ts.month},
            std::chrono::day {ts.day}
        };
        auto time_duration = std::chrono::hours {ts.hour} +
          std::chrono::minutes {ts.minute} +
          std::chrono::seconds {ts.second};
        cval.v = DateTime { std::chrono::sys_days{date} + time_duration };
        break;
      }
    case DYN_COL_DYNCOL:
      {
        MYSQL_LEX_STRING str = val.x.string.value;
        DYNAMIC_COLUMN dc {}; mariadb_dyncol_init(&dc);
        dc.length = str.length;
        dc.max_length = str.length;
        dc.str = static_cast<char*>(std::malloc(str.length));
        std::memcpy(dc.str, str.str, str.length);

        if (uses_named_keys(&dc))
        {
          str_kv_t kv = unpack(&dc);
          cval.v = kv;
        } else
        {
          num_kv_t kv {};
          auto keys = num_keys(&dc);
          for (uint32_t key: keys)
            kv.insert({key, get(key, &dc)});
          cval.v = kv;
        }

        mariadb_dyncol_free(&dc);
        break;
      };
    default:
      break;
  }
  return cval;
}

}
