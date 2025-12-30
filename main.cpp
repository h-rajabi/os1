
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <random>
#include <chrono>

using namespace std;

int updatesCount=0;

struct FileOpenResult {
    string fileName;
    int64_t finalNumber;
    vector<int64_t> numbers;
    int64_t totalLine;
};

struct ResultCables{
    int TID;
    int64_t finalNumber;
    vector<int64_t> numbers;
    int64_t totalLine;
};

random_device rd;
mt19937 gen(rd());
bernoulli_distribution dis(0.5);

//define parent function
bool readAndProcessFile();
void displayAllResult();
void displayFinalResult();

//define shared function
int CoresNumber();


//define child function
DWORD WINAPI MyThreadFunction( LPVOID lpParam );
typedef struct MyData {
    int index;
    int64_t time;
} MYDATA, *PMYDATA;// define arg child thread function

void runChild(int index, int64_t timee);


bool ran(){    
    bool random_bool = dis(gen);
    return random_bool;
}


FileOpenResult* input = new FileOpenResult;

ResultCables* result = new ResultCables;

HANDLE activeThreadSemaphore;
HANDLE readSemaphore;
HANDLE writeSemaphore;
HANDLE parentSemaphore;


int activeThreads = 0;
bool stop;

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
    // int Cores = 2;    
    PMYDATA pDataArray[Cores];
    DWORD   dwThreadIdArray[Cores];
    HANDLE  hThreadArray[Cores]; 

    result->finalNumber = input->finalNumber *2;

    readSemaphore = CreateSemaphore(NULL,0,1,NULL);
    if (readSemaphore == NULL) {
        printf("error in create semaphore: %d\n", GetLastError());
        return 1;
    }
    
    writeSemaphore = CreateSemaphore(NULL,1,1,NULL);
    if (writeSemaphore == NULL) {
        printf("error in create semaphore: %d\n", GetLastError());
        return 1;
    }

    parentSemaphore = CreateSemaphore(NULL,0,1,NULL);
    if (parentSemaphore == NULL) {
        printf("error in create semaphore: %d\n", GetLastError());
        return 1;
    }
    
    activeThreadSemaphore = CreateSemaphore(NULL,1,1,NULL);
    if (activeThreadSemaphore == NULL) {
        printf("error in create semaphore: %d\n", GetLastError());
        return 1;
    }
    
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
    
    while (!stop)
    {
        WaitForSingleObject(parentSemaphore, INFINITE);
        if (stop)
        {
            break;
        }
        cout<<"thread :"<< result->TID <<" find better result: "<< result->finalNumber - input->finalNumber<<endl;
        ReleaseSemaphore(readSemaphore, 1, NULL);
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
    // displayFinalResult();

    cout<<"\nupdates Count = "<<updatesCount<<endl;

    CloseHandle(readSemaphore);
    CloseHandle(writeSemaphore);
    CloseHandle(parentSemaphore);
    CloseHandle(activeThreadSemaphore);

    delete input;
    delete result;

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

void displayFinalResult(){
    cout<<"end of program \n";
    cout<<"the result is :"<<result->finalNumber<<endl;
    cout<<"The difference between your number ("<<input->finalNumber<<") and the result ("<<result->finalNumber<<") is :" << result->finalNumber - input->finalNumber <<"\n";
    cout<<"program chose this cables :\n";
    
    for (size_t i = 0; i < result->numbers.size(); i++)
    {
        cout<<result->numbers[i]<<",";
    }
    cout<<"also we chosse "<<result->numbers.size()<<" cables.\n";
}

// shared
int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}


// child code
DWORD WINAPI MyThreadFunction( LPVOID lpParam ){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    runChild(pDataArray->index, pDataArray->time);

    return 0;
}

void runChild(int index, int64_t timee) {
    auto startTime = chrono::steady_clock::now();
    vector<int64_t> TempResult;
    int64_t sum = 0 ;
    
    WaitForSingleObject(activeThreadSemaphore, INFINITE);
    activeThreads++;
    if (activeThreads == 1)
    {
        stop = false;
    }
    ReleaseSemaphore(activeThreadSemaphore, 1, NULL);

    bool r;
    bool loop = true;
    while (loop) {
        // cout<<"TID:"<< index <<"wait for search\n";
        // WaitForSingleObject(readSemaphore, INFINITE);
        // cout<<"TID:"<< index <<"start for search\n";
		auto now = chrono::steady_clock::now();
		auto spent = chrono::duration_cast<chrono::seconds>(now - startTime);
		if (spent.count() >= timee) {
            // ReleaseSemaphore(readSemaphore, 1, NULL);
			break;
		}
        sum=0;
        TempResult.clear();
        for (size_t i = 0; i < input->totalLine -1; i++)
        {   
            r=ran();
            if (r)
            {
                sum += input->numbers[i];
                TempResult.push_back(input->numbers[i]);
                if (sum > input->finalNumber && sum < result->finalNumber)
                {   
                    WaitForSingleObject(writeSemaphore, INFINITE);
                    if (sum < result->finalNumber)
                    {
                        result->TID = index;
                        result->numbers = TempResult;
                        result->finalNumber = sum;
                        updatesCount++;

                        ReleaseSemaphore(parentSemaphore, 1, NULL);
                        WaitForSingleObject(readSemaphore, INFINITE);
                    }
                    ReleaseSemaphore(writeSemaphore, 1, NULL);
                    break;
                }
            }
        }
        
    }

    WaitForSingleObject(activeThreadSemaphore,INFINITE);
    activeThreads--;
    if (activeThreads == 0)
    {
        stop = true;
        ReleaseSemaphore(parentSemaphore, 1, NULL);
    }
    ReleaseSemaphore(activeThreadSemaphore, 1, NULL);
}
