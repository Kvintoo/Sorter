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
#include <chrono>
#include <xfunctional>




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


void SortParts(int& fileCounter)//сортировка частей файла
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
  std::vector<uint32> arrayForSort(ARRAY_SIZE);//инициализируем массив 0
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
}


void MergeParts(int fileCounter)
{
  std::vector<CFileReader> fileReaders(fileCounter);
  int fileNumber = 1;
  for (auto& reader : fileReaders)
  {
    std::string fileName = "output" + std::to_string(fileNumber);
    ++fileNumber;
    reader.SetFile(fileName);
  }

  std::fstream ostream("output", std::ios::binary | std::ios::out);
  if (!ostream.is_open())
  {
    std::cout << "failed to open " << "output" << '\n';
    return;
  }
  std::vector<uint32> numbers;
  numbers.reserve(fileCounter);
  int readFiles = 0;
  uint32 ethalon = std::numeric_limits<uint32>::max();

  while (readFiles < fileCounter)
  {
    uint32 testNumber = 0;
    for (auto& reader : fileReaders)
    {
      if (reader.GetNumber() != ethalon)
      {
        reader.ReadNumber(testNumber);
        numbers.push_back(testNumber);
      }

      else//если текущий элемент потока = записанному элементу потока
      {
        while (reader.GetNumber() != ethalon)
        {
          if(reader.ReadNumber(testNumber))
            ostream.write(reinterpret_cast<char*>(&ethalon), sizeof ethalon);
          else
          {
            numbers.reserve(numbers.size() - 1);
            numbers.shrink_to_fit();
            ++readFiles;
          }
        }
      }
    }

    std::sort(numbers.begin(), numbers.end(), std::greater<uint32>());

    ethalon = numbers.back();
    ostream.write(reinterpret_cast<char*>(&ethalon), sizeof ethalon);
    numbers.pop_back();

//     uint32 ethalon = numbers.back();
//     for (auto rit = numbers.rbegin(); rit != numbers.rend(); )
//     {
//       if ((*rit) == ethalon)
//       {
//         ostream.write(reinterpret_cast<char*>(&(*rit)), sizeof(*rit));
//         numbers.pop_back();
//       }
//       else
//         ++rit;
//     }

  }

  //NOTE можно считывать сразу несколько чисел в массивы и их анализировать, если это будет проще
  //сравниваем все числа из всех потоков. Наименьшее записываем в выходной массив.
  //как только выходной массив достигает максимального значения, записываем массив в выходной файл
  //делаем так пока все файлы не будут дочитаны до конца
  //если останется один недочитанный файл, а остальные уже будут дочитаны, 
  //то его содержимое от позиции до конца переписываем в выходной файл
  
  ostream.close();
}






int main()
{
//   auto now = std::chrono::system_clock::now();
  int fileCounter = 1;
  SortParts(fileCounter);

  //реализуем алгоритм сортировки слиянием
  //открываем на чтение количество потоков, равное количеству файлов

  //небольшой файл, весь поместился в ОП
  if (fileCounter == 1)//как переименовать файл программно?
    return 0;

  MergeParts(fileCounter);


  //FIXME сделать, чтобы по завершении работы программы сгенерированные файлы удалялись (оставался только output)
#ifdef WIN32
  std::system("PAUSE");
#endif
}



