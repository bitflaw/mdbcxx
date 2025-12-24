#include "row.hpp"
#include "result_metadata.hpp"

namespace mcxx {
class Result {
public:

  Result () = default;
  Result (const Result&);
  Result (Result&&);
  Result (MYSQL_RES* res);

  Row& operator[] (std::size_t index);
  std::vector<Row>::iterator begin ();
  std::vector<Row>::const_iterator cbegin ();
  std::vector<Row>::reverse_iterator rbegin();
  std::vector<Row>::const_reverse_iterator crbegin();
  std::vector<Row>::iterator end ();
  std::vector<Row>::const_iterator cend ();
  std::vector<Row>::reverse_iterator rend ();
  std::vector<Row>::const_reverse_iterator crend ();

  std::size_t size() const;
  bool empty () const;
  void append(Row&&);

  ResultMetadata& get_metadata();

  ~Result() = default;
private:
  std::vector<Row> result_set {};
  ResultMetadata rmetadata {};
};

};
