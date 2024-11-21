#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

const int PAGE_SIZE = 4; 
const int NUM_FRAMES = 4; 
const int NUM_PAGES = 8;

struct PageTableEntry {
    int frameNumber;
    bool valid;
};

vector<int> physicalMemory(NUM_FRAMES, -1); 
vector<PageTableEntry> pageTable(NUM_PAGES, { -1, false }); 
vector<int> frameQueue; 
vector<int> TLB(NUM_FRAMES, -1); 
int tlbIndex = 0; 

void replacePage(int virtualPage) {
    if (frameQueue.size() >= NUM_FRAMES) {
        int victimPage = frameQueue[0];
        frameQueue.erase(frameQueue.begin());
        pageTable[victimPage].valid = false; 
    }
    int frameNumber = frameQueue.size();
    pageTable[virtualPage] = { frameNumber, true };
    frameQueue.push_back(virtualPage);
}

void updateTLB(int virtualPage, int frameNumber) {
    TLB[tlbIndex] = virtualPage;
    tlbIndex = (tlbIndex + 1) % NUM_FRAMES;
}

int translateAddress(int virtualAddress) {
    int virtualPage = virtualAddress / PAGE_SIZE;
    int offset = virtualAddress % PAGE_SIZE;

    for (int i = 0; i < NUM_FRAMES; i++) {
        if (TLB[i] == virtualPage) {
            cout << "TLB hit!" << endl;
            return pageTable[virtualPage].frameNumber * PAGE_SIZE + offset;
        }
    }

    if (pageTable[virtualPage].valid) {
        cout << "TLB miss, Page Table hit!" << endl;
        updateTLB(virtualPage, pageTable[virtualPage].frameNumber);
        return pageTable[virtualPage].frameNumber * PAGE_SIZE + offset;
    }

    cout << "Page fault!" << endl;
    replacePage(virtualPage);
    int frameNumber = pageTable[virtualPage].frameNumber;
    updateTLB(virtualPage, frameNumber);

    return frameNumber * PAGE_SIZE + offset;
}

void printStatus() {
    cout << "TLB: ";
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (TLB[i] != -1)
            cout << "[VPage: " << TLB[i] << "] ";
    }
    cout << endl;

    cout << "Page Table: ";
    for (int i = 0; i < NUM_PAGES; i++) {
        if (pageTable[i].valid)
            cout << "[VPage: " << i << " -> Frame: " << pageTable[i].frameNumber << "] ";
        else
            cout << "[VPage: " << i << " -> Invalido] ";
    }
    cout << endl;
}

int main() {
    vector<int> virtualAddresses = { 0, 4, 8, 12, 16, 20, 4, 24, 28 };

    for (int address : virtualAddresses) {
        cout << "Endereco virtual traduzido: " << address << endl;
        int physicalAddress = translateAddress(address);
        cout << "Endereco fisico: " << physicalAddress << endl << endl;
        printStatus();
    }

    return 0;
}
