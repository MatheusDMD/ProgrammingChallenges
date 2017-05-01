#include "node.hxx"

//Constructor/Destructor
link::link(int from, int to){
  this->from = from;
  this->to = to;
}

link::~link(){
}

//FROM/TO
int link::get_from(){
  return from;
}
int link::get_to(){
  return to;
}
