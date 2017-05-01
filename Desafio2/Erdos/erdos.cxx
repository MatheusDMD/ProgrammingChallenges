#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <fstream>

#include "node.hxx"
#include "link.hxx"

int main(int argc, char const *argv[]) {
  std::ifstream file("in.txt");
  std::string line; // each line in the file
  std::stack<node *> not_explored;
  std::vector<node *> node_list;// list to keep track of nodes

  int from, to, n, m;
  //FILLING IN THE NODES AND LINKS

  std::getline(file, line);
  std::istringstream iss(line);
  iss >> n; // number of people
  m = n;
  while(m > 0){
    std::getline(file, line);
    node *nd = new node(line);
    node_list.push_back(nd);
    m--;
  }
  std::getline(file, line);
  std::istringstream iss2(line);
  iss2 >> m; // number of connections
  while(m > 0){
    std::getline(file, line);
    std::istringstream iss(line);
    iss >> from >> to;
    if(from > to){
      int temp = from;
      from = to;
      to = temp;
    }
    link *lk = new link(from, to);
    node_list[from]->add_link(lk);
    m--;
  }

  //ALGORITHM
  std::cout << n << '\n';
  node *nd;
  not_explored.push(node_list[0]);
  node_list[0]->make_a_visit();
  node_list[0]->set_distance(0);
  std::vector<link*> node_links;
  while (!not_explored.empty()) {
    nd = not_explored.top();
    not_explored.pop();
    node_links = nd->get_links();
    for(std::vector<link*>::size_type j = 0; j != node_links.size(); j++) {
      node *curr_node = node_list[node_links[j]->get_to()];
      if(!curr_node->was_visited()){
        curr_node->make_a_visit();
        curr_node->set_distance(nd->get_distance()+1);
        not_explored.push(curr_node);
      }
    }
  }
  for(std::vector<node*>::size_type i = 0; i != node_list.size(); i++) {
    std::vector<link*> links = node_list[i]->get_links();
    std::cout << node_list[i]->get_name() << ": " << node_list[i]->get_distance() << '\n';
  }
  return 0;
}

// for(std::vector<node*>::size_type i = 0; i != node_list.size(); i++) {
//   std::vector<link*> links = node_list[i]->get_links();
//   std::cout << node_list[i]->get_name() << ", " << i << '\n';
//   for(std::vector<link*>::size_type j = 0; j != links.size(); j++) {
//     std::cout << links[j]->get_from() <<','<<links[j]->get_to()<< '\n';
//   }
// }
