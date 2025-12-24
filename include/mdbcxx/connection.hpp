#include "extra_props.hpp"
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <cstdint>
#include "prepped.hpp"

namespace mcxx {
struct Properties{
  std::string host {"127.0.0.1"};
  std::string user {};
  std::string passwd {};
  std::string db_name {};
  std::string sock {};
  ulong flags {0};
  uint16_t port {3306};
};

class Connection
{
public:
  Connection () = default;
  Connection (Properties&);
  Connection (std::string user, std::string passwd, std::string db);
  Connection (const Connection&) = delete;
  Connection (Connection&&) = delete;

  Connection& operator= (const Connection&) = delete;
  Connection& operator= (Connection&&) = delete;

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
  void close();

  void prepare(std::string, std::string);
  prepped_stmt& prepped (std::string name);
  MYSQL* raw ();

  ~Connection ();

private:
  MYSQL* db_handle = mysql_init(NULL);
  Properties conn_params {};
  uint32_t refresh_options {
    REFRESH_GRANT | REFRESH_TABLES | REFRESH_THREADS |
    REFRESH_HOSTS | REFRESH_STATUS | REFRESH_LOG |
    REFRESH_SLAVE | REFRESH_MASTER
  };
  std::unordered_map<std::string, prepped_stmt> prepped_statements {};
};

};//namespace mcxx
