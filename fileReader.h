#ifndef __FileReader__
#define __FileReader__


#include <fstream>
#include "utils.h"


class CFileReader
{
public:
  CFileReader();
  CFileReader(std::string fileName);

  ~CFileReader();

  bool IsInitialized();//!< Проверяет, можно ли работать с классом CFileReader

  bool ReadNumber(uint32& number_);//!< Читает следующее значение из файла
  void ShowProgress();//!< Отображает прогресс чтения файла
  int  GetRemainder();//!< Возвращает размер от текущей позиции до конца файла

  const size_t FileSize() const;//!< Возвращает размер файла

private:

  bool OpenFile(std::string fileName);//!< Открывает файл, переданный как аргумент командной строки
  void GetFileSize(std::string fileName);//!< Возвращает размер файла в байтах 


  int m_prevPercent;//!< Предыдущее значение прогресса чтения файла в %
  int m_fileSize;//!< Размер файла в байтах

  std::ifstream m_is;

};


#endif