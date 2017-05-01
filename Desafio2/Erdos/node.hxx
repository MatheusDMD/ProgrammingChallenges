#ifndef NODE_HXX
#define NODE_HXX

#include "link.hxx"
#include <vector>
#include <string>

class node {
private:
  std::string name;
  std::vector<link *> link_list;
  int distance;
  bool visited;

public:
  node(std::string name);
  ~node();

  std::string get_name();

  int get_distance();
  void set_distance(int distance);

  void add_link(link *connection);
  std::vector<link*> get_links();

  bool was_visited();
  void make_a_visit();
};

#endif
