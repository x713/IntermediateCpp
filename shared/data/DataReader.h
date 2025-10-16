#pragma once

#include <fstream>
#include <string>
#include <iostream> 

void RD() {


  std::string filename = "Test2.b";

  std::ifstream istrm(filename, std::ios::binary);
  if (!istrm.is_open()) {
    std::cout << "failed to open " << filename << '\n';
    return;
  }


  double d;
  istrm.read(reinterpret_cast<char*>(&d), sizeof d); // binary input
  int n;
  std::string s;
  if (istrm >> n >> s)                               // text input
    std::cout << "read back from file: " << d << ' ' << n << ' ' << s << '\n';

}