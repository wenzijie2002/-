#include "relocation.h"

#include <sys/mman.h>

void handleRela(std::vector<ObjectFile> &allObject, ObjectFile &mergedObject, bool isPIE)
{
    /* When there is more than 1 objects,
     * you need to adjust the offset of each RelocEntry
     */
    /* Your code here */
    // Compute total code length for adjusting RelocEntry offset
    uint64_t currentOffset = 0;
    for (ObjectFile &obj : allObject)
    {
        for (RelocEntry &reloc : obj.relocTable)
        {
            reloc.offset += currentOffset; // Adjust the offset
        }
        currentOffset += obj.sections[".text"].size;
    }
    /* in PIE executables, user code starts at 0xe9 by .text section */
    /* in non-PIE executables, user code starts at 0xe6 by .text section */
    uint64_t userCodeStart = isPIE ? 0xe9 : 0xe6;
    uint64_t textOff = mergedObject.sections[".text"].off + userCodeStart;
    uint64_t textAddr = mergedObject.sections[".text"].addr + userCodeStart;

    /* Your code here */
    // uint64_t currentTextSize = 0;

    for (auto &obj : allObject)
    {
        for (auto &reloc : obj.relocTable)
        {
            // Compute the relocation address
            uint64_t addr = textOff + reloc.offset;

            // Compute the symbol address
            uint64_t symAddr = reloc.sym->value;
            uint64_t finalAddr;
            // Compute the final relocated address
            if (reloc.type == R_X86_64_PC32 || reloc.type == R_X86_64_PLT32)
            {
                finalAddr = symAddr - (reloc.offset + textAddr) + reloc.addend;
            }
            else
            {
                finalAddr = symAddr + reloc.addend;
            }
            // Update the relocation entry
            *reinterpret_cast<int *>(mergedObject.baseAddr + addr) = finalAddr;
        }
    }
}