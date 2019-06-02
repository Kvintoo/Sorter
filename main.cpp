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

// Файл для функций и классов работы со временем.
#include <chrono>

using namespace std;




void SaveSorted(vector<uint32> &arrayForSort, int fileCounter)
{
  sort(arrayForSort.begin(), arrayForSort.end());
  string fileName = "output" + to_string(fileCounter);
  fstream ostream(fileName, ios::binary | ios::out);
  if (!ostream.is_open())
  {
    cout << "failed to open " << fileName << '\n';
  }

  ostream.write(reinterpret_cast<char*>(&arrayForSort[0]), arrayForSort.size()*sizeof(arrayForSort[0]));
  ostream.close();
}


bool SortParts(int& fileCounter)//сортировка частей файла
{
  CFileReader fileReader("input");
  if (!fileReader.IsInitialized())
  {
#ifdef WIN32
    system("PAUSE");
    return false;
#endif
  }

  uint32 testNumber = 0;
  //FIXME Определять размер массива в зависимости от размера файла?
  const int fileSizeArray = fileReader.FileSize() / NUMBER_SIZE;
  const int arraySize = fileSizeArray > ARRAY_SIZE ? ARRAY_SIZE : fileSizeArray;
  vector<uint32> arrayForSort(arraySize);//инициализируем массив 0
  int counter = 0;

  while (fileReader.ReadNumber(testNumber))//пока есть значение
  {
    arrayForSort[counter] = testNumber;
    testNumber = 0;
    ++counter;

    if (counter >= arraySize)
    {
      SaveSorted(arrayForSort, fileCounter);

      counter = 0;
      ++fileCounter;

      unsigned long remainder = fileReader.GetRemainder() / NUMBER_SIZE;
      if (remainder < static_cast<unsigned long>(arraySize))
      {
        arrayForSort.resize(remainder);
      }
    }
  }

  if (!arrayForSort.empty())
    SaveSorted(arrayForSort, fileCounter);

  return true;
}

void WriteData(vector<uint32> &outData, uint32 testNumber, fstream &ostream)
{
  if (outData.size() < ARRAY_SIZE)
    outData.push_back(testNumber);
  else
  {
    //ostream.write(reinterpret_cast<char*>(&arrayForSort[0]), arrayForSort.size()*sizeof(arrayForSort[0]));
    ostream.write(reinterpret_cast<char*>(outData.data()), outData.size()*sizeof(outData.data()));
    outData.clear();
  }
}


void MergeParts(int fileCounter)
{
  vector<CFileReader> fileReaders(fileCounter);
  int fileNumber = 1;
  for (auto& reader : fileReaders)
  {
    string fileName = "output" + to_string(fileNumber);
    ++fileNumber;
    reader.SetFile(fileName);
  }

  fstream ostream("output", ios::binary | ios::out);
  if (!ostream.is_open())
  {
    cout << "failed to open " << "output" << '\n';
    return;
  }
  vector<uint32> numbers;
  numbers.reserve(fileCounter);
  int readFiles = 0;
  uint32 ethalon = 0;
  vector<uint32> outData(ARRAY_SIZE);

  while (readFiles < fileCounter)
  {
    uint32 testNumber = 0;
    for (auto& reader : fileReaders)
    {
      while (reader.GetNumber() == ethalon)//если текущий элемент потока = записанному элементу потока
      {
        if (reader.ReadNumber(testNumber))
        {
          if (testNumber != ethalon)
          {
            numbers.push_back(testNumber);
            break;
          }
          else
          {
            WriteData(outData, testNumber, ostream);
          }
        }
        
        else//закончился файл для чтения
        {
          if (numbers.size() > 0)
            numbers.reserve(numbers.size());

          ++readFiles;
          break;
        }
      }
    }

    if (numbers.size() > 0)
    {
      sort(numbers.begin(), numbers.end());
      ethalon = numbers.front();

//       //лямбда не заработала, т.к. в цикле стоит break
//       numbers.erase(
//         remove_if(numbers.begin(), numbers.end(),
//                   [&](uint32& num)
//                   {
//                     if (num == ethalon)
//                     {
//                       WriteData(outData, num, ostream);
//                       //ostream.write(reinterpret_cast<char*>(&num), sizeof(num));
//                       return true;
//                     }
//                     return false;
//                   }),
//                   numbers.end());
      for (auto it = numbers.begin(); it != numbers.end();)//FIXME сделать лямбду
      {
        if ((*it) == ethalon)
        {
          ostream.write(reinterpret_cast<char*>(&(*it)), sizeof(*it));
          it = numbers.erase(it);
        }
        else
          break;
      }
    }
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
  // Получаем текущее время, используя высокоточный таймер.
  auto t1 = chrono::high_resolution_clock::now();

  int fileCounter = 1;
  if (!SortParts(fileCounter))
    return 0;

  // Снова получаем текущее время после выполнения операции.
  auto t2 = chrono::high_resolution_clock::now();

  //небольшой файл, весь поместился в ОП
  if (fileCounter == 2)//как переименовать файл программно?
  {
    chrono::duration<double> elapsed1 = t2 - t1;
    // Отображаем результаты.
    cout << "Sort temp files: " << elapsed1.count() << endl;
#ifdef WIN32
    system("PAUSE");
#endif
    return 0;
  }

  MergeParts(fileCounter);

  // Опять определяем текущее время после выполнения другой операции.
  auto t3 = chrono::high_resolution_clock::now();

  // Определяем время выполнения первой функции.
  chrono::duration<double> elapsed1 = t2 - t1;
  // Определяем время выполнения второй функции.
  chrono::duration<double> elapsed2 = t3 - t2;
  // Определяем суммарное время выполнения программы.
  chrono::duration<double> elapsed3 = t3 - t1;

  // Отображаем результаты.
  cout << "Sort temp files: " << elapsed1.count() << endl;
  cout << "Merge temp files: " << elapsed2.count() << endl;
  cout << "Total time: "      << elapsed3.count() << endl;

  //FIXME сделать, чтобы по завершении работы программы сгенерированные файлы удалялись (оставался только output)
#ifdef WIN32
  system("PAUSE");
#endif
}





