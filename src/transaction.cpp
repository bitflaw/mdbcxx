#include "../include/mdbcxx/transaction.hpp"
#include <iostream>
#include <format>
#include <mysql/mysql.h>
#include <stdexcept>

Transaction::Transaction (Connection& c): cxn(c) {}
Transaction::Transaction (const Transaction& t): cxn(t.cxn) {}

Result Transaction::exec(std::string sql)
{
  if (mysql_real_query(cxn.raw(), sql.data(), sql.size()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec(std::string)' with arg = {}] {}!", sql, mysql_error(cxn.raw()))
    );
  }
  if (mysql_field_count(cxn.raw())) return Result{mysql_store_result(cxn.raw())};
  else return Result{};
}

Result Transaction::exec(prepped_stmt& pstmt, params p)
{

  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] Expected {} param(s), got {}",
                  pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> bind_struct {p.size()};
  for (param_T& param : p.raw()) bind_struct.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get())
      )
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  if (mysql_stmt_bind_result(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  Result res {};
  while (mysql_stmt_fetch(pstmt.stmt.get()) != MYSQL_NO_DATA)
  {
    res.append(Row{bind_struct});
  }
  return res;
}

void Transaction::exec0(std::string sql)
{
  if (mysql_real_query(cxn.raw(), sql.data(), sql.size())) {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec0(std::string)' with arg = {}] {}", sql, mysql_error(cxn.raw()))
    );
  }
}

void Transaction::exec0(prepped_stmt& pstmt, params p)
{
  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'exec0(prepped_stmt&, params)'] Expected {} params, got {}",
                   pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> bind_struct {p.size()};
  for (param_T& param : p.raw()) bind_struct.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec0(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec0(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
}

std::optional<Row> Transaction::exec1(std::string sql)
{
  if (!cxn.is_connected()) {
    std::cerr<<"connection down!"<<std::endl;
    return std::nullopt;
  }
  if (mysql_real_query(cxn.raw(), sql.c_str(), sql.size())) {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec1(std::string)' with arg = {}] {}", sql, mysql_error(cxn.raw()))
    );
  }

  if (!mysql_field_count(cxn.raw())) return std::nullopt;

  Result res {mysql_store_result(cxn.raw())};
  if (res.size() > 1)
    throw std::length_error(
      std::format("[ERROR: in 'exec1(std::string)'] Expected 1 row, got {}", res.size())
    );
  return res[0];
}

Row Transaction::exec1(prepped_stmt& pstmt, params p)
{
  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] Expected {} params, got {}",
                  pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> bind_struct {p.size()};
  for (param_T& param : p.raw()) bind_struct.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
  if (mysql_stmt_bind_result(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  if (mysql_stmt_fetch(pstmt.stmt.get()) == MYSQL_NO_DATA)
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
  return Row{bind_struct};
}

std::optional<Result> Transaction::execn(std::size_t n, std::string sql)
{
  if (mysql_real_query(cxn.raw(), sql.data(), sql.size())) {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'execn(std::size_t, std::string)' with args = ({},{})] {}",
        n, sql, mysql_error(cxn.raw())
      )
    );
  }
  if (!mysql_field_count(cxn.raw())) return std::nullopt;

  Result res {mysql_store_result(cxn.raw())};
  if (res.size() > n) throw std::length_error("bad length");
  return res;
}

// INFO: there is a concept of streaming, which i intend to add.
// this is brought about by using the mysql_fetch without first calling
// mysql_store_result or mysql_stmt_store_result which buffers the complete
// result set.
//
//
Result Transaction::execn(std::size_t n, prepped_stmt& pstmt, params p)
{
  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] Expected {} param(s), got {}",
                  pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> bind_struct {p.size()};
  for (param_T& param : p.raw()) bind_struct.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'execn(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'execn(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  if (mysql_stmt_bind_result(pstmt.stmt.get(), bind_struct.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'execn(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  Result res {};
  for (std::size_t i = 1; i<n; i++)
  {
//INFO: add support for the third return value type: indicates values have been truncated
//from the result set.
    res.append(Row{bind_struct});
    if (mysql_stmt_fetch(pstmt.stmt.get()))
    {
      throw std::length_error(
        std::format("[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
      );
    }
  }
  return res;
}

void Transaction::commit()
{
  if(mysql_commit(cxn.raw()))
  {
    throw std::runtime_error(std::format("[ERROR: in 'commit()'] {}", mysql_error(cxn.raw())));
  }
}

void Transaction::rollback()
{
  if(mysql_rollback(cxn.raw()))
  {
    throw std::runtime_error(std::format("[ERROR: in 'rollback()'] {}", mysql_error(cxn.raw())));
  }
}

void Transaction::toggle_autocommit(bool mode)
{
  if (mysql_autocommit(cxn.raw(), mode? 1:0))
  {
    throw std::runtime_error(std::format("[ERROR: in 'rollback()'] {}", mysql_error(cxn.raw())));
  }
}
