#ifndef __FileReader__
#define __FileReader__


#include <fstream>
#include "utils.h"


class CFileReader
{
public:
  CFileReader();
  CFileReader(std::string fileName);


  bool IsInitialized();//!< Проверяет, можно ли работать с классом CFileReader

  bool ReadNumber(uint32& number_);//!< Читает следующее значение из файла
  void ShowProgress();//!< Отображает прогресс чтения файла
  unsigned long GetRemainder();//!< Возвращает размер от текущей позиции до конца файла

  bool SetFile(std::string fileName);
  uint32 GetNumber() const;

  const int FileSize() const;//!< Возвращает размер файла

private:

  bool OpenFile(std::string fileName);//!< Открывает файл, переданный как аргумент командной строки
  void SetFileSize(std::string fileName);//!< Возвращает размер файла в байтах 


  int m_prevPercent;//!< Предыдущее значение прогресса чтения файла в %
  unsigned long m_fileSize;//!< Размер файла в байтах

  uint32 m_number;//!< Последнее считанное значение из файла

  std::ifstream m_is;

};


#endif
