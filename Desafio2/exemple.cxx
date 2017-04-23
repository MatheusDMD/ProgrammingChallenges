#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <fstream>
#include <map>

std::ifstream infile("in.txt");

int main() {
  std::string in;
  std::vector<char> params;
  std::vector<int> items;
  char char_item;
  int int_item;
  char val;
  int list_val[2] = {-1,-1};

  int num_of_params;
  infile >> num_of_params;
  //num_of_params++;

  for (int i = 0; i < num_of_params; i++) {
    infile >> char_item;
    params.push_back(char_item);
  }

  std::string opp;
  infile >> opp;

  int n;
  infile >> n;
  std::cout << n;
  std::cout << std::endl;

  std::map<char, int> map;
  while (n > 0)
  {
    for (int j = 0; j < num_of_params; j++) {
      infile >> int_item;
      map[params[j]] = int_item;
    }
    for(std::string::size_type i = 0; i < opp.size(); ++i) {
      val = opp[i];
      if(val == '&'){
        list_val[0] = -1;
        list_val[1] = -1;
        std::cout << "&" << ' ';
      }else if(val == '|'){
        list_val[0] = -1;
        list_val[1] = -1;
        std::cout << "|" << ' ';
      }else if(val == '^'){
        list_val[0] = -1;
        list_val[1] = -1;
        std::cout << "^" << ' ';
      }else{
        if(list_val[0] == -1){
          list_val[0] = map[opp[i]];
        }else if(list_val[1] == -1){
          list_val[1] = map[opp[i]];
        }else{
          std::cout << "WRONG SINTAX: Too many arguments for each operation";
          break;
        }
        std::cout << map[opp[i]] << ' ';
      }
    }

    std::cout << std::endl;
    n--;
    //trata os bichinhos
  }

  return 0;
}
