#pragma once

#include <windows.h>

inline void Patch(void* address, void* data, size_t size, void* overWrittenBytes) {
    DWORD oldProtect;
    WINBOOL success = VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);

    if (!success) {
        // err handling?
    }

    if (overWrittenBytes) {
        memcpy(overWrittenBytes, address, size);
    }
    memcpy(address, data, size);

    success = VirtualProtect(address, size, oldProtect, &oldProtect);

    if (!success) {
        // err handling?
    }
}

/**
 *  \brief Calculates a relative jump offset from the given parameters
 *      and patches it in to the target address.
 * 
 *  \param patchAddress Address to patch
 *  \param jmpDestination Hook function address
 *  \param overWrittenBytes RetVal for the bytes overwritten by the patch. Can be null.
 */
inline void Patch_RelativeJump(void* patchAddress, void* jmpDestination, void* overWrittenBytes) {
    // CallsiteAddr + instructionSize + relJumpOffset = FuncAddress
    // relJumpOffset = FuncAddress - CallsiteAddr - instructionSize
    DWORD relativeJump = reinterpret_cast<DWORD>(jmpDestination) - reinterpret_cast<DWORD>(patchAddress) - sizeof(DWORD);
    Patch(patchAddress, &relativeJump, sizeof(relativeJump), overWrittenBytes);
}