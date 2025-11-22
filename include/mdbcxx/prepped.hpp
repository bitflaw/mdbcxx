#include <memory>
#include <mysql/mysql.h>

inline auto stmt_dtor = [](MYSQL_STMT* stmt)
  {
    if (stmt)
    {
      try {mysql_stmt_close(stmt);} catch(...){}
    }
    delete stmt;
  };

struct prepped_stmt
{
  std::shared_ptr<MYSQL_STMT> stmt { nullptr, stmt_dtor};
  uint16_t param_count {0};
};
