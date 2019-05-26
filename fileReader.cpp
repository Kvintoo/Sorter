#include <iostream>
#include <sys/stat.h>
#include "fileReader.h"

const int NUMBER_SIZE = 4;
CFileReader::CFileReader()
{

}

CFileReader::CFileReader(int argc_, char** argv_) :
m_prevPercent(0),
m_fileSize(0)
{
  if (OpenFile(argc_, argv_))
  {
    GetFileSize(argv_[1]);
  }
}

CFileReader::~CFileReader()
{
  m_is.close();
}

bool CFileReader::OpenFile(int argc_, char** argv_)
{
  if ((argc_ < 1) || (!argv_))
    return false;

  if ((argc_ < 2))
  {
    std::cout << "Enter file name to analyze.";
    return false;
  }
  
  m_is.open(argv_[1], std::ios::binary | std::ios::in);

  if (!m_is.is_open())
  {
    std::cerr << "Could not open file.\n";
    return false;
  }

  return true;
}

void CFileReader::GetFileSize(char* argv_)
{
  struct stat fi;
  stat(argv_, &fi);
  m_fileSize = static_cast<long>(fi.st_size);
}

bool CFileReader::IsInitialized()
{
  return m_fileSize > 0 ? true : false;//если длина файла не 0, то с файлом можно работать
}

void CFileReader::ShowProgress()
{
  int currentPercent = static_cast<int>(m_is.tellg() * 100 / m_fileSize);
  if (currentPercent - m_prevPercent > 5)//сообщаем о прогрессе чтения, если считали больше цифр, 
  {                                      //чем 5% от предыдущего выведенного значения прогресса
    std::cout << currentPercent << "% read\n";
    m_prevPercent = currentPercent;
  }
}

bool CFileReader::ReadNumber(uint64_t& number_)
{
  m_is.read((char*)&number_, NUMBER_SIZE);

  if (m_is.gcount() != NUMBER_SIZE)
    return false;

  return true;
}
