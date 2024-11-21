#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>

using namespace std;

// Configurações
const int PAGE_SIZE = 4; // Tamanho da página em KB
const int NUM_FRAMES = 4; // Número de quadros na memória física
const int NUM_PAGES = 8; // Número total de páginas na memória virtual

// Estruturas para memória virtual e TLB
struct PageTableEntry {
    int frameNumber;
    bool valid;
};

class VirtualMemory {
private:
    vector<int> physicalMemory; // Memória física (MP)
    unordered_map<int, PageTableEntry> pageTable; // Tabela de páginas
    unordered_map<int, int> TLB; // TLB (cache de traduções)
    queue<int> frameQueue; // Fila para substituição de páginas (FIFO)
    int tlbSize; // Capacidade da TLB

    // Substituição de página (FIFO)
    void replacePage(int virtualPage) {
        if (frameQueue.size() >= NUM_FRAMES) {
            int victimPage = frameQueue.front();
            frameQueue.pop();
            pageTable[victimPage].valid = false; // Marca página como inválida
        }
        int frameNumber = frameQueue.size();
        pageTable[virtualPage] = {frameNumber, true};
        frameQueue.push(virtualPage);
    }

    // Atualizar TLB
    void updateTLB(int virtualPage, int frameNumber) {
        if (TLB.size() >= tlbSize) {
            TLB.erase(TLB.begin()); // Remove a entrada mais antiga
        }
        TLB[virtualPage] = frameNumber;
    }

public:
    VirtualMemory(int tlbCapacity) : tlbSize(tlbCapacity) {
        physicalMemory.resize(NUM_FRAMES, -1); // Inicializa MP com -1
    }

    int translateAddress(int virtualAddress) {
        int virtualPage = virtualAddress / PAGE_SIZE;
        int offset = virtualAddress % PAGE_SIZE;

        // Busca na TLB
        if (TLB.find(virtualPage) != TLB.end()) {
            cout << "TLB hit!" << endl;
            return TLB[virtualPage] * PAGE_SIZE + offset;
        }

        // Busca na tabela de páginas
        if (pageTable[virtualPage].valid) {
            cout << "TLB miss, Page Table hit!" << endl;
            updateTLB(virtualPage, pageTable[virtualPage].frameNumber);
            return pageTable[virtualPage].frameNumber * PAGE_SIZE + offset;
        }

        // Falta de página
        cout << "Page fault!" << endl;
        replacePage(virtualPage);
        int frameNumber = pageTable[virtualPage].frameNumber;
        updateTLB(virtualPage, frameNumber);

        return frameNumber * PAGE_SIZE + offset;
    }

    void printStatus() {
        cout << "TLB: ";
        for (const auto &entry : TLB) {
            cout << "[VPage: " << entry.first << " -> Frame: " << entry.second << "] ";
        }
        cout << endl;

        cout << "Page Table: ";
        for (const auto &entry : pageTable) {
            cout << "[VPage: " << entry.first << " -> Frame: " 
                 << (entry.second.valid ? to_string(entry.second.frameNumber) : "Invalid") << "] ";
        }
        cout << endl;
    }
};

int main() {
    VirtualMemory vm(2); // Capacidade da TLB = 2

    vector<int> virtualAddresses = {0, 4, 8, 12, 16, 20, 4, 24, 28};

    for (int address : virtualAddresses) {
        cout << "Translating virtual address: " << address << endl;
        int physicalAddress = vm.translateAddress(address);
        cout << "Physical address: " << physicalAddress << endl << endl;
        vm.printStatus();
    }

    return 0;
}
