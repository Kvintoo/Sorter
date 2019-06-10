#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <thread>
#include <mutex>

//#define MEASURE_TIME
// Файл для функций и классов работы со временем.
#ifdef MEASURE_TIME
#include <chrono>
#endif

using namespace std;

const size_t ARRAY_SIZE = 120L * 1024 * 1024 / 4; // Размер массива в 120 Мб 4 байтных чисел


void SaveSorted(vector<uint32_t> &arrayForSort, int fileCounter,
                size_t numElements)
{
  vector<uint32_t>::iterator itEnd = arrayForSort.begin() + numElements;

  sort(arrayForSort.begin(), itEnd);
  string fileName = "output" + to_string(fileCounter);
  fstream ostream(fileName, ios::binary | ios::out);
  if (!ostream.is_open())
    throw runtime_error("Can't open file: " + fileName);

  ostream.write(reinterpret_cast<char*>(arrayForSort.data()), numElements*sizeof(uint32_t));
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

  bool ReadData(vector<uint32_t>& buffer, size_t& numElements, int& counter)
  {
    lock_guard<mutex> lk(inputFileMutex);
    inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32_t));

    if (!inputFile.eof())
      numElements = buffer.size();
    else
      numElements = static_cast<size_t>(inputFile.gcount() / sizeof(uint32_t));

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
  //размер буфера определяется в зависимости от длины файла и
  //количества ядер на компьютере
  //на компьютере с большим количеством ядер и малым объемом памяти
  //алгоритм будет генерировать слишком много мелких файлов
  vector<uint32_t> buffer(ARRAY_SIZE / fileData.numThreads);
  size_t numElements = 0;
  int counter = 0;

  while(fileData.ReadData(buffer, numElements, counter))
    SaveSorted(buffer, counter, numElements);
}

int SortParts()
{
  InputFile fileData;
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
    inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(uint32_t));

    if (!inputFile.eof())
      maxPos = buffer.size();
    else
      maxPos = static_cast<size_t>(inputFile.gcount() / sizeof(uint32_t));

    if (maxPos)
      number = buffer[0];

    //равен 1, т.к. из буфера уже присвоено одно значение number
    //если maxPos = 0, то значение currentPos не важно
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

  void WriteTail(fstream& output)
  {
    output.write(reinterpret_cast<char*>(&number), sizeof(uint32_t));
    output.write(reinterpret_cast<char*>(buffer.data()), (maxPos - currentPos)*sizeof(uint32_t));

    output << inputFile.rdbuf();
  }

  void RemoveInputFile()
  {
    inputFile.close();
    remove(fileName.c_str());
  }

  ifstream inputFile;
  uint32_t number = 0;
  vector<uint32_t> buffer;
  string fileName;
  int currentPos = 0;
  int maxPos = 0;
};

void MergeParts(int fileCounter)
{
  //сравниваются все числа из всех потоков, наименьшее записывается в выходной массив
  //для ускорения алгоритмя используется буфер, в который зачитываются числа из файлов
  //у каждого файла свой буфер
  //как только выходной массив достигает максимального значения, записываем массив в выходной файл
  //делаем так пока все файлы не будут дочитаны до конца
  //если останется один недочитанный файл, а остальные уже будут дочитаны,
  //то его содержимое от текущей позиции до конца переписываем в выходной файл
  fstream output("output", ios::binary | ios::out);
  if (!output.is_open())
    throw runtime_error("Can't open output file.");

  //размер вектора менять нельзя, т.к. тогда указатели dataPointers станут невалидны
  vector<MergedFileData> fileDatas(fileCounter);
  vector<MergedFileData*> dataPointers(fileDatas.size());
  int fileNumber = 1;
  for (size_t i = 0; i < fileDatas.size(); ++i)
  {
    auto& fileData = fileDatas[i];
    fileData.fileName = "output" + to_string(fileNumber);
    ++fileNumber;
    fileData.inputFile.open(fileData.fileName, ios::binary);
    if (!fileData.inputFile.is_open())
      throw runtime_error("Can't open file: " + fileData.fileName);

    fileData.ReadData(fileCounter);
    dataPointers[i] = &fileData;
  }

  sort(dataPointers.begin(), dataPointers.end(),
       [](const auto& l, const auto& r)
       {return r->number < l->number;});//сортировка по убыванию

  vector<uint32_t> outData(ARRAY_SIZE);
  size_t outPos = 0;
  while (dataPointers.size() > 1)
  {
    MergedFileData* ptrToMinValue = dataPointers.back();

    if (outPos >= ARRAY_SIZE)
    {
      output.write(reinterpret_cast<char*>(outData.data()), outData.size()*sizeof(uint32_t));
      outPos = 0;
    }

    outData[outPos] = ptrToMinValue->number;
    ++outPos;

    dataPointers.pop_back();

    if (!ptrToMinValue->GetNextNumber())
      ptrToMinValue->RemoveInputFile();
    else
    {
      auto itNewPos = lower_bound(dataPointers.begin(), dataPointers.end(),
                                  ptrToMinValue->number,
                                  [](const auto& it, const auto& value)
                                  {return value < it->number;});
      dataPointers.insert(itNewPos, ptrToMinValue);
    }
  }

  output.write(reinterpret_cast<char*>(outData.data()), outPos*sizeof(uint32_t));
  auto& lastFile = *dataPointers.front();
  lastFile.WriteTail(output);
  lastFile.RemoveInputFile();
}




int main()
{
  try
  {
#ifdef MEASURE_TIME
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
#else
    int fileCounter = SortParts();

    //небольшой файл, весь поместился в ОП
    if (fileCounter == 1)
      rename("output1", "output");
    else
      MergeParts(fileCounter);
#endif

  }
  catch(const exception& e)
  {
    cout << e.what() << endl;
     return 1;
  }
  return 0;
}
