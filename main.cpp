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
const size_t ARRAY_SIZE = 120L * 1024 * 1024 / 4; //!< Размер массива в 120 Мб 4 байтных чисел


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

struct InputFile
{
  InputFile()
  {
    //открыть файл на чтение
    inputFile.open("input", ios::binary);
    if (!inputFile.is_open())
      throw runtime_error("Can't open input file.");

    inputFile.seekg (0, inputFile.end);
    long length = static_cast<long>(inputFile.tellg());
    inputFile.seekg (0, inputFile.beg);

    const size_t maxThreads = static_cast<size_t>(4 * ARRAY_SIZE + length) / (4 * ARRAY_SIZE);
    const size_t hardwareThreads = thread::hardware_concurrency();
    numThreads = min(hardwareThreads != 0 ? hardwareThreads : 2, maxThreads);
  }

  bool ReadData(vector<uint32>& buffer, size_t& numElements, int& counter)
  {
    lock_guard<mutex> lk(inputFileMutex);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32));

    if (!inputFile.eof())
      numElements = buffer.size();
    else
      numElements = static_cast<size_t>(inputFile.gcount() / sizeof(uint32));

    if(numElements != 0)
    {
      counter = ++fileCounter;
      return true;
    }

    return false;
  }

  ifstream inputFile;
  int fileCounter = 0;
  size_t numThreads = 0;

  mutex inputFileMutex;
};

void SortPartFile(InputFile &fileData)
{
  vector<uint32> buffer(ARRAY_SIZE / fileData.numThreads);
  size_t numElements = 0;
  int counter = 0;

  while(fileData.ReadData(buffer, numElements, counter))
    SaveSorted(buffer, counter, numElements);
}

int SortParts()
{
  InputFile fileData;

  cout << "Number of threads: "<< fileData.numThreads << endl;
  vector<thread> threads(fileData.numThreads);

  for (auto& worker : threads)
    worker = thread{ SortPartFile, ref(fileData)};

  //дожидаемся завершения всех потоков
  for_each(threads.begin(), threads.end(), mem_fn(&thread::join)); 
  return fileData.fileCounter;
}

////////////////////////////////////////////////////////////////////////////////
struct MergedFileData
{
  void ReadData()
  {
    inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32));

    if (!inputFile.eof())
      maxPos = buffer.size();
    else
      maxPos = static_cast<size_t>(inputFile.gcount() / sizeof(uint32));

    if (maxPos)
      number = buffer[0];

    currentPos = 1;
  }

  void ReadData(int fileCount)
  {
    buffer.resize(ARRAY_SIZE / fileCount);
    ReadData();
  }

  bool GetNextNumber()
  {
    if (currentPos < maxPos)
    {
      number = buffer[currentPos];
      ++currentPos;
      return true;
    }
    else if (!inputFile.eof())
    {
      ReadData();
      return maxPos > 0;
    }

    return false;
  }
  ifstream inputFile;
  uint32 number = 0;
  vector<uint32> buffer;
  string fileName;
  int currentPos = 0;
  int maxPos = 0;
};

void MergeParts(int fileCounter)
{
  fstream output("output", ios::binary | ios::out);
  if (!output.is_open())
    throw runtime_error("Can't open output file.");

  list<MergedFileData> fileDatas(fileCounter);
  int fileNumber = 1;
  for (auto& fileData : fileDatas)
  {
    fileData.fileName = "output" + to_string(fileNumber);
    ++fileNumber;
    fileData.inputFile.open(fileData.fileName, ios::binary);
    if (!fileData.inputFile.is_open())
      throw runtime_error("Can't open file: " + fileData.fileName);

    fileData.ReadData(fileCounter);
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

    if (!itMin->GetNextNumber())
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
