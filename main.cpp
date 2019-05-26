#include <iostream>
#include <fstream>
#include <string>




int main() {
  std::string infile = "input.bin";
  std::fstream stream(infile, std::ios::binary | /*std::ios::trunc |*/ std::ios::in | std::ios::out);
  if (!stream.is_open()) 
  {
    std::cout << "failed to open " << infile <<  '\n';
  }
  else 
  {
     std::string outfile = "output.bin";
     std::fstream stream(outfile, std::ios::binary | /*std::ios::trunc | std::ios::in |*/ std::ios::out);
     if (!stream.is_open())
     {
        std::cout << "failed to open " << outfile << '\n';
     }
    // write
    double d = 3.14;
    stream.write(reinterpret_cast<char*>(&d), sizeof d); // binary output
    int n = 123;
    stream.write(reinterpret_cast<char*>(&n), sizeof n); // binary output
    std::string str {"abc"};
    stream.write(str.c_str(), str.size()); // binary output


  }
#ifdef WIN32
  std::system("PAUSE");
#endif
}

