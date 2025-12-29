#pragma once
#include <cstddef>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <vector>
#include <variant>
#include <cstdint>
#include <mysql/mysql.h>
#include <concepts>

namespace mcxx {
enum sql_string_types
{
  CHAR = MYSQL_TYPE_VARCHAR,
  VARCHAR = MYSQL_TYPE_VAR_STRING,
  TEXT = MYSQL_TYPE_STRING
};

struct sqlstringT
{
  sql_string_types type {};
  std::variant<u_char, char, std::string> ssv {};
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
  }, value.ssv);
}

using param_T = std::variant<
  uint8_t, int8_t, uint16_t, int16_t,
  uint32_t,int32_t, uint64_t, int64_t,
  float, double, std::vector<std::byte>,
  sqlstringT, std::nullptr_t, bool
>;


template <typename Self, typename T>
concept NotSelf = !std::same_as<std::decay_t<T>, Self>;

class params
{
  std::vector<param_T> param_vec {};
public:
  params () = default;

  template<typename... Args>
  requires (sizeof...(Args) > 0 && (... && NotSelf<params, Args>))
  params (Args&&...args)
  {
    (param_vec.emplace_back(std::forward<Args>(args)), ...);
  }

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
  void append(params&);

  template<typename... Args>
  requires (sizeof...(Args) > 0)
  void append_multi(Args&&... args) { (append(args), ...); }

  std::vector<param_T>::iterator begin ();
  std::vector<param_T>::const_iterator cbegin ();
  std::vector<param_T>::reverse_iterator rbegin ();
  std::vector<param_T>::const_reverse_iterator crbegin();
  std::vector<param_T>::iterator end ();
  std::vector<param_T>::const_iterator cend ();
  std::vector<param_T>::reverse_iterator rend ();
  std::vector<param_T>::const_reverse_iterator crend ();

  std::size_t size() const;
  void reserve(std::size_t n);
  void clear();
  std::vector<param_T>& raw();
};

MYSQL_BIND set_param(param_T& p);

};
