#include "../include/mdbcxx/connection.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <format>

namespace mcxx 
{

Connection::Connection (Connection&& cxn)
{
  db_handle = cxn.db_handle;
  cxn.db_handle = NULL;
  refresh_options = std::move(cxn.refresh_options);
  prepped_statements = std::move(cxn.prepped_statements);
  conn_params = std::move(cxn.conn_params);
}

Connection& Connection::operator= (Connection&& cxn)
{
  db_handle = cxn.db_handle;
  cxn.db_handle = NULL;
  refresh_options = std::move(cxn.refresh_options);
  prepped_statements = std::move(cxn.prepped_statements);
  conn_params = std::move(cxn.conn_params);
  return *this;
}

Connection::Connection (Properties& cparams):
  conn_params(cparams)
{
  if (db_handle == NULL)
  {
    throw std::runtime_error(
      "[ERROR: in 'Connection::Connection(Properties&)] Database connection failed!"
    );
  }
  conn_params.flags |= CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS |
                       CLIENT_REMEMBER_OPTIONS;
  set_extra_props();
  MYSQL* ret_handle = mysql_real_connect(
    db_handle,
    conn_params.host.c_str(),
    conn_params.user.c_str(),
    conn_params.passwd.c_str(),
    conn_params.db_name.c_str(),
    conn_params.port,
    (conn_params.sock.empty() ? NULL : conn_params.sock.c_str()),
    conn_params.flags
  );

  if (ret_handle == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in Connection::Connection(Properties&)] {}",
        mysql_error(db_handle)
      )
    );
  }
}

Connection::Connection (std::string user, std::string passwd, std::string db)
{
  if (db_handle == NULL)
  {
    throw std::runtime_error(
      "[ERROR: in 'Connection::Connection(std::string user, std::string passwd, std::string db)]"
      " Database connection failed!"
    );
  }
  conn_params.user = user;
  conn_params.passwd = passwd;
  conn_params.db_name = db;
  conn_params.flags = CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS |
                      CLIENT_REMEMBER_OPTIONS;

  set_extra_props();
  MYSQL* ret_handle = mysql_real_connect(
    db_handle,
    conn_params.host.c_str(),
    conn_params.user.c_str(),
    conn_params.passwd.c_str(),
    conn_params.db_name.c_str(),
    conn_params.port,
    (conn_params.sock.empty()? NULL: conn_params.sock.c_str()),
    conn_params.flags
  );
  if (ret_handle == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in Connection::Connection(std::string user, std::string passwd, std::string db)] {}",
        mysql_error(db_handle)
      )
    );
  }
}

Connection::~Connection ()
{
  conn_params = {};
  if (db_handle)
  {
    mysql_close(db_handle);
    db_handle = nullptr;
  }
}

void Connection::close()
{
  conn_params = {};
  if (db_handle)
  {
    mysql_close(db_handle);
    db_handle = nullptr;
  }
}

bool Connection::reset ()
{
  if (db_handle == NULL) return false;

  if (mysql_reset_connection(db_handle)) return false;
  return true;
}

void Connection::set_extra_props()
{
  for (auto [opt, opt_value] : extra_properties)
  {
    std::visit([&](auto& arg) {
      using arg_T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<arg_T, const char*>)
      {
        if (arg != nullptr && strlen(arg) > 0) mysql_optionsv(db_handle, opt, arg, strlen(arg));
      } else if constexpr (std::is_same_v<arg_T, bool>)
      {
        my_bool local = arg ? 1 : 0;
        mysql_optionsv(db_handle, opt, &local);
      } else
      {
        mysql_optionsv(db_handle, opt, &arg);
      }

//TODO: check for more special types like the callbacks which we haven't added yet.

    }, opt_value);
  }
}

void Connection::set_prop(std::pair<mysql_option, extra_opt_type> prop)
{
  auto it = extra_properties.find(prop.first);
  if (it == extra_properties.end())
  {
    std::cerr << "[ERROR: in 'Connection::set_prop()'] Trying to change undefined property! "
      << "Skipping property!"
      << std::endl;
  }
  it->second = prop.second;
}

void Connection::set_props(std::vector<std::pair<mysql_option, extra_opt_type>> props)
{
  for (auto& prop : props)
  {
    auto it = extra_properties.find(prop.first);
    if (it == extra_properties.end())
    {
      std::cerr << "[ERROR: in 'Connection::set_props()'] Trying to change undefined property! "
        << "Skipping property!"
        <<std::endl;
    }
    it->second = prop.second;
  }
}

void Connection::connect (Properties conn_props)
{
  conn_params = std::move(conn_props);
  conn_params.flags = CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS;

  set_extra_props();
  MYSQL* ret_handle = mysql_real_connect(
    db_handle,
    conn_params.host.c_str(),
    conn_params.user.c_str(),
    conn_params.passwd.c_str(),
    conn_params.db_name.c_str(),
    conn_params.port,
    (conn_params.sock.empty() ? NULL : conn_params.sock.c_str()),
    conn_params.flags
  );
  if (ret_handle == NULL)
  {
    throw std::runtime_error(
      std::format("[ERROR: in Connection::connect()] {}",mysql_error(db_handle))
    );
  }
}

bool Connection::is_connected ()
{
  if (db_handle == NULL) return false;
  return mariadb_connection(db_handle);
}

bool Connection::ping ()
{
  if (db_handle == NULL) return false;
  if (mysql_ping(db_handle))
  {
    std::cerr<<"[ERROR: in Connection::ping()'] "<<mysql_error(db_handle)<<std::endl;
    return false;
  }
  return true;
}

bool Connection::reconnect ()
{
  if (db_handle == NULL) return false;
  return mariadb_reconnect(db_handle);
}

bool Connection::svr_reload (uint32_t reload_flags)
{
  if (db_handle == NULL) return false;
  int retval {-1};
  if ( reload_flags == 0 ) retval = mysql_refresh(db_handle, refresh_options);
  else retval = mysql_refresh(db_handle, reload_flags);
  return (retval == 0 ? true : false);
}

bool Connection::abort ()
{
  if (db_handle == NULL) return false;
  if (mariadb_cancel(db_handle))
  {
    std::cerr<<"[ERROR: in 'Connection::abort()'] "<<mysql_error(db_handle)<<std::endl;
    return false;
  }
  return true;
}

void Connection::prepare(std::string stmt_name, std::string prepped_sql)
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in 'Connection::prepare()'] Connection handle is NULL"
      <<std::endl;
    return;
  }
  prepped_stmt prepped {
    .stmt = std::shared_ptr<MYSQL_STMT>{mysql_stmt_init(db_handle)}
  };
  if (prepped.stmt == NULL)
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in Connection::prepare()] {}!",
        mysql_stmt_error(prepped.stmt.get())
      )
    );
  }

  if (mysql_stmt_prepare(prepped.stmt.get(), prepped_sql.c_str(), prepped_sql.size()))
  {
    throw std::runtime_error(
      std::format(
        "[ERROR: in Connection::prepare()] {}!",
        mysql_stmt_error(prepped.stmt.get())
      )
    );
  }
  prepped.param_count = mysql_stmt_param_count(prepped.stmt.get());
  prepped_statements.insert({stmt_name,prepped});
}

prepped_stmt& Connection::prepped(std::string name)
{
  if (!prepped_statements.contains(name))
  {
    throw std::invalid_argument("The name provided for the prepared statement doesn't exist!");
  }
  return prepped_statements.at(name);
}

MYSQL* Connection::raw () { return db_handle; }

};
