// #include "SharedMemory.h"
// #include "child.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <random>
#include <chrono>
// #include <thread>

using namespace std;

struct FileOpenResult {
    string fileName;
    int64_t finalNumber;
    vector<int64_t> numbers;
    int64_t totalLine;
};

struct ResultCables{
    int64_t finalNumber;
    vector<int64_t> numbers;
    int64_t totalLine;
};

random_device rd;
mt19937 gen(rd());
bernoulli_distribution dis(0.5);
//define parent function
bool readAndProcessFile();
void displayResults();
void readResult();
void displayAllResult();
void displayFinalResult(vector<int64_t> numbers, int64_t finalNumber);

//define shared function
int CoresNumber();
// void ErrorHandler(LPCTSTR lpszFunction);

//define child function
DWORD WINAPI MyThreadFunction( LPVOID lpParam );
typedef struct MyData {
    int index;
    int64_t time;
} MYDATA, *PMYDATA;

void runChild(int index, int64_t timee);
void findBestCables(ResultCables* Result, int64_t timee);
void writeResult(int index, ResultCables* Result);

bool ran(){    
    bool random_bool = dis(gen);
    return random_bool;
}


FileOpenResult* input = new FileOpenResult;

ResultCables* result = new ResultCables;

int main() {
    
    string filename = "random_numbers.txt";
    // string filename = "test1.txt";
    int64_t finalNumber;
    vector<int64_t> numbers;

    input->fileName=filename;
    if (!readAndProcessFile()) {
        // displayResults(result);
        return 0;
    }

    int64_t timee;
    cout << "input time for searching for best result:";
	cin >> timee;
	cin.ignore();
    
    int Cores = CoresNumber();
    // cout<<"cores :"<<Cores<<"/n";
    // int Cores = 1;
    result->numbers.resize(input->totalLine * Cores);
    result->totalLine = Cores * input->totalLine;
    // cout<<"size result:"<<result->numbers.size()<<endl;
    PMYDATA pDataArray[Cores];
    DWORD   dwThreadIdArray[Cores];
    HANDLE  hThreadArray[Cores]; 


    // vector<thread> threads;
    
    for (int i = 0; i < Cores; i++) {
        // Allocate memory for thread data.
        pDataArray[i] = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,sizeof(MYDATA));
        
        if( pDataArray[i] == NULL )
        {
            ExitProcess(2);
        }

        pDataArray[i]->index= i;
        pDataArray[i]->time= timee;

        hThreadArray[i] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction,       // thread function name
            pDataArray[i],          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier
        
        if (hThreadArray[i] == NULL) 
        {
            cout<<"error in create thread/n";
        //    ErrorHandler(TEXT("CreateThread"));
           ExitProcess(3);
        }
	}
    
    WaitForMultipleObjects(Cores, hThreadArray, TRUE, INFINITE);
    
    for(int i=0; i<Cores; i++)
    {
        CloseHandle(hThreadArray[i]);
        if(pDataArray[i] != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pDataArray[i]);
            pDataArray[i] = NULL;    // Ensure address is not reused.
        }
    }
    // cout<<"ok in cilds\n";
    readResult();

    return 0;
}

//parent
bool readAndProcessFile() {
    ifstream file(input->fileName);
    
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << input->fileName << endl;
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
                input->finalNumber = number;
                firstLine = false;
            } else {
                input->numbers.push_back(number);
            }
        } else {
            cout << "Warning: Line " << totalLines << " is not a valid number: " << line << endl;
        }
    }
    input->totalLine = totalLines;
    file.close();
    return true;
}

void displayResults() {
    cout << "=== File Processing Results ===" << endl;
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

void readResult(){
    // displayAllResult();
    int64_t min = result->numbers[0];

    int jm = 0;
    int j=0;
    int size_array= CoresNumber() * input->totalLine;
    for (size_t i = result->totalLine ; i < size_array ; i += result->totalLine)
    {
        j++;
        if (result->numbers[i] < min)
        {
            min = result->numbers[i];
            jm=j;
        }
    }
    // cout<<"find best in index :"<<jm<<endl;

    vector<int64_t> final;
    int64_t fnumber;
    int64_t index = jm * result->totalLine;
    fnumber=result->numbers[index];
    for (size_t i = index+1 ; i < ( index + result->totalLine); i++)
    {
        if (result->numbers[i] != 0)
        {
            final.push_back(result->numbers[i]);
        }
        else break;
    }
    displayFinalResult(final, fnumber);
}

void displayAllResult(){
    cout<<"total lines:"<<result->totalLine<<endl;
    cout<<"first line:"<<result->numbers[0]<<endl;
    for (int i = 0; i < result->totalLine; i++)
    {
        if (i%result->totalLine == 0)
        {
            cout<<"final number in index :"<<i%result->totalLine<<endl;
        }else {
            cout<<"i:"<<i<<result->numbers[i]<<endl;
        }
    }
}

void displayFinalResult(vector<int64_t> numbers, int64_t finalNumber){
    cout<<"end of program \n";
    cout<<"the result is :"<<finalNumber<<endl;
    cout<<"The difference between your number ("<<input->finalNumber<<") and the result ("<<finalNumber<<") is :" << finalNumber - input->finalNumber <<"\n";
    cout<<"program chose this cables :\n";
    
    for (size_t i = 0; i < numbers.size(); i++)
    {
        cout<<numbers[i]<<",";
    }
    cout<<"also we chosse "<<numbers.size()<<" cables.\n";
}

// shared
int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

// void ErrorHandler(LPCTSTR lpszFunction) 
// { 
//     // Retrieve the system error message for the last-error code.

//     LPVOID lpMsgBuf;
//     LPVOID lpDisplayBuf;
//     DWORD dw = GetLastError(); 

//     FormatMessage(
//         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
//         FORMAT_MESSAGE_FROM_SYSTEM |
//         FORMAT_MESSAGE_IGNORE_INSERTS,
//         NULL,
//         dw,
//         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//         (LPTSTR) &lpMsgBuf,
//         0, NULL );

//     // Display the error message.

//     lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
//         (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR)); 
//     StringCchPrintf((LPTSTR)lpDisplayBuf, 
//         LocalSize(lpDisplayBuf) / sizeof(TCHAR),
//         TEXT("%s failed with error %d: %s"), 
//         lpszFunction, dw, lpMsgBuf); 
//     MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK); 

//     // Free error-handling buffer allocations.

//     LocalFree(lpMsgBuf);
//     LocalFree(lpDisplayBuf);
// }

// child code

DWORD WINAPI MyThreadFunction( LPVOID lpParam ){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    // cout<<"index :"<<pDataArray->index <<"and time :"<<pDataArray->time<<"\n";
    runChild(pDataArray->index, pDataArray->time);

    return 0;
}


void runChild(int index, int64_t timee) {
    
    int Cores =CoresNumber();

    ResultCables* resultt = new ResultCables;

    cout<<"TID :"<< index <<" start his job\n";
    findBestCables(resultt, timee);
    cout<<"TID :"<< index <<" finsh his job\n";
    writeResult(index, resultt);

}

void findBestCables(ResultCables* Result, int64_t timee){
    auto startTime = chrono::steady_clock::now();
    vector<int64_t> TempResult;
    int64_t min = -1;
    
    vector<int64_t> TempSum;
    int64_t sum = 0 ;
    
    bool r;
    bool loop = true;
    int64_t t =0;
    while (loop) {
        t++;
		auto now = chrono::steady_clock::now();
		auto spent = chrono::duration_cast<chrono::seconds>(now - startTime);
		if (spent.count() >= timee) {
			break;
		}
        sum=0;
        TempSum.clear();
        for (size_t i = 0; i < input->totalLine -1; i++)
        {   
            r=ran();
            if (r)
            {
                TempSum.push_back(input->numbers[i]);
                sum += input->numbers[i];
            
                if (sum > input->finalNumber )
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
                else if (sum == input->finalNumber ){
                    min=sum;
                    TempResult = TempSum;
                    // cout << "Found best solution, size:" << TempResult.size() << " sum:" << sum << endl;
                    loop=false;
                    break;
                }
            }
        }
    }


    Result->totalLine=input->totalLine;
    Result->finalNumber = min;
    Result->numbers=TempResult;
    cout<<"thread t :"<<t<<endl;
}

void writeResult(int index, ResultCables* Result){

    int j = index * input->totalLine;
    if (index ==0)
    {
        cout<<"index 0 find :"<<Result->numbers[0]<<endl;
    }
    
    result->numbers[j] = Result->finalNumber;
    int h =0;
    for (size_t i = j + 1; i < (j + Result->totalLine) -1; i++)
    {
        if (h >= Result->numbers.size())
        {
            result->numbers[i] = 0;
        }else
        {
            result->numbers[i]= Result->numbers[h];
            h++;    
        }
    }
}

