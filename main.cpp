#include "SharedMemory.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

using namespace std;


int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

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

    SharedMemory* MapMemory = new SharedMemory("MapMemory",result,Cores,1);
    vector<int> mapv;

    SharedMemory* ResultMemory = new SharedMemory("ResultMemory",result,Cores,2);

    STARTUPINFO* childs = new STARTUPINFO[Cores];
    PROCESS_INFORMATION* prosess= new PROCESS_INFORMATION[Cores];

    for (int i = 0; i < Cores; i++) {
        ZeroMemory(&childs[i], sizeof(childs[i]));
        childs[i].cb = sizeof(childs[i]);
        ZeroMemory(&prosess[i], sizeof(prosess[i]));
        bool b = CreateProcess(TEXT("child.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &childs[i], &prosess[i]);
        if (!b)
        {
            cout << "Create Process failed" << endl;
            return 0;
        }
        cout << "Create Process sucess with PID:"<< prosess[i].dwProcessId << endl;
        mapv.push_back(prosess[i].dwProcessId);
	}
    MapMemory->writeMapToRAM(mapv);

    for (int i = 0; i < Cores ; i++)
		WaitForSingleObject(prosess[i].hProcess, INFINITE);


    // ResultMemory->readAllResultFromRAM();
    ResultMemory->readResultFromRAM();
    return 0;
}
