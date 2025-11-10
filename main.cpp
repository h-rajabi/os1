#include "SharedMemory.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

using namespace std;

// class SharedMemory;

// struct FileOpenResult {
//     string fileName;
//     int64_t finalNumber;
//     vector<int64_t> numbers;
//     int totalLine;
// };

// bool readAndProcessFile(FileOpenResult* result) {
//     ifstream file(result->fileName);
    
//     if (!file.is_open()) {
//         cout << "Error: Cannot open file " << result->fileName << endl;
//         return false;
//     }
    
//     string line;
//     int totalLines = 0;
//     bool firstLine = true;
    
//     while (getline(file, line)) {
//         totalLines++;
        
//         size_t start = line.find_first_not_of(" \t");
//         size_t end = line.find_last_not_of(" \t");
        
//         if (start == string::npos || end == string::npos) {
//             continue;
//         }
        
//         line = line.substr(start, end - start + 1);
        
//         stringstream ss(line);
//         int64_t number;
        
//         if (ss >> number) {
//             if (firstLine) {
//                 result->finalNumber = number;
//                 firstLine = false;
//             } else {
//                 result->numbers.push_back(number);
//             }
//         } else {
//             cout << "Warning: Line " << totalLines << " is not a valid number: " << line << endl;
//         }
//     }
//     result->totalLine = totalLines;
//     file.close();
//     return true;
// }

// void displayResults(FileOpenResult* result) {
//     cout << "=== File Processing Results ===" << endl;
//     cout << "File name: " << result->fileName << endl;
//     cout << "Final number (first line): " << result->finalNumber << endl;
//     cout << "Total lines in file: " << result->totalLine << endl;
//     cout << "Random numbers count: " << result->numbers.size() << endl;
//     cout << "Random numbers: ";
    
//     for (size_t i = 0; i < result->numbers.size(); ++i) {
//         cout << result->numbers[i];
//         if (i < result->numbers.size() - 1) {
//             cout << ", ";
//         }
//     }
//     cout << endl;
// }

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
    if (readAndProcessFile(result)) {
        displayResults(result);
    }else return 0;


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
