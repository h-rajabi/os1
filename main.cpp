#include "SharedMemory.h"
#include "child.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <thread>

using namespace std;


int main() {
    
    string filename = "random_numbers.txt";
    int64_t finalNumber;
    int64_t timee;
    vector<int64_t> numbers;
    FileOpenResult* result = new FileOpenResult;

    result->fileName=filename;
    if (!readAndProcessFile(result)) {
        // displayResults(result);
        return 0;
    }


    cout << "input time for searching for best result:";
	cin >> timee;
	cin.ignore();
    
    int Cores = CoresNumber();
    // int Cores = 1;

    SharedMemory* fileMemory = new SharedMemory("FileMemory",result,Cores,0);
    fileMemory->writeFileToRAM(timee);

    // SharedMemory* MapMemory = new SharedMemory("MapMemory",result,Cores,1);
    // vector<thread::id> mapv;

    SharedMemory* ResultMemory = new SharedMemory("ResultMemory",result,Cores,2);

    vector<thread> threads;
    
    for (int i = 0; i < Cores; i++) {
        thread t(runChild,i);
        if (t.joinable())
        {
            cout << "Create Thread sucess with TID:"<< t.get_id() << endl;
            threads.push_back(move(t));
            // mapv.push_back(t.get_id());
        }else{
            cout << "Create Thread failed!\n" << endl;
            return 0;
        }
	}
    // MapMemory->writeMapToRAM(mapv);

    for (auto& t : threads) {
        t.join();
    }

    // ResultMemory->readAllResultFromRAM();
    ResultMemory->readResultFromRAM();
    return 0;
}
