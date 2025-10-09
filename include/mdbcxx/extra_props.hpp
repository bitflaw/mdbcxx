#include <mysql/mysql.h>
#include <unordered_map>
#include <variant>

using extra_opt_type = std::variant<bool, u_char, uint, ulong, const char*>;

inline std::unordered_map<mysql_option, extra_opt_type> extra_properties
  {
    {MYSQL_SECURE_AUTH, false},
    {MYSQL_OPT_RECONNECT, true},
    {MYSQL_OPT_SSL_ENFORCE, false},
    {MYSQL_REPORT_DATA_TRUNCATION, true},
    {MARIADB_OPT_SKIP_READ_RESPONSE, false},
    {MYSQL_OPT_SSL_VERIFY_SERVER_CERT, false},
    {MYSQL_OPT_LOCAL_INFILE, (uint) 0},
    {MYSQL_OPT_READ_TIMEOUT, (uint) 30},
    {MYSQL_OPT_WRITE_TIMEOUT, (uint) 30},
    {MYSQL_OPT_WRITE_TIMEOUT, (uint) 30},
    {MYSQL_OPT_CONNECT_TIMEOUT, (uint) 30},
    {MYSQL_OPT_ZSTD_COMPRESSION_LEVEL, (u_char)'9'},
    {MYSQL_OPT_NET_BUFFER_LENGTH, (ulong) 16777216},
    {MYSQL_OPT_MAX_ALLOWED_PACKET, (ulong) 1073741824},
    {MYSQL_OPT_PROTOCOL, (uint) MYSQL_PROTOCOL_DEFAULT},
    {MYSQL_OPT_SSL_KEY, ""},
    {MYSQL_OPT_SSL_CERT, ""},
    {MYSQL_OPT_SSL_CA, ""},
    {MYSQL_OPT_SSL_CAPATH, ""},
    {MYSQL_OPT_SSL_CIPHER, ""},
    {MYSQL_SET_CHARSET_NAME, ""},
    {MYSQL_SET_CHARSET_DIR, ""},
    {MYSQL_DEFAULT_AUTH, ""},
    {MYSQL_OPT_BIND, ""},
    {MYSQL_PLUGIN_DIR, ""},
    {MYSQL_SHARED_MEMORY_BASE_NAME, ""},
    {MARIADB_OPT_SSL_FP, ""},
    {MARIADB_OPT_SSL_FP_LIST, ""},
    {MARIADB_OPT_TLS_PASSPHRASE, ""},
    {MARIADB_OPT_TLS_VERSION, ""},
    {MARIADB_OPT_RESTRICTED_AUTH, ""},
    {MARIADB_OPT_CONNECTION_HANDLER, ""},
    {MYSQL_SERVER_PUBLIC_KEY, ""},
    {MYSQL_OPT_CONNECT_ATTR_ADD, ""},
    {MYSQL_OPT_CONNECT_ATTR_DELETE, ""},
    {MYSQL_OPT_CONNECT_ATTR_RESET, ""},
    {MARIADB_OPT_RPL_REGISTER_REPLICA, ""},
    {MYSQL_OPT_CONNECT_ATTR_RESET, ""},
    {MARIADB_OPT_PROXY_HEADER, ""}
  };
