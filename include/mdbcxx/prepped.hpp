#pragma once
#include <memory>
#include <mysql.h>

namespace mcxx
{

inline auto stmt_dtor = [](MYSQL_STMT* stmt) { if (stmt) mysql_stmt_close(stmt); };

struct prepped_stmt
{
  std::shared_ptr<MYSQL_STMT> stmt { nullptr, stmt_dtor};
  uint16_t param_count {0};
};

};
