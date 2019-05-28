#include <iostream>
#include <sys/stat.h>
#include "fileReader.h"
#include "utils.h"

CFileReader::CFileReader() :
m_prevPercent(0),
m_fileSize(0),
m_number(0)
{

}

CFileReader::CFileReader(std::string fileName) :
m_prevPercent(0),
m_fileSize(0),
m_number(0)
{
  SetFile(fileName);
//   if (OpenFile(fileName))
//   {
//     SetFileSize(fileName);
//   }
}

CFileReader::~CFileReader()
{
  m_is.close();
}

bool CFileReader::OpenFile(std::string fileName)
{
  m_is.open(fileName.c_str(), std::ios::binary | std::ios::in);

  if (!m_is.is_open())
  {
    std::cerr << "Could not open file.\n";
    return false;
  }

  return true;
}

void CFileReader::SetFileSize(std::string fileName)
{
  struct stat fi;
  stat(fileName.c_str(), &fi);
  m_fileSize = static_cast<long>(fi.st_size);
}

bool CFileReader::IsInitialized()
{
  return m_fileSize > 2*NUMBER_SIZE ? true : false;//если длина файла больше двух цифр, то с файлом можно работать
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

int CFileReader::GetRemainder()
{
  return static_cast<int>(m_fileSize - m_is.tellg());
}

uint32 CFileReader::GetNumber() const
{
  return m_number;
}

bool CFileReader::SetFile(std::string fileName)
{
  if (OpenFile(fileName))
  {
    SetFileSize(fileName);
  }

  return true;
}

bool CFileReader::ReadNumber(uint32& number_)
{
  m_is.read((char*)&number_, NUMBER_SIZE);

  if (m_is.gcount() != NUMBER_SIZE)
    return false;

  m_number = number_;
  return true;
}

const size_t CFileReader::FileSize() const
{
  return static_cast<size_t>(m_fileSize);
}
