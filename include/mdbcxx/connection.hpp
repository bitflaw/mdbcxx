#include "extra_props.hpp"
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <cstdint>

struct Properties{
  std::string host {"127.0.0.1"};
  std::string user {};
  std::string passwd {};
  std::string db_name {};
  std::string sock {NULL};
  ulong flags {0};
  uint16_t port {3306};
};


class Connection
{
public:
  Connection () = default;
  Connection (Properties&);
  Connection (std::string user, std::string passwd, std::string db);
  Connection (const Connection&);
  Connection (Connection&&);

  Connection& operator= (Connection&);
  Connection& operator= (Connection&&);
  Connection& operator() (Connection&);
  Connection& operator() (Connection&&);

  bool default_db (std::string db_name);
  bool reset ();
  void set_extra_props();
  void set_prop(std::pair<mysql_option, extra_opt_type>);
  void set_props(std::vector<std::pair<mysql_option, extra_opt_type>>);
  void connect (Properties);
  bool is_connected ();
  bool ping ();
  bool reconnect ();
  bool svr_reload (uint32_t = 0);
  bool abort ();
  bool kill ();

  ~Connection ();

private:
  MYSQL* db_handle = mysql_init(NULL);
  Properties conn_params {};
  uint32_t refresh_options {
    REFRESH_GRANT | REFRESH_TABLES | REFRESH_THREADS |
    REFRESH_HOSTS | REFRESH_STATUS | REFRESH_LOG |
    REFRESH_SLAVE | REFRESH_MASTER
  };
};
