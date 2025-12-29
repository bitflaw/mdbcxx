#pragma once
#include "connection.hpp"
#include "result.hpp"
#include "params.hpp"
#include <cstddef>
#include <optional>

namespace mcxx {
class Transaction
{
  Connection& cxn;
public:

  Transaction (Connection&);
  Transaction (const Transaction&);
  Transaction (Transaction&&) = delete;

  Transaction& operator= (const Transaction&) = delete;
  Transaction& operator= (Transaction&&) = delete;

  std::optional<Result> exec(std::string);
  std::optional<Result> exec(prepped_stmt&, params);
  void exec0(std::string);
  void exec0(prepped_stmt&, params);
  Row exec1(std::string);
  Row exec1(prepped_stmt&, params);
  std::optional<Result> execn(std::size_t, std::string);
  std::optional<Result> execn(std::size_t, prepped_stmt&, params);

  void commit();
  void rollback();
  void toggle_autocommit(bool);

  ~Transaction() = default;
};

};//namespace mcxx
