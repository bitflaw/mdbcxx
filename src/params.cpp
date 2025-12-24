#include "../include/mdbcxx/params.hpp"
#include <stdexcept>

namespace mcxx
{
void params::append(std::nullptr_t val) { param_vec.emplace_back(val); }
void params::append(bool val) { param_vec.emplace_back(val); }
void params::append(uint8_t val) { param_vec.emplace_back(val); }
void params::append(int8_t val) { param_vec.emplace_back(val); }
void params::append(uint16_t val) { param_vec.emplace_back(val); }
void params::append(int16_t val) { param_vec.emplace_back(val); }
void params::append(uint32_t val) { param_vec.emplace_back(val); }
void params::append(int32_t val) { param_vec.emplace_back(val); }
void params::append(uint64_t val) { param_vec.emplace_back(val); }
void params::append(int64_t val) { param_vec.emplace_back(val); }
void params::append(float val) { param_vec.emplace_back(val); }
void params::append(double val) { param_vec.emplace_back(val); }
void params::append(std::vector<std::byte>& val) { param_vec.emplace_back(val); }
void params::append(sqlstringT val) { param_vec.emplace_back(val); }
void params::append(params& val)
{
  for (param_T v: val.raw()) param_vec.emplace_back(v);
}

void params::append(std::vector<param_T>& val)
{
  for (param_T & v: val) param_vec.emplace_back(v);
}

std::size_t params::size() const { return param_vec.size(); }

void params::reserve(std::size_t n) {param_vec.reserve(n);}
void params::clear() {param_vec.clear();}
std::vector<param_T>& params::raw() { return param_vec; }

MYSQL_BIND set_param(param_T& p)
{
  MYSQL_BIND param {};
  std::visit([&](auto& arg){
    using arg_T = std::decay_t<decltype(arg)>;
    if constexpr(std::is_integral_v<arg_T>)
    {
      param.buffer = &arg;
      param.buffer_length = sizeof(arg);
      if constexpr (
        std::is_same_v<arg_T, uint8_t>||
        std::is_same_v<arg_T, int8_t>
      ) { param.buffer_type = MYSQL_TYPE_TINY;}
      else if constexpr (
        std::is_same_v<arg_T, uint16_t> ||
        std::is_same_v<arg_T, int16_t>
      ) { param.buffer_type = MYSQL_TYPE_SHORT;}
      else if constexpr (
        std::is_same_v<arg_T, uint32_t> ||
        std::is_same_v<arg_T, int32_t>
      ) { param.buffer_type = MYSQL_TYPE_LONG;}
      else if constexpr (
        std::is_same_v<arg_T, uint64_t> ||
        std::is_same_v<arg_T, int64_t>
      ) { param.buffer_type = MYSQL_TYPE_LONGLONG;}
      else if constexpr ( std::is_same_v<arg_T, bool>)
      { //param.buffer_type = MYSQL_TYPE_BOOL;
      }
//WARNING: this could be problematic for types not checked here like char, which are checked elswhere
      //
      //else { throw std::runtime_error("Type not found for integral type"); }
    } else if constexpr (std::is_floating_point_v<arg_T>)
    {
      param.buffer = &arg;
      param.buffer_length = sizeof(arg);
      if constexpr(std::is_same_v<arg_T, float>) {param.buffer_type=MYSQL_TYPE_FLOAT;}
      else if constexpr(std::is_same_v<arg_T, double>){param.buffer_type=MYSQL_TYPE_DOUBLE;}
      else {}
    }else if constexpr(std::is_same_v<arg_T, sqlstringT>)
    { handle_strTs(&param, arg); }
    else if constexpr(std::is_same_v<arg_T, std::nullptr_t>)
    {
      param.buffer= nullptr;
      param.buffer_type = MYSQL_TYPE_NULL;
    }
    else if constexpr(std::is_same_v<arg_T, std::vector<std::byte>>)
    {
      uint64_t blob_size = arg.size();
      param.buffer = arg.data();
      param.buffer_length = blob_size;
      if (blob_size <= 2e8) {param.buffer_type = MYSQL_TYPE_TINY_BLOB;}
      else if (blob_size <= 2e16) {param.buffer_type = MYSQL_TYPE_BLOB;}
      else if (blob_size <= 2e24) {param.buffer_type = MYSQL_TYPE_MEDIUM_BLOB;}
      else if (blob_size <= 2e32) {param.buffer_type = MYSQL_TYPE_LONG_BLOB;}
      else { throw std::length_error("[ERROR: set_param()] Cannot set blob larger than 2e32 bits");}
    }
    else { throw std::runtime_error("Type not found"); }
  },p);
  return param;
}
};//namespace mcxx
