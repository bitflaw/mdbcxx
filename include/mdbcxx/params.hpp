#include <cstddef>
//#include <mysql/mariadb_com.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <vector>
#include <variant>
#include <cstdint>
#include <mysql/mysql.h>

enum sql_string_types
{
  CHAR = MYSQL_TYPE_VARCHAR,
  VARCHAR = MYSQL_TYPE_VAR_STRING,
  TEXT = MYSQL_TYPE_STRING
};

struct sqlstringT
{
  sql_string_types type {};
  std::variant<u_char, char, std::string> ssT {};
};

inline void handle_strTs (MYSQL_BIND* param, sqlstringT& value)
{
  std::visit([&](auto& strv){
    using strv_T = std::decay_t<decltype(strv)>;
    param->buffer_type = (enum_field_types) value.type;
    if constexpr (std::is_same_v<strv_T,std::string>)
    {
      param->buffer = strv.data();
      param->buffer_length = strv.size();
      return;
    }
    param->buffer = &strv;
  }, value.ssT);
}

using param_T = std::variant<
  uint8_t, int8_t, uint16_t, int16_t,
  uint32_t,int32_t, uint64_t, int64_t,
  float, double, std::vector<std::byte>,
  sqlstringT, std::nullptr_t, bool
>;


class params
{
  std::vector<param_T> param_vec {};
public:
  params () = default;
  template<typename... Args> params (Args&&...args);
  void append(std::nullptr_t);
  void append(bool);
  void append(uint8_t);
  void append(int8_t);
  void append(uint16_t);
  void append(int16_t);
  void append(uint32_t);
  void append(int32_t);
  void append(uint64_t);
  void append(int64_t);
  void append(float);
  void append(double);
  void append(std::vector<std::byte>&);
  void append(sqlstringT);
  void append(std::vector<param_T>&);
  template<typename... Args> void append_multi(Args... args);
  std::size_t size() const;
  std::vector<param_T>& raw();
};

inline MYSQL_BIND set_param(param_T& p){
  MYSQL_BIND* param = nullptr;
  std::visit([&](auto& arg){
    using arg_T = std::decay_t<decltype(arg)>;
    if constexpr(std::is_integral_v<arg_T>)
    {
      param->buffer = &arg;
      param->buffer_length = sizeof(arg);
      if constexpr (
        std::is_same_v<arg_T, uint8_t>||
        std::is_same_v<arg_T, int8_t>
      ) { param->buffer_type = MYSQL_TYPE_TINY;}
      else if constexpr (
        std::is_same_v<arg_T, uint16_t> ||
        std::is_same_v<arg_T, int16_t>
      ) { param->buffer_type = MYSQL_TYPE_SHORT;}
      else if constexpr (
        std::is_same_v<arg_T, uint32_t> ||
        std::is_same_v<arg_T, int32_t>
      ) { param->buffer_type = MYSQL_TYPE_LONG;}
      else if constexpr (
        std::is_same_v<arg_T, uint64_t> ||
        std::is_same_v<arg_T, int64_t>
      ) { param->buffer_type = MYSQL_TYPE_LONGLONG;}
      else if constexpr ( std::is_same_v<arg_T, bool>)
      { //param->buffer_type = MYSQL_TYPE_BOOL;
      }
//WARNING: this could be problematic for types not checked here like char, which are checked elswhere
      //
      //else { throw std::runtime_error("Type not found for integral type"); }
    } else if constexpr (std::is_floating_point_v<arg_T>)
    {
      param->buffer = &arg;
      param->buffer_length = sizeof(arg);
      if constexpr(std::is_same_v<arg_T, float>) {param->buffer_type=MYSQL_TYPE_FLOAT;}
      else if constexpr(std::is_same_v<arg_T, double>){param->buffer_type=MYSQL_TYPE_DOUBLE;}
      else {}
    }else if constexpr(std::is_same_v<arg_T, sqlstringT>)
    { handle_strTs(param, arg); }
    else if constexpr(std::is_same_v<arg_T, std::nullptr_t>)
    {
      param->buffer= nullptr;
      param->buffer_type = MYSQL_TYPE_NULL;
    }
    else if constexpr(std::is_same_v<arg_T, std::vector<std::byte>>)
    {
      uint64_t blob_size = arg.size();
      param->buffer = arg.data();
      param->buffer_length = blob_size;
      if (blob_size <= 2e8) {param->buffer_type = MYSQL_TYPE_TINY_BLOB;}
      else if (blob_size <= 2e16) {param->buffer_type = MYSQL_TYPE_BLOB;}
      else if (blob_size <= 2e24) {param->buffer_type = MYSQL_TYPE_MEDIUM_BLOB;}
      else if (blob_size <= 2e32) {param->buffer_type = MYSQL_TYPE_LONG_BLOB;}
      else { throw std::length_error("[ERROR: set_param()] Cannot set blob larger than 2e32 bits");}
    }
    else { throw std::runtime_error("Type not found"); }
  },p);
  return *param;
}
