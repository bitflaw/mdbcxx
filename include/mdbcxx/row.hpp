#pragma once
#include "field.hpp"
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mcxx
{
class Row {
public:
  Row(const Row&);
  Row(Row&&);
  Row (MYSQL_FIELD*, std::size_t, MYSQL_ROW, unsigned long*);
  Row (MYSQL_FIELD*, std::vector<MYSQL_BIND>&);

  Row& operator= (const Row&);
  Row& operator= (Row&&);
  Field& operator[] (std::size_t);
  Field& operator[] (std::string column_name);

  std::vector<Field>::iterator begin ();
  std::vector<Field>::const_iterator cbegin ();
  std::vector<Field>::reverse_iterator rbegin();
  std::vector<Field>::const_reverse_iterator crbegin();
  std::vector<Field>::iterator end ();
  std::vector<Field>::const_iterator cend ();
  std::vector<Field>::reverse_iterator rend ();
  std::vector<Field>::const_reverse_iterator crend ();

  template <typename TUPLE_T> TUPLE_T as_tuple();
  std::size_t size() const;

private:
  std::vector<Field> fields {};
  std::vector<std::string> col_names {};

  template <typename TUPLE, std::size_t... I>
  TUPLE build_tuple (std::index_sequence<I...>);
};

template <typename TUPLE, std::size_t... I>
TUPLE Row::build_tuple (std::index_sequence<I...> seq)
{
  return TUPLE {
    ([](Field& field){
      using TargetT = std::tuple_element_t<I, TUPLE>;
      if(field.is_null())
      {
        if constexpr (std::__is_optional_v<TargetT>) return TargetT {std::nullopt};
        else return TargetT {};
      }
      if constexpr (std::__is_optional_v<TargetT>) return TargetT { field.as<typename TargetT::value_type>() };
      else return field.as<TargetT> ();
    }(fields[I]))...
  };
}

template <typename TUPLE_T>
TUPLE_T Row::as_tuple()
{
  constexpr std::size_t t_size = std::tuple_size_v<TUPLE_T>;
  if(t_size != fields.size()) throw std::length_error("Tuple size doesn't match row size");
  auto t_sequence = std::make_index_sequence<t_size> {};
  return build_tuple<TUPLE_T>(t_sequence);
}

};//namespace mcxx
