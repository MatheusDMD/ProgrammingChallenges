#include "node.hxx"

//Constructor/Destructor
node::node(std::string name) {
  visited = false;
  this->name = name;
}

node::~node() {
  for(std::vector<link*>::size_type i = 0; i != link_list.size(); i++) {
      delete link_list[i];
  }
}

//Distance
int node::get_distance(){
  return distance;
}

void node::set_distance(int distance){
  this->distance = distance;
}

//Name
std::string node::get_name(){
  return name;
}

//Visited
bool node::was_visited() {
  return visited;
}

void node::make_a_visit(){
  visited = true;
}

//link_list
void node::add_link(link *connection){
  link_list.push_back(connection);
}

std::vector<link *> node::get_links() {
  return link_list;
}
