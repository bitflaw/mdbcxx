#include "../include/mdbcxx/transaction.hpp"
#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>

Transaction::Transaction (Connection& c): cxn(c) {}
Transaction::Transaction (const Transaction& t): cxn(t.cxn) {}

std::optional<Result> Transaction::exec(std::string sql)
{
  if (mysql_real_query(cxn.raw(), sql.data(), sql.size()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'Transaction::exec(std::string)'] {}!", mysql_error(cxn.raw()))
    );
  }
  if (!mysql_field_count(cxn.raw())) return std::nullopt;
  return Result{mysql_store_result(cxn.raw())};
}

void malloc_rbstruct_buf (MYSQL_BIND& rbstruct, enum_field_types type)
{
  switch (type)
  {
    case MYSQL_TYPE_NULL:
      rbstruct.buffer = nullptr;
      *rbstruct.is_null = true;
      break;
    default:
      rbstruct.buffer = new char[rbstruct.buffer_length];
      std::memset(rbstruct.buffer, 0, rbstruct.buffer_length);
      break;
  }
}

void init_rbstructs(std::vector<MYSQL_BIND>& rbstructs, MYSQL_FIELD* fields)
{
  std::size_t n = rbstructs.size();
  for (std::size_t i = 0; i<n; i++)
  {
    rbstructs[i].buffer_length = std::max(fields[i].max_length + 1, std::size_t {64});
    rbstructs[i].buffer_type = MYSQL_TYPE_STRING;
    rbstructs[i].length = new ulong {0};
    rbstructs[i].is_null = new my_bool {0};
    rbstructs[i].error = new my_bool {0};

    malloc_rbstruct_buf(rbstructs[i], fields[i].type);
  }
}

void deinit_rbstructs(std::vector<MYSQL_BIND>& rbstructs)
{
  for (MYSQL_BIND& rbstruct : rbstructs)
  {
    delete rbstruct.length;
    delete rbstruct.is_null;
    delete rbstruct.error;
    if (rbstruct.buffer)
    {
      delete[] static_cast<char*>(rbstruct.buffer);
      rbstruct.buffer = nullptr;
    }
  }
}

std::optional<Result> Transaction::exec(prepped_stmt& pstmt, params p)
{
  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] Expected {} param(s), got {}",
                  pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> pbstructs {};
  pbstructs.reserve(p.size());
  for (param_T& param : p.raw()) pbstructs.emplace_back(set_param(param));

  if(mysql_stmt_bind_param(pstmt.stmt.get(), pbstructs.data()))
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec(prepped_stmt&, params)'] {}",
        mysql_stmt_error(pstmt.stmt.get())
      )
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }


  uint32_t nfields = mysql_stmt_field_count(pstmt.stmt.get());
  if(nfields == 0) return std::nullopt;

  std::vector<MYSQL_BIND> rbstructs {nfields};
  MYSQL_FIELD* fields = mariadb_stmt_fetch_fields(pstmt.stmt.get());
  if(fields == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec(prepped_stmt&, params)'] Expected a field array of len {}, "
        "instead got NULL!",
        nfields
      )
    );
  }
  init_rbstructs(rbstructs, fields);

  if (mysql_stmt_bind_result(pstmt.stmt.get(), rbstructs.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec(prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }

  Result res {};
  int retval = mysql_stmt_fetch(pstmt.stmt.get());
  while (retval != MYSQL_NO_DATA)
  {
    res.append(Row{fields, rbstructs});
    retval = mysql_stmt_fetch(pstmt.stmt.get());
    if(retval == 0) continue;
    else if (retval != MYSQL_NO_DATA && retval != MYSQL_DATA_TRUNCATED)
    {
      throw std::runtime_error(
        std::format("[ERROR: in 'exec(prepped_stmt&, params)'] {}",
                    mysql_stmt_error(pstmt.stmt.get())
                    )
      );
    }
  }
  deinit_rbstructs(rbstructs);
  return res;
}

void Transaction::exec0(std::string sql)
{
  if (mysql_real_query(cxn.raw(), sql.data(), sql.size()))
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec0(std::string)' with arg = {}] {}", sql, mysql_error(cxn.raw()))
    );
  }
  std::size_t nfields = mysql_field_count(cxn.raw());
  std::size_t nrows = mysql_num_rows(mysql_store_result(cxn.raw()));
  if (nfields>0)
  {
    throw std::length_error(
      std::format(
        "[ERROR: in 'exec0(std::string)'] Expected 0 rows, got {}", nrows)
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
  std::vector<MYSQL_BIND> pbstructs {};
  pbstructs.reserve(p.size());
  for (param_T& param : p.raw()) pbstructs.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), pbstructs.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec0(prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }
  if (mysql_stmt_execute(pstmt.stmt.get()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec0(prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }
  std::size_t nfields = mysql_stmt_field_count(pstmt.stmt.get());
  mysql_stmt_store_result(pstmt.stmt.get());
  std::size_t nrows = mysql_stmt_num_rows(pstmt.stmt.get());
  if (nfields>0)
  {
    throw std::length_error(
      std::format(
        "[ERROR: in 'exec0(std::string)'] Expected 0 rows, got {}", nrows)
    );
  }
}

Row Transaction::exec1(std::string sql)
{
  if (!cxn.is_connected())
  {
    throw std::runtime_error("[ERROR: in Transaction::exec1(std::string)] Connection handle is NULL!");
  }
  if (mysql_real_query(cxn.raw(), sql.c_str(), sql.size())) {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec1(std::string)' with arg = {}] {}", sql, mysql_error(cxn.raw()))
    );
  }

  if (!mysql_field_count(cxn.raw()))
  {
    throw std::length_error(
      std::format(
        "[ERROR: in 'exec1(std::string)' with arg = {}] Expected 1 row, got 0",
        sql)
    );
  }

  Result res {mysql_store_result(cxn.raw())};
  if (res.size() > 1)
  {
    throw std::length_error(
      std::format("[ERROR: in 'exec1(std::string)'] Expected 1 row, got {}", res.size())
    );
  }
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
  std::vector<MYSQL_BIND> pbstructs {};
  pbstructs.reserve(p.size());
  for (param_T& param : p.raw()) pbstructs.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), pbstructs.data()))
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

  uint32_t nfields = mysql_stmt_field_count(pstmt.stmt.get());
  if(nfields == 0)
  {
    throw std::length_error("[ERROR: in 'exec1(std::string)'] "
                            "Expected a result set, got none!");
  }

  std::vector<MYSQL_BIND> rbstructs {nfields};
  MYSQL_FIELD* fields = mariadb_stmt_fetch_fields(pstmt.stmt.get());
  if(fields == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'exec1(prepped_stmt&, params)'] Expected a field array of len {}, "
        "instead got NULL!",
        nfields
      )
    );
  }
  init_rbstructs(rbstructs, fields);

  if (mysql_stmt_bind_result(pstmt.stmt.get(), rbstructs.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}", mysql_stmt_error(pstmt.stmt.get()))
    );
  }

  if (mysql_stmt_fetch(pstmt.stmt.get()) == MYSQL_NO_DATA)
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'exec1(prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }

  Row row {fields, rbstructs};
  deinit_rbstructs(rbstructs);
  return row;
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
  if (res.size() > n)
  {
    throw std::length_error(
      std::format(
        "[ERROR: in 'exec(std::size_t, std::string)'] Expected {} rows, got {}",
        n, res.size()
      )
    );
  }
  return res;
}

// INFO: there is a concept of streaming, which i intend to add.
// this is brought about by using the mysql_fetch without first calling
// mysql_store_result or mysql_stmt_store_result which buffers the complete
// result set.

std::optional<Result> Transaction::execn(std::size_t n, prepped_stmt& pstmt, params p)
{
  if (p.size() != pstmt.param_count)
  {
    throw std::length_error(
      std::format("[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] Expected {} param(s), got {}",
                  pstmt.param_count, p.size()
                  )
    );
  }
  std::vector<MYSQL_BIND> pbstructs {};
  pbstructs.reserve(p.size());
  for (param_T& param : p.raw()) pbstructs.push_back(set_param(param));
  if(mysql_stmt_bind_param(pstmt.stmt.get(), pbstructs.data()))
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

  uint32_t nfields = mysql_stmt_field_count(pstmt.stmt.get());
  if(nfields == 0) return std::nullopt;

  std::vector<MYSQL_BIND> rbstructs {nfields};
  MYSQL_FIELD* fields = mariadb_stmt_fetch_fields(pstmt.stmt.get());
  if(fields == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] "
        "Expected a field array of length {}, got NULL!",
        nfields
      )
    );
  }
  init_rbstructs(rbstructs, fields);

  if (mysql_stmt_bind_result(pstmt.stmt.get(), rbstructs.data()))
  {
    throw std::runtime_error(
      std::format("[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] {}",
                  mysql_stmt_error(pstmt.stmt.get())
                  )
    );
  }

  Result res {};
  int retval = mysql_stmt_fetch(pstmt.stmt.get());
  for (std::size_t i = 1; i<=n; i++)
  {
    res.append(Row{fields, rbstructs});
    retval = mysql_stmt_fetch(pstmt.stmt.get());
    if(retval == 0) continue;
    else if (retval == MYSQL_NO_DATA) break;
    else if (retval != MYSQL_DATA_TRUNCATED)
    {
      throw std::runtime_error(
        std::format("[ERROR: in 'execn(std::size_t, prepped_stmt&, params)'] {}",
                    mysql_stmt_error(pstmt.stmt.get())
                    )
      );
    }
  }
  deinit_rbstructs(rbstructs);
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
