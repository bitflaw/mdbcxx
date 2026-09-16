#pragma once
#include <chrono>
#include <cstdint>
#include <mysql.h>
#include <mariadb_dyncol.h>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace mcxx
{
  enum class DyncolError
  {
    FORMAT,
    LIMIT,
    RESOURCE,
    DATA,
    UNKNOWN_CHARSET
  };
  const std::unordered_map<DyncolError, std::string> dyncol_strerr
  {
    {DyncolError::FORMAT,          "Wrong format of the encoded string"},
    {DyncolError::LIMIT,           "A limit of implementation reached"},
    {DyncolError::RESOURCE,        "Out of resources"},
    {DyncolError::DATA,            "Incorrect input data"},
    {DyncolError::UNKNOWN_CHARSET, "Unknown character set"}
  };

  DyncolError translate_dyncol_error(int rc);


  using Date     = std::chrono::year_month_day;
  using Time     = std::chrono::hh_mm_ss<std::chrono::microseconds>;
  using DateTime = std::chrono::sys_time<std::chrono::microseconds>;

  struct DynColVal_T;
  using str_kv_pair = std::pair<std::string, DynColVal_T>;
  using num_kv_pair = std::pair<uint32_t,    DynColVal_T>;
  using str_kv_t   = std::unordered_map<std::string, DynColVal_T>;
  using num_kv_t     = std::unordered_map<uint32_t, DynColVal_T>;
  using dyncol_val_t = std::variant<
    longlong, ulonglong, double, std::string,
    Date, Time, DateTime, str_kv_t, num_kv_t
      >;
  struct DynColVal_T { dyncol_val_t v; };

  class DynamicColumn
  {
    public:
      DynamicColumn ();
      DynamicColumn (DYNAMIC_COLUMN);
      DynamicColumn (DynamicColumn &&);
      DynamicColumn (const DynamicColumn &);
      DynamicColumn& operator=(DynamicColumn &&);
      DynamicColumn& operator=(const DynamicColumn &);
      ~DynamicColumn ();

      bool check_format ();
      uint32_t column_count (DYNAMIC_COLUMN* = nullptr);

      bool exists (uint32_t key);
      bool exists (std::string key);

      bool uses_named_keys (DYNAMIC_COLUMN* = nullptr);
      std::string json ();

      std::vector<std::string> str_keys (DYNAMIC_COLUMN* = nullptr);
      std::vector<uint32_t> num_keys (DYNAMIC_COLUMN* = nullptr);

      str_kv_t unpack (DYNAMIC_COLUMN* = nullptr);

      DynColVal_T get (uint32_t key, DYNAMIC_COLUMN* = nullptr);
      DynColVal_T get (std::string key, DYNAMIC_COLUMN* = nullptr);

      std::optional<bool> compare_keys (std::string key1, std::string key2);

      void insert (str_kv_pair kv_pair, bool rst = true, DYNAMIC_COLUMN* = nullptr);
      void insert (num_kv_pair kv_pair, bool rst = true, DYNAMIC_COLUMN* = nullptr);
      void insert (str_kv_t kv, bool rst = true, DYNAMIC_COLUMN* = nullptr);
      void insert (num_kv_t kv, bool rst = true, DYNAMIC_COLUMN* = nullptr);

      void update (str_kv_pair kv_pair);
      void update (num_kv_pair kv_pair);
      void update (str_kv_t kv_pairs);
      void update (num_kv_t kv_pairs);

      void remove (std::string key);
      void remove (uint32_t    key);
      void remove (std::vector<std::string> keys);
      void remove (std::vector<uint32_t> keys);

      std::vector<std::byte> serialize ();

    private:
      DYNAMIC_COLUMN dyncol {};

    private:
      DynColVal_T to_dyncol_val_t (DYNAMIC_COLUMN_VALUE&);
      DYNAMIC_COLUMN_VALUE to_dyncol_val (DynColVal_T&);
  };
}
