#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

const int NUMBER_SIZE = 4; //!< Размер чисел, которые мы будем читать, анализировать (4 байта)
bool readNumber(std::fstream& stream, uint32_t& number_)
{
  stream.read((char*)&number_, NUMBER_SIZE);

  if (stream.gcount() != NUMBER_SIZE)
    return false;

  return true;
}

int main()
{
   std::string input = "input";
   std::fstream stream(input, std::ios::binary | std::ios::in);
   if (!stream.is_open())
   {
      std::cout << "failed to open " << input <<  '\n';
      return 0/*1*/;
   }

   struct stat fi;
   stat(input.c_str(),&fi);
   long fileSize = static_cast<long>(fi.st_size);
   if(fileSize <= 0)
   {
      std::cout << "empty file " << input <<  '\n';
      return 0;
   }

   uint32_t testNumber = 0;
   while (readNumber(stream, testNumber))//пока есть значение
   {
      std::cout<< testNumber << std::endl;
   // read and analyze numbers
   }

   stream.close();

   std::string output = "output";
   std::fstream ostream(output, std::ios::binary | std::ios::out);
   if (!ostream.is_open())
   {
      std::cout << "failed to open " << output << '\n';
   }

   // write
   double d = 3.14;
   ostream.write(reinterpret_cast<char*>(&d), sizeof d); // binary output
   int n = 123;
   ostream.write(reinterpret_cast<char*>(&n), sizeof n); // binary output
   std::string str {"abc"};
   ostream.write(str.c_str(), str.size()); // binary output

   ostream.close();

#ifdef WIN32
   std::system("PAUSE");
#endif
}

