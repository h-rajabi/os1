#ifndef CHILD_H
#define CHILD_H

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
#include <thread>

using namespace std;

int runChild(int index);
void findBestCables(FileOpenResult* Cables, FileOpenResult* Result, int64_t timee);
bool ran();
int CoresNumber();

#endif