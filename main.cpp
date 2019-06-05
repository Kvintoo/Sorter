#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <thread>
#include <mutex>

// Файл для функций и классов работы со временем.
#include <chrono>

using namespace std;

using uint32 = uint32_t; //!< Беззнаковое целое число длиной 4 байта
const size_t ARRAY_SIZE = 26214400; //!< Размер массива в 100 Мб 4 байтных чисел


void SaveSorted(vector<uint32> &arrayForSort, int fileCounter,
                size_t numElements)
{
  vector<uint32>::iterator itEnd = arrayForSort.begin() + numElements;

  sort(arrayForSort.begin(), itEnd);
  string fileName = "output" + to_string(fileCounter);
  fstream ostream(fileName, ios::binary | ios::out);
  if (!ostream.is_open())
  {
    cout << "failed to open " << fileName << '\n';
  }

  ostream.write(reinterpret_cast<char*>(arrayForSort.data()), numElements*sizeof(uint32));
}

mutex fileCounterMutex;

void SortPartFile(std::ifstream &input, int & fileCounter)
{
  vector<uint32> buffer(ARRAY_SIZE);
  int counter = 0;
  size_t numElements = 0;
  for(;;)
  {
    {
    lock_guard<mutex> lk(fileCounterMutex);
    input.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32));
    if (!input.eof())
      numElements = ARRAY_SIZE;

    else
      numElements = static_cast<size_t>(input.gcount() / sizeof(uint32));

    if(numElements == 0)
      break;

    counter = ++fileCounter;
    }

    SaveSorted(buffer, counter, numElements);
  }
}

/*
class SortedFileData
{
  ifstream inputFile;
  int counter = 1;
  string fileName;
};*/

int SortParts()
{
  //открыть файл на чтение
  ifstream input("input", ios::binary);
  if (!input.is_open())
    throw runtime_error("Can't open input file.");

  input.seekg (0, input.end);
  int length = input.tellg();
  input.seekg (0, input.beg);

  unsigned long const maxThreads = static_cast<long>(4 * ARRAY_SIZE + length) / (4 * ARRAY_SIZE);
  unsigned long const hardwareThreads = thread::hardware_concurrency();
  unsigned long const numThreads = min(hardwareThreads != 0 ? hardwareThreads : 2, maxThreads);

  cout << "Number of threads: "<< numThreads << endl;

  vector<thread> threads(numThreads);
  int fileCounter = 0;
  for (auto& worker : threads)
  {
    //каждому потоку передать файл, счетчик файлов
    worker = thread{ SortPartFile, ref(input), ref(fileCounter)};
  }

  //дожидаемся завершения всех потоков
  for_each(threads.begin(), threads.end(), mem_fn(&thread::join)); 
  return fileCounter;
}

#if 0
bool SortParts(int& fileCounter)//сортировка частей файла
{
  ifstream input("input", ios::binary);
  if (!input.is_open())
  {
#ifdef WIN32
    system("PAUSE");
#endif
    return false;//FIXME выбросить исключение
  }

  vector<uint32> buffer(ARRAY_SIZE);
  fileCounter = 0;

  for (;;)
  {
    int retflag;
    SortPartFile(input, buffer, fileCounter, retflag);
    if (retflag == 2) break;
  }
  return true;
}
#endif

struct MergedFileData
{
  ifstream inputFile;
  uint32 number = 0;
  string fileName;
};

void MergeParts(int fileCounter)
{
  fstream output("output", ios::binary | ios::out);
  if (!output.is_open())
  {
    cout << "failed to open " << "output" << '\n';
    return;
  }

  list<MergedFileData> fileDatas(fileCounter);
  int fileNumber = 1;
  for (auto& fileData : fileDatas)
  {
    fileData.fileName = "output" + to_string(fileNumber);
    ++fileNumber;
    fileData.inputFile.open(fileData.fileName, ios::binary);
    if (!fileData.inputFile.is_open())
      return;//FIXME переделать на исключение

    fileData.inputFile.read(reinterpret_cast<char*>(&fileData.number), sizeof(uint32));
  }

  vector<uint32> outData(ARRAY_SIZE);
  size_t outPos = 0;
  while (fileDatas.size() > 1)
  {
    auto itMin = min_element(fileDatas.begin(), fileDatas.end(),
                             [](const auto& l_, const auto& r_)
                             { return l_.number < r_.number; });

    if (outPos >= ARRAY_SIZE)
    {
      output.write(reinterpret_cast<char*>(outData.data()), outData.size()*sizeof(uint32));
      outPos = 0;
    }

    outData[outPos] = itMin->number;
    ++outPos;

    itMin->inputFile.read(reinterpret_cast<char*>(&itMin->number), sizeof(uint32));
    if (itMin->inputFile.eof())
    {
      const string fileName(itMin->fileName);
      fileDatas.erase(itMin);
      remove(fileName.c_str());
    }
  }

  output.write(reinterpret_cast<char*>(outData.data()), outPos*sizeof(uint32));
  auto& lastFile = fileDatas.front();
  output.write(reinterpret_cast<char*>(&lastFile.number), sizeof(uint32));

  output << lastFile.inputFile.rdbuf();

  const string fileName(lastFile.fileName);
  fileDatas.clear();
  remove(fileName.c_str());

  //NOTE можно считывать сразу несколько чисел в массивы и их анализировать, если это будет проще
  //сравниваем все числа из всех потоков. Наименьшее записываем в выходной массив.
  //как только выходной массив достигает максимального значения, записываем массив в выходной файл
  //делаем так пока все файлы не будут дочитаны до конца
  //если останется один недочитанный файл, а остальные уже будут дочитаны, 
  //то его содержимое от позиции до конца переписываем в выходной файл
}




int main()
{
  try
  {
  // Получаем текущее время, используя высокоточный таймер.
  auto t1 = chrono::high_resolution_clock::now();

  int fileCounter = SortParts();

//  if (!SortParts(fileCounter))
//    return 1;

  // Снова получаем текущее время после выполнения операции.
  auto t2 = chrono::high_resolution_clock::now();

  //небольшой файл, весь поместился в ОП
  if (fileCounter == 1)
    rename("output1", "output");
  else
    MergeParts(fileCounter);


  // Определяем текущее время после выполнения другой операции.
  auto t3 = chrono::high_resolution_clock::now();

  // Определяем время выполнения первой функции.
  chrono::duration<double> elapsed1 = t2 - t1;
  // Определяем время выполнения второй функции.
  chrono::duration<double> elapsed2 = t3 - t2;
  // Определяем суммарное время выполнения программы.
  chrono::duration<double> elapsed3 = t3 - t1;

  cout << "Files count: " << fileCounter << endl;

  // Отображаем результаты.
  cout << "Sort temp files: " << elapsed1.count() << endl;
  cout << "Merge temp files: " << elapsed2.count() << endl;
  cout << "Total time: " << elapsed3.count() << endl;

  }
  catch(const exception& e)
  {
    cout << e.what() << endl;
     return 1;
  }
  return 0;
}
