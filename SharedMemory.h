// SharedMemory.h
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <windows.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct FileOpenResult {
    string fileName;
    int64_t finalNumber;
    vector<int64_t> numbers;
    int64_t totalLine;
};

class SharedMemory {
private:
    HANDLE HMapFile;
    string Name;
    FileOpenResult* Result;
    int Process;
    void* PData;

public:
    SharedMemory(const string memoryName, FileOpenResult* res, int proc = 1, int size=0);
    SharedMemory(const string memoryName, int proc=1);
    ~SharedMemory();
    
    template<typename T>
    T* as() const { 
        if (!PData) {
            cout << "Error: Cannot cast - shared memory not mapped" << endl;
            return nullptr;
        }
        return reinterpret_cast<T*>(PData); 
    }

    HANDLE getHandle() { return HMapFile; }
    FileOpenResult* GetResult() {return Result;}
    int GetProcessorCount(){return Process;} 
    bool is_valid() const { return PData != NULL; }
    void writeFileToRAM(int64_t time);
    int64_t readFileFromRAM();
    void writeMapToRAM(const vector<int> map);
    int readMapFromRAM(int pid);
    void writeResultToRAM(int index);
    void readResultFromRAM();
    void displayFinalResult(vector<int64_t> numbers, int finalNumber);
    size_t getMemorySize(int type);
    void displaySharedData();
    void setResult(FileOpenResult *result, int index);
};

bool readAndProcessFile(FileOpenResult& result);
void displayResults(FileOpenResult& result);
int GetProcessorCount();

#endif // SHARED_MEMORY_H