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
#include <cstdint>

using namespace std;

// struct fileOpenResult {
//     string fileName;
//     int64_t finalNumber;
//     vector<int64_t> numbers;
//     int totalLine;
// };

random_device rd;
mt19937 gen(rd());
bernoulli_distribution dis(0.5);


int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

bool ran(){
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
    bool loop = true;
    while (loop) {
		auto now = chrono::steady_clock::now();
		auto spent = chrono::duration_cast<chrono::seconds>(now - start);
		if (spent.count() >= timee) {
			break;
		}
        sum=0;
        TempSum.clear();
        for (size_t i = 0; i < Cables->totalLine -1; i++)
        {   
            r=ran();
            if (r)
            {
                TempSum.push_back(Cables->numbers[i]);
                sum += Cables->numbers[i];
            
                if (sum > Cables->finalNumber )
                {
                    if(min == -1){
                        min=sum;
                        TempResult = TempSum;
                        // cout << "Found first solution, size:" << TempResult.size() << " sum:" << sum << endl;
                        break;
                    }
                    else if ( min > sum )
                    {
                        min=sum;
                        TempResult = TempSum;
                        // cout << "Found better solution, size:" << TempResult.size() << " sum:" << sum << endl;
                        break;
                    }
                    else break;
                }
                else if (sum == Cables->finalNumber ){
                    min=sum;
                    TempResult = TempSum;
                    // cout << "Found best solution, size:" << TempResult.size() << " sum:" << sum << endl;
                    loop=false;
                    break;
                }
            }
        }
    }


    Result->totalLine=Cables->totalLine;
    Result->finalNumber = min;
    Result->numbers=TempResult;

}

int main() {
    
    int Cores =CoresNumber();
    int pId = GetCurrentProcessId();
    int64_t timee;
    SharedMemory* FileMemory = new SharedMemory("FileMemory",Cores,0);
    
    timee = FileMemory->readFileFromRAM();
    
    FileOpenResult* Cables = FileMemory->GetResult();
    FileOpenResult* Result = new FileOpenResult;
    
    random_device rd;
    mt19937 gen(rd());
    bernoulli_distribution dis(0.5);

    cout<<"PID :"<<pId<<" start his job\n";
    findBestCables(Cables,Result,timee);

    SharedMemory* MapMemory = new SharedMemory("MapMemory",Cores,1);

    int index = MapMemory->readMapFromRAM(pId);
    if (index == -1 )
    {
        cout<<"Error can`t find pID : "<<pId<<" in mapMemory\n";
        return 0;
    }
    SharedMemory* ResultMemory = new SharedMemory("ResultMemory",Cores,2);
    cout<<"PID :"<<pId<<" finsh his job\n";
    ResultMemory->setResult(Result,index);

    return 0;
}
