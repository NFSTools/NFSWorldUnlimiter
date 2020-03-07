/*
   MIT License
   Copyright (c) Berkay Yigit
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#pragma once
#include "pch.h"
#include <cstdarg>

namespace Memory {
    extern DWORD baseAddress;

    void   openMemoryAccess(const DWORD address, const int size);
    void   restoreMemoryAccess();
    DWORD  calculateRelativeAddress(const DWORD from, const DWORD to, const bool isFromAbsolute = true);
    DWORD  makeAbsolute(const DWORD relativeAddress);
    DWORD* readPointer(const DWORD baseOffset, const bool isBaseOffsetAbsolute = true, const int offsetCount = 0, ...);
    void   writeCall(const DWORD from, const DWORD to, const bool isFromAbsolute = true);
    void   writeJMP(const DWORD from, const DWORD to, const bool isFromAbsolute = true);
    void   writeInterrupt3(const DWORD to, const int amount, const bool isFromAbsolute = true);
    void   writeRet(const DWORD to, const bool isToAbsolute = true);
    void   writeNop(const DWORD to, const int amount, const bool isToAbsolute = true);
    void   writeRaw(const DWORD to, const bool isToAbsolute, const int byteCount, ...);
    void   Init();
}