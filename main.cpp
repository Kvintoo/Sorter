#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include "utils.h"
#include "fileReader.h"
#include <algorithm>
#include <array>
#include <vector>
#include <memory>



void SaveSorted(std::array<uint32, ARRAY_SIZE> &arrayForSort, int fileCounter)
{
  std::sort(arrayForSort.begin(), arrayForSort.end());
  std::string fileName = "output" + std::to_string(fileCounter);
  std::fstream ostream(fileName, std::ios::binary | std::ios::out);
  if (!ostream.is_open())
  {
    std::cout << "failed to open " << fileName << '\n';
  }

  for (auto a : arrayForSort)
  {
    ostream.write(reinterpret_cast<char*>(&a), sizeof a);
  }
  ostream.close();
}

void SaveSorted(std::vector<uint32> &arrayForSort, int fileCounter)
{
  std::sort(arrayForSort.begin(), arrayForSort.end());
  std::string fileName = "output" + std::to_string(fileCounter);
  std::fstream ostream(fileName, std::ios::binary | std::ios::out);
  if (!ostream.is_open())
  {
    std::cout << "failed to open " << fileName << '\n';
  }

  for (auto a : arrayForSort)
  {
    ostream.write(reinterpret_cast<char*>(&a), sizeof a);
  }
  ostream.close();
}


void SortParts(int fileCounter)//сортировка частей файла
{
  CFileReader fileReader("input");
  if (!fileReader.IsInitialized())
  {
#ifdef WIN32
    std::system("PAUSE");
#endif
  }

  uint32 testNumber = 0;
  //FIXME Определять размер массива в зависимости от размера файла?
  const size_t arraySize = fileReader.FileSize() > ARRAY_SIZE ? ARRAY_SIZE : fileReader.FileSize();
  std::array<uint32, ARRAY_SIZE> arrayForSort = {};//инициализируем массив 0
  size_t counter = 0;

  while (fileReader.ReadNumber(testNumber))//пока есть значение
  {
    arrayForSort[counter] = testNumber;
    fileReader.ShowProgress();//показываем прогресс чтения файла
    testNumber = 0;
    ++counter;

    if (counter >= ARRAY_SIZE)
    {
      SaveSorted(arrayForSort, fileCounter);

      counter = 0;
      ++fileCounter;

      int remainder = fileReader.GetRemainder();
      if (fileReader.GetRemainder() < NUMBER_SIZE*ARRAY_SIZE)
      {
        //FIXME очистить массив, освободить память. Можно не делать. Память можно перезаписать.
        //arrayForSort.clear();
        break;
      }
    }
  }

  std::vector<uint32> remainder;
  remainder.reserve(fileReader.GetRemainder());
  while (fileReader.ReadNumber(testNumber))//пока есть значение
  {
    remainder.push_back(testNumber);
    fileReader.ShowProgress();//показываем прогресс чтения файла
    testNumber = 0;
    ++counter;
  }

  SaveSorted(remainder, fileCounter);
  //FIXME очистить вектор, освободить память
  //remainder.clear();//очистить вектор, освободить память
}


CFileReader/*&&*/ OpenFile()//NRVO не сработает скорее всего
{
  CFileReader fileReader("input");
  if (!fileReader.IsInitialized())
  {
#ifdef WIN32
    std::system("PAUSE");
#endif
  }
  return /*std::move(*/fileReader;
}





int main()
{
  int fileCounter = 1;
  SortParts(fileCounter);

  //реализуем алгоритм сортировки слиянием
  //открываем на чтение количество потоков, равное количеству файлов

  //небольшой файл, весь поместился в ОП
  if (fileCounter == 1)//как переименовать файл программно?
    return 0;

  //убрать дублирование кодов (статический класс?). Можно реализовать через фабрику классов-читателей файла
  std::vector<CFileReader> fileReaders;//unique_ptr?
  int fileNumber = 0;
  while (fileNumber != fileCounter)
  {
    fileReaders.push_back(OpenFile());
  }
  

  //NOTE можно считывать сразу несколько чисел в массивы и их анализировать, если это будет проще
  //сравниваем все числа из всех потоков. Наименьшее записываем в выходной массив.
  //как только выходной массив достигает максимального значения, записываем массив в выходной файл
  //делаем так пока все файлы не будут дочитаны до конца
  //если останется один недочитанный файл, а остальные уже будут дочитаны, 
  //то его содержимое от позиции до конца переписываем в выходной файл

  //FIXME сделать, чтобы по завершении работы программы сгенерированные файлы удалялись (оставался только output)
#ifdef WIN32
  std::system("PAUSE");
#endif
}



