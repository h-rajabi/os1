// SharedMemory.cpp
#include "SharedMemory.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <thread>

using namespace std;


bool readAndProcessFile(FileOpenResult* result) {
    ifstream file(result->fileName);
    
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << result->fileName << endl;
        return false;
    }
    
    string line;
    int64_t totalLines = 0;
    bool firstLine = true;
    
    while (getline(file, line)) {
        totalLines++;
        
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        
        if (start == string::npos || end == string::npos) {
            continue;
        }
        
        line = line.substr(start, end - start + 1);
        
        stringstream ss(line);
        int64_t number;
        
        if (ss >> number) {
            if (firstLine) {
                result->finalNumber = number;
                firstLine = false;
            } else {
                result->numbers.push_back(number);
            }
        } else {
            cout << "Warning: Line " << totalLines << " is not a valid number: " << line << endl;
        }
    }
    result->totalLine = totalLines;
    file.close();
    return true;
}

void displayResults(FileOpenResult* result) {
    cout << "=== File Processing Results ===" << endl;
    cout << "File name: " << result->fileName << endl;
    cout << "Final number (first line): " << result->finalNumber << endl;
    cout << "Total lines in file: " << result->totalLine << endl;
    cout << "Random numbers count: " << result->numbers.size() << endl;
    cout << "Random numbers: ";
    
    // for (size_t i = 0; i < result->numbers.size(); ++i) {
    //     cout << result->numbers[i];
    //     if (i < result->numbers.size() - 1) {
    //         cout << ", ";
    //     }
    // }
    cout << endl;
}

int GetProcessorCount(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

// پیاده‌سازی متدهای کلاس SharedMemory
SharedMemory::SharedMemory(const string memoryName, FileOpenResult* res, int proc, int size) 
    : Name(memoryName), Result(res), Process(proc), HMapFile(NULL), PData(NULL) {
    
    size_t memorySize;
    
    memorySize=getMemorySize(size);
    
    HMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        memorySize,
        Name.c_str()
    );
    
    if (HMapFile == NULL) {
        cerr << "CreateFileMapping failed: " << GetLastError() << endl;
        return;
    }
    
    PData = MapViewOfFile(
        HMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0
    );
    
    if (PData == NULL) {
        cerr << "MapViewOfFile failed in parrent: " << GetLastError() << endl;
        CloseHandle(HMapFile);
        HMapFile = NULL;
        exit(0);
    } else {
        cout << "Shared memory created successfully: " << Name << endl;
    }
}

SharedMemory::SharedMemory(const string memoryName, int proc,int size) :
    Name(memoryName), HMapFile(NULL), Process(proc) {
        Result= new FileOpenResult;
        
        HMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, Name.c_str());
        if (HMapFile == NULL) {
            cerr << "OpenFileMapping failed: " << Name << endl;
            exit(0);
        }
        PData = MapViewOfFile(
            HMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0
        );
        
        if (PData == NULL) {
            cerr << "MapViewOfFile failed: " << Name << endl;
            CloseHandle(HMapFile);
            HMapFile = NULL;
            exit(0);
        } else {
            cout << "Shared memory opened successfully: " << Name << endl;
        }
    }

SharedMemory::~SharedMemory() {
    if (PData) {
        UnmapViewOfFile(PData);
        cout << "Unmapped shared memory view" << endl;
    }
    if (HMapFile) {
        CloseHandle(HMapFile);
        cout << "Closed shared memory handle" << endl;
    }
}

size_t SharedMemory::getMemorySize(int type){
    switch (type)
    {
    case 0:
        return (sizeof(int64_t) * Result->totalLine + 2*sizeof(int64_t));
        break;
    case 1:
        return Process*sizeof(int);
        break;
    case 2:
        return (Process*sizeof(int64_t) * Result->totalLine);
        break;
    }
    return GetProcessorCount() * sizeof(int64_t) * (GetResult()->totalLine);
}
//parent
void SharedMemory::writeFileToRAM(int64_t time) {
    if (!PData) {
        cout << "Cannot write - shared memory not initialized" << endl;
        return;
    }
    int64_t* Buf = as<int64_t>();
    Buf[0] = time;
    Buf[1] = Result->totalLine;
    Buf[2] = Result->finalNumber;
    
    for (size_t i = 0; i < Result->numbers.size(); i++) {
        Buf[i+3] = Result->numbers[i];
        cout<<"number: "<<Result->numbers[i]<<endl;
    }

    
    cout << "Data written to shared memory successfully" << endl;

    cout << "Total elements written: " << (Result->numbers.size() + 3) << endl;
}
//child
int64_t SharedMemory::readFileFromRAM() {
    if (!PData) {
        cerr << "Cannot read - shared memory not initialized" << endl;
        return 0;
    }
    
    int64_t* Buf = as<int64_t>();
    int64_t time = Buf[0];
    Result->totalLine = Buf[1];
    Result->finalNumber = Buf[2];
    cout<<"time :"<<time<<endl;
    for (int i = 3; i <= ( Result->totalLine +2 ); i++) {
        Result->numbers.push_back(Buf[i]);
    }

    return time;
}
//parent
void SharedMemory::writeMapToRAM(const vector<int> map){

    if (!PData) {
        cout << "Cannot write - shared memory not initialized" << endl;
        return;
    }

    int* Buf = as<int>();

    for (size_t i = 0; i < Process; i++)
    {
        Buf[i] = map[i];
    }
    cout<<"write to mapMemory sucess \n";
}
//child
int SharedMemory::readMapFromRAM(int pid){

    if (!PData) {
        cerr << "Cannot read - shared memory not initialized" << endl;
        return -1;
    }

    int* Buf = as<int>();

    for (size_t i = 0; i < Process; i++)
    {
        if (pid == Buf[i])
        {
            return i;
        }
    }
    return -1;
}
//child
void SharedMemory::writeResultToRAM(int index){
    if (!PData) {
        cout << "Cannot write - shared memory not initialized" << endl;
        return;
    }
    
    int64_t* Buf = as<int64_t>();

    int j = index * Result->totalLine;

    Buf[j] = Result->finalNumber;
    int h =0;
    for (size_t i = j + 1; i < (j + Result->totalLine) -1; i++)
    {
        if (h >= Result->numbers.size())
        {
            Buf[i] = 0;
        }else
        {
            Buf[i]= Result->numbers[h];
            h++;    
        }
    }
}
//parent
void SharedMemory::readResultFromRAM(){
    if (!PData) {
        cerr << "Cannot read - shared memory not initialized" << endl;
        return;
    }

    int64_t* Buf = as<int64_t>();

    int64_t min = Buf[0];
    int jm = 0;
    int j=0;
    for (size_t i = Result->totalLine ; i < Process*Result->totalLine ; i += Result->totalLine)
    {
        // cout<<"index :"<<i<<endl;
        j++;
        if (Buf[i] < min)
        {
            min = Buf[i];
            jm=j;
        }
    }
    // cout<<"find best in index :"<<jm<<endl;

    vector<int64_t> final;
    int64_t fnumber;
    int64_t index = jm * Result->totalLine;
    fnumber=Buf[index];
    for (size_t i = index+1 ; i < ( index + Result->totalLine); i++)
    {
        if (Buf[i] != 0)
        {
            final.push_back(Buf[i]);
        }
        else break;
    }
    displayFinalResult(final, fnumber);
}

void SharedMemory::readAllResultFromRAM(){
    if (!PData) {
        cerr << "Cannot read - shared memory not initialized" << endl;
        return;
    }

    int64_t* Buf = as<int64_t>();
    int x = Process*Result->totalLine;
    for (size_t i = 0; i < x ; i++)
    {
        cout<<"index["<<i<<"] is :"<<Buf[i]<<endl;
    }
    
}

void SharedMemory::displaySharedData() {
    cout << "=== Shared Memory Info ===" << endl;
    cout << "Name: " << Name << endl;
    cout << "Process count: " << Process << endl;
    cout << "Is valid: " << (is_valid() ? "Yes" : "No") << endl;
}

void SharedMemory::displayFinalResult(vector<int64_t> numbers, int64_t finalNumber){
    cout<<"end of program \n";
    cout<<"the result is :"<<finalNumber<<endl;
    cout<<"The difference between your number ("<<Result->finalNumber<<") and the result ("<<finalNumber<<") is :" << finalNumber - Result->finalNumber <<"\n";
    cout<<"program chose this cables :\n";
    
    for (size_t i = 0; i < numbers.size(); i++)
    {
        cout<<numbers[i]<<",";
    }
    cout<<"also we chosse "<<numbers.size()<<" cables.\n";
}

void SharedMemory::setResult(FileOpenResult* result, int index){
    Result = result;
    writeResultToRAM(index);
}

