#include "../include/mdbcxx/connection.hpp"
#include <cstring>
#include <mysql/mysql.h>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <format>

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
    conn_params.host.data(),
    conn_params.user.data(),
    conn_params.passwd.data(),
    conn_params.db_name.data(),
    conn_params.port,
    (conn_params.sock.data() != NULL ? conn_params.sock.data() : NULL),
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
    conn_params.host.data(),
    conn_params.user.data(),
    conn_params.passwd.data(),
    conn_params.db_name.data(),
    conn_params.port,
    (conn_params.sock.data() != NULL ? conn_params.sock.data() : NULL),
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

Connection::Connection (const Connection& conn):
  db_handle(conn.db_handle),
  conn_params(conn.conn_params)
{ mysql_close(conn.db_handle); }

Connection::Connection (Connection&& conn):
  db_handle(conn.db_handle),
  conn_params(std::move(conn.conn_params))
{ mysql_close(conn.db_handle); }

Connection::~Connection ()
{
  conn_params = {};
  mysql_close(db_handle);
}

Connection& Connection::operator= (Connection& conn)
{
  db_handle = conn.db_handle;
  conn_params = conn.conn_params;
  return *this;
}

Connection& Connection::operator= (Connection&& conn)
{
  if (this != &conn)
  {
    db_handle = conn.db_handle;
    conn_params = std::move(conn.conn_params);
  }
  return *this;
}

Connection& Connection::operator() (Connection& conn)
{
  db_handle = conn.db_handle;
  conn_params = conn.conn_params;
  return *this;
}

Connection& Connection::operator() (Connection&& conn)
{
  if (this != &conn)
  {
    db_handle = conn.db_handle;
    conn_params = std::move(conn.conn_params);
  }
  return *this;
}

bool Connection::default_db (std::string db_name)
{
  return (mysql_select_db(db_handle, db_name.data()) ? false : true);
}

bool Connection::reset ()
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in Connection::reset()'] Connection handle is NULL"<<std::endl;
    return false;
  }

  if (mysql_reset_connection(db_handle) != 0)
  {
    std::cerr<< "[ERROR: in 'Connection::reset()'] Failed to reset connection!"<<std::endl;
    return false;
  }else {
    std::cout<< "[INFO] Connection reset! "<<std::endl;
    return true;
  }
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
              << "Skipping property!";
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
                << "Skipping property!";
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
    conn_params.host.data(),
    conn_params.user.data(),
    conn_params.passwd.data(),
    conn_params.db_name.data(),
    conn_params.port,
    (conn_params.sock.data() != NULL ? conn_params.sock.data() : NULL),
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
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in Connection::is_connected()'] Connection handle is NULL"<<std::endl;
    return false;
  }
  return mariadb_connection(db_handle);
}

bool Connection::ping ()
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in Connection::ping()'] Connection handle is NULL"<<std::endl;
    return false;
  }
  if (mysql_ping(db_handle) != 0) return false;
  return true;
}

bool Connection::reconnect ()
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in Connection::reconnect()'] Connection handle is NULL"<<std::endl;
    return false;
  }
  return mariadb_reconnect(db_handle);
}

bool Connection::svr_reload (uint32_t reload_flags)
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in 'Connection::svr_reload()'] Connection handle is NULL"<<std::endl;
    return false;
  }
  int retval {-1};
  if ( reload_flags == 0 ) retval = mysql_refresh(db_handle, refresh_options);
  else retval = mysql_refresh(db_handle, reload_flags);
  return (retval == 0 ? true : false);
}

bool Connection::abort ()
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in 'Connection::abort()'] Connection handle is NULL"
      <<std::endl;
    return false;
  }
  if (mariadb_cancel(db_handle) != 0) return false;
  return true;
}

bool Connection::kill ()
{
  if (db_handle == NULL)
  {
    std::cerr<<"[ERROR: in 'Connection::kill()'] Connection handle is NULL"
      <<std::endl;
    return false;
  }
  if (mysql_kill(db_handle, mysql_thread_id(db_handle)) != 0) return false;
  return true;
}
