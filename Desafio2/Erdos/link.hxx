#ifndef LINK_HXX
#define LINK_HXX

#include <vector>
#include <string>

class link {
private:
  int from;
  int to;

public:
  link(int from, int to);
  ~link();
  int get_from();
  int get_to();
};

#endif
