#include <iostream>
#include <vector>
#include <time.h>

using namespace std;

typedef struct Partition {
	int id = 0;                     // Partition ID
	int size = 0;                   // Size of partition
	bool alloted = false;
	vector<int> noOfProc;           // List of process in partition
} part;

typedef struct Process {
	int id = 0;                     // Process ID
	int memory_required = 0;        // Required memory of the process
	int allocated_partition = 0;    // Allocated in partition
	bool allocated = false;     
} proc;

// Algorithm First fit
void FirstFit(Partition* listPart, int numOfPart, Process* listProc, int numOfProc) {
	for (int i = 0; i < numOfProc; i++) {
		for (int j = 0; j < numOfPart; j++) {
			if (listProc[i].memory_required > listPart[j].size) continue;
			else {
				listProc[i].allocated = true;
				listProc[i].allocated_partition = listPart[j].id;
				listPart[j].alloted = true;
				listPart[j].size -= listProc[i].memory_required;
				listPart[j].noOfProc.push_back(listProc[i].id);
				break;
			}
		}
	}
}
// Algorithm Best fit
void BestFit(Partition* listPart, int numOfPart, Process* listProc, int numOfProc) {
	for (int i = 0; i < numOfProc; i++) {
		int* satisfyingList = new int[numOfPart];
		vector<int> listKey;
		int noOfSatisfy = 0;
		for (int j = 0; j < numOfPart; j++) {
			if (listProc[i].memory_required > listPart[j].size) continue;
			else {
				satisfyingList[noOfSatisfy++] = listPart[j].id;
				listKey.push_back(j);
			}
		}
		if (noOfSatisfy != 0) {
			int current = satisfyingList[0], key = listKey[0];
			for (int j = 1; j < noOfSatisfy; j++) {
				if (listPart[listKey[j]].size < listPart[key].size) {
					key = listKey[j];
					current = satisfyingList[j];
				}
			}
			listProc[i].allocated = true;
			listProc[i].allocated_partition = current;
			listPart[key].alloted = true;
			listPart[key].size -= listProc[i].memory_required;
			listPart[key].noOfProc.push_back(listProc[i].id);
		}
	}
}
// Algorithm Worst fit
void WorstFit(Partition* listPart, int numOfPart, Process* listProc, int numOfProc) {
	for (int i = 0; i < numOfProc; i++) {
		int maxFreePart = listPart[0].id, key = 0;
		for (int j = 1; j < numOfPart; j++) {
			if (listPart[j].size > listPart[key].size) {
				maxFreePart = listPart[j].id;
				key = j;
			}
		}
		if (listProc[i].memory_required > listPart[key].size) continue;
		else {
			listProc[i].allocated = true;
			listProc[i].allocated_partition = maxFreePart;
			listPart[key].alloted = true;
			listPart[key].size -= listProc[i].memory_required;
			listPart[key].noOfProc.push_back(listProc[i].id);
		}
	}
}

// Print results
void PrintResults(Partition* listPart, int numOfPart, Process* listProc, int numOfProc) {
	cout << endl << "ProcID\tMemRequired\tAllocated\tAllocatedPartID" << endl;
	for (int i = 0; i < numOfProc; i++) {
		cout << listProc[i].id << "\t" << listProc[i].memory_required << "\t\t";
		if (listProc[i].allocated) {
			cout << "Yes\t\t";
			cout << listProc[i].allocated_partition << endl;
		}
		else {
			cout << "No\t\t";
			cout << "-" << endl;
		}
	}
	cout << endl << "PartID\tAlloted\t\tInterFrag\tProcIDInPart" << endl;
	for (int i = 0; i < numOfPart; i++) {
		cout << listPart[i].id << "\t";
		if (listPart[i].alloted) {
			cout << "Yes\t\t";
			cout << listPart[i].size << "\t\t";
			for (int j = 0; j < listPart[i].noOfProc.size(); j++) {
				cout << listPart[i].noOfProc[j];
				if (j != listPart[i].noOfProc.size() - 1) cout << ",";
			}
			cout << endl;
		}
		else {
			cout << "No\t\t";
			cout << listPart[i].size << "\t\t";
			cout << "-" << endl;;
		}
	}
	cout << "-------------------------------------------------------" << endl;
	cout << endl << endl;
}

void ChooseAlgorithm(Partition* listPart, int numOfPart, Process* listProc, int numOfProc, int totalMemory, char algorithm) {
	clock_t start, end;
	double timeUsed = 0;
	int totalInterFrag = 0, memoryRequiredNotAllocated = 0;
	cout << endl << "-------------------------------------------------------" << endl;
	switch (algorithm) {
	case 'F':
		cout << "               ~ FIRST FIT ALGORITHM ~                 " << endl;
		cout << "-------------------------------------------------------" << endl;
		start = clock();
		FirstFit(listPart, numOfPart, listProc, numOfProc);
		end = clock();
		break;
	case 'B':
		cout << "                ~ BEST FIT ALGORITHM ~                 " << endl;
		cout << "-------------------------------------------------------" << endl;
		start = clock();
		BestFit(listPart, numOfPart, listProc, numOfProc);
		end = clock();
		break;
	case 'W':
		cout << "               ~ WORST FIT ALGORITHM ~                 " << endl;
		cout << "-------------------------------------------------------" << endl;
		start = clock();
		WorstFit(listPart, numOfPart, listProc, numOfProc);
		end = clock();
		break;
	}
	cout << endl << "Total memory: " << totalMemory << endl;
	for (int i = 0; i < numOfPart; i++) {
		totalInterFrag += listPart[i].size;
	}
	cout << "Total internal fragmentation: " << totalInterFrag << endl;
	for (int i = 0; i < numOfProc; i++) {
		if (!listProc[i].allocated) memoryRequiredNotAllocated += listProc[i].memory_required;
	}
	cout << "Memory required not allocated: " << memoryRequiredNotAllocated << endl;
	timeUsed = (double)(((double)end - (double)start) / CLOCKS_PER_SEC) * 1000000;
	//cout << "Time rum algorithm: " << timeUsed << "us" << endl;
	PrintResults(listPart, numOfPart, listProc, numOfProc);
}

int main() {
	int numOfPart, numOfProc;
	int totalMemory = 0;
	//double timeUsed, timeUsed2, timeUsed3;
	cout << "Enter the number of partition: ";
	cin >> numOfPart;
	Partition* listPart = new Partition[numOfPart], * listPart2 = new Partition[numOfPart], * listPart3 = new Partition[numOfPart];
	cout << "Enter the size of each partition: ";
	for (int i = 0; i < numOfPart; i++) {
		listPart[i].id = listPart2[i].id = listPart3[i].id = i + 1;
		cin >> listPart[i].size;
		listPart2[i].size = listPart3[i].size = listPart[i].size;
		totalMemory += listPart[i].size;
	}
	cout << "Enter the number of process: ";
	cin >> numOfProc;
	Process* listProc = new Process[numOfProc], * listProc2 = new Process[numOfProc], * listProc3 = new Process[numOfProc];
	cout << "Enter memory required for each process: ";
	for (int i = 0; i < numOfProc; i++) {
		listProc[i].id = listProc2[i].id = listProc3[i].id = i + 1;
		cin >> listProc[i].memory_required;
		listProc2[i].memory_required = listProc3[i].memory_required = listProc[i].memory_required;
	}
	ChooseAlgorithm(listPart, numOfPart, listProc, numOfProc, totalMemory, 'F');
	ChooseAlgorithm(listPart2, numOfPart, listProc2, numOfProc, totalMemory, 'B');
	ChooseAlgorithm(listPart3, numOfPart, listProc3, numOfProc, totalMemory, 'W');
}

// Number of partition: 8
// Size of each partition: 80 100 150 247 78 54 179 136
// Num of process: 8
// Memory required of each process: 90 48 87 169 190 110 25 50
