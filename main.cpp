// #include "SharedMemory.h"
// #include "child.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <thread>

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
}

random_device rd;
mt19937 gen(rd());
bernoulli_distribution dis(0.5);
//define parent function
bool readAndProcessFile();
void displayResults();
void readResult();
void displayFinalResult(vector<int64_t> numbers, int64_t finalNumber);

//define shared function
int CoresNumber();

//define child function
int runChild(int index);
void findBestCables(ResultCables* Result);
void writeResult(int index, ResultCables* Result);

bool ran(){    
    bool random_bool = dis(gen);
    return random_bool;
}


FileOpenResult* start = new FileOpenResult;

ResultCables* result = new ResultCables;
int64_t timee;

int main() {
    
    string filename = "random_numbers.txt";
    int64_t finalNumber;
    vector<int64_t> numbers;

    start->fileName=filename;
    if (!readAndProcessFile(start)) {
        // displayResults(result);
        return 0;
    }

    
    cout << "input time for searching for best result:";
	cin >> timee;
	cin.ignore();
    
    int Cores = CoresNumber();
    // int Cores = 1;
    result->numbers.resize(start->totalLine * Cores);

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

    for (auto& t : threads) {
        t.join();
    }

    ResultMemory->readResultFromRAM();
    return 0;
}

//parent
bool readAndProcessFile() {
    ifstream file(start->fileName);
    
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << start->fileName << endl;
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
                start->finalNumber = number;
                firstLine = false;
            } else {
                start->numbers.push_back(number);
            }
        } else {
            cout << "Warning: Line " << totalLines << " is not a valid number: " << line << endl;
        }
    }
    start->totalLine = totalLines;
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
    int64_t min = result->numbers[0];

    int jm = 0;
    int j=0;
    int size_array= CoresNumber() * start->totalLine;
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

void displayFinalResult(vector<int64_t> numbers, int64_t finalNumber){
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

// shared
int CoresNumber(){
    SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	int Cores = sys.dwNumberOfProcessors;
	Cores = Cores - 1;
    return Cores;
}

// child code
int runChild(int index) {
    
    int Cores =CoresNumber();

    ResultCables* resultt = new ResultCables;

    cout<<"TID :"<<this_thread::get_id()<<" start his job\n";
    findBestCables(resultt);

    cout<<"TID :"<<this_thread::get_id()<<" finsh his job\n";
    writeResult(index, resultt);

    return 0;
}

void findBestCables(ResultCables* Result){
    auto startTime = chrono::steady_clock::now();
    vector<int64_t> TempResult;
    int64_t min = -1;
    
    vector<int64_t> TempSum;
    int64_t sum = 0 ;
    
    bool r;
    bool loop = true;
    while (loop) {
		auto now = chrono::steady_clock::now();
		auto spent = chrono::duration_cast<chrono::seconds>(now - startTime);
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

void writeResult(int index, ResultCables* Result){

    int j = index * start->totalLine;

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

