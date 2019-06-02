#include <iostream>
#include <sys/stat.h>
#include "fileReader.h"
#include "utils.h"


CFileReader::CFileReader(std::string fileName)
{
  SetFile(fileName);
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

bool CFileReader::IsInitialized() const
{
  if (m_fileSize % NUMBER_SIZE)
    return false;

  return m_fileSize > 2*NUMBER_SIZE ? true : false;//если длина файла больше двух цифр, то с файлом можно работать
}

unsigned long CFileReader::GetRemainder()
{
  return static_cast<unsigned long>(m_fileSize - m_is.tellg());
}

bool CFileReader::SetFile(std::string fileName)
{
  if (OpenFile(fileName))
  {
    SetFileSize(fileName);
    return true;
  }

  return false;
}

bool CFileReader::ReadNumber(uint32& number_)
{
  m_is.read((char*)&number_, NUMBER_SIZE);

  return !m_is.eof();
}

const int CFileReader::FileSize() const
{
  return static_cast<size_t>(m_fileSize);
}
