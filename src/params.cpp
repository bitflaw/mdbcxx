#include "../include/mdbcxx/params.hpp"
#include <utility>

template<typename... Args>
params::params (Args&&...args)
{
  param_vec.push_back(std::forward(args)...);
}

void params::append(std::nullptr_t val) { param_vec.push_back(val); }
void params::append(bool val) { param_vec.push_back(val); }
void params::append(uint8_t val) { param_vec.push_back(val); }
void params::append(int8_t val) { param_vec.push_back(val); }
void params::append(uint16_t val) { param_vec.push_back(val); }
void params::append(int16_t val) { param_vec.push_back(val); }
void params::append(uint32_t val) { param_vec.push_back(val); }
void params::append(int32_t val) { param_vec.push_back(val); }
void params::append(uint64_t val) { param_vec.push_back(val); }
void params::append(int64_t val) { param_vec.push_back(val); }
void params::append(float val) { param_vec.push_back(val); }
void params::append(double val) { param_vec.push_back(val); }
void params::append(std::vector<std::byte>& val) { param_vec.push_back(val); }
void params::append(sqlstringT val) { param_vec.push_back(val); }

template<typename... Args>
void params::append_multi(Args... args)
{
  (append(args), ...);
}

void params::append(std::vector<param_T>& val)
{
  for (param_T & v: val) param_vec.push_back(v);
}

std::size_t params::size() const { return param_vec.size(); }

std::vector<param_T>& params::raw() { return param_vec; }
