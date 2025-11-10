#include <windows.h>
#include "SharedMemory.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <time.h>
#include <chrono>

using namespace std;

// struct fileOpenResult {
//     string fileName;
//     int64_t finalNumber;
//     vector<int64_t> numbers;
//     int totalLine;
// };

int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

bool ran(){
    random_device rd;
    mt19937 gen(rd());
    bernoulli_distribution dis(0.5);
    
    bool random_bool = dis(gen);
    return random_bool;
}

void findBestCables(FileOpenResult* Cables, FileOpenResult* Result, int64_t timee){
    auto start = chrono::steady_clock::now();
    vector<int64_t> TempResult;
    int64_t min = -1;
    
    vector<int64_t> TempSum;
    int64_t sum = 0 ;
    
    bool r;
    while (true) {
		auto now = chrono::steady_clock::now();
		auto spent = chrono::duration_cast<chrono::seconds>(now - start);
		if (spent.count() >= timee) {
			break;
		}
        sum=0;
        for (size_t i = 0; i < Cables->totalLine; i++)
        {
            if (sum >= Cables->finalNumber )
            {
                if ( min > sum )
                {
                    min=sum;
                    TempResult = TempSum;
                    break;
                }
                break;
            }
            r=ran();
            if (r)
            {
                TempSum.push_back(Cables->numbers[i]);
                sum += Cables->numbers[i];
            }
        }
    }

    Result->finalNumber=min;
    Result->numbers=TempResult;
    Result->totalLine=TempResult.size();

}

int main() {
    
    int Cores =CoresNumber();
    int pId = GetCurrentProcessId();
    int64_t timee;
    SharedMemory* FileMemory = new SharedMemory("FileMemory",Cores);

    timee = FileMemory->readFileFromRAM();

    FileOpenResult* Cables = FileMemory->GetResult();
    FileOpenResult* Result;

    findBestCables(Cables,Result,timee);

    SharedMemory* MapMemory = new SharedMemory("MapMemory",Cores);

    int index = MapMemory->readMapFromRAM(pId);
    if (index == -1 )
    {
        cout<<"Error can`t find pID : "<<pId<<" in mapMemory\n";
        return 0;
    }
    SharedMemory* ResultMemory = new SharedMemory("ResultMemory",Cores);
    cout<<"PID :"<<pId<<" finsh his job\n";
    ResultMemory->setResult(Result,index);

    return 0;
}
