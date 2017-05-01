#include <iostream>
#include <string>
#include <list>

class node {
private:
  char val;

public:
  node(char *val, node next);
  ~node();

  std::char get_val();
};

int main(int argc, char const *argv[]) {
  std::string item;
  std::vector<char> letters = {'A','C','G','T'};

  return 0;
}
