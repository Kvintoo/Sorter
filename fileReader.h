#ifndef __FileReader__
#define __FileReader__


#include <fstream>
//#include "utils.h"


class CFileReader
{
public:
  CFileReader();
  CFileReader(int argc_, char** argv_);

  ~CFileReader();

  bool IsInitialized();//!< Проверяет, можно ли работать с классом CFileReader

  bool ReadNumber(uint64_t& number_);//!< Читает следующее значение из файла
  void ShowProgress();//!< Отображает прогресс чтения файла

private:

  bool OpenFile(int argc_, char** argv_);//!< Открывает файл, переданный как аргумент командной строки
  void GetFileSize(char* argv_);//!< Возвращает размер файла в байтах 
  

  int m_prevPercent;//!< Предыдущее значение прогресса чтения файла в %
  int m_fileSize;//!< Размер файла в байтах

  std::ifstream m_is;

};


#endif
