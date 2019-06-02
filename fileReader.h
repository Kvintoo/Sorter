#ifndef __FileReader__
#define __FileReader__


#include <fstream>
#include "utils.h"


class CFileReader
{
public:
  CFileReader() = default;
  CFileReader(std::string fileName);


  bool IsInitialized() const;//!< Проверяет, можно ли работать с классом CFileReader

  bool ReadNumber(uint32& number_);//!< Читает следующее значение из файла
  unsigned long GetRemainder();//!< Возвращает размер от текущей позиции до конца файла

  bool SetFile(std::string fileName);

  const int FileSize() const;//!< Возвращает размер файла

private:

  bool OpenFile(std::string fileName);//!< Открывает файл, переданный как аргумент командной строки
  void SetFileSize(std::string fileName);//!< Возвращает размер файла в байтах 

  unsigned long m_fileSize = 0L;//!< Размер файла в байтах

  std::ifstream m_is;

};


#endif
