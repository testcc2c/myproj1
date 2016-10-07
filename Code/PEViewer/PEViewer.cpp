// PEViewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
//#include <winnt.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8;

void PrintLastError(LPCTSTR msg)
{
    DWORD err = GetLastError();
    _tprintf(_T("%s (%u, 0x%x)\n"), msg, err, err);
    exit(1);
}

void PrintError(LPCTSTR msg)
{
    _tprintf(_T("%s\n"), msg);
    exit(1);
}

int _tmain(int argc, _TCHAR* argv[])
{
    LPCTSTR fileName = _T("test.exe");
    HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        PrintLastError(_T("failed to open file"));
    }

    HANDLE mapObj = CreateFileMapping(file, nullptr, PAGE_READONLY /*| SEC_IMAGE*/, 0, 0, nullptr);
    if (mapObj == NULL) {
        PrintLastError(_T("failed to create mapping object"));
    }

    uint8* startOfImage = (uint8*)MapViewOfFile(mapObj, FILE_MAP_READ, 0, 0, 0);
    if (!startOfImage) {
        PrintLastError(_T("failed to MapViewOfFile"));
    }

    {
        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)startOfImage;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            PrintError(_T("this is not PE file!\n"));
        }

        _tprintf(_T("\nIMAGE_DOS_HEADER\n"));
        _tprintf(_T("e_magic(0x%x) : %x // Magic number\n"), (uint8*)&dosHeader->e_magic - (uint8*)dosHeader, dosHeader->e_magic);
        _tprintf(_T("e_lfanew(0x%x) : %d, %x // File address of new exe header\n"), (uint8*)&dosHeader->e_lfanew - (uint8*)dosHeader, dosHeader->e_lfanew, dosHeader->e_lfanew);


        IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(startOfImage + dosHeader->e_lfanew);
        if (ntHeader->Signature != IMAGE_NT_SIGNATURE) {
            PrintError(_T("this is not NT PE file!\n"));
        }

        _tprintf(_T("\nIMAGE_NT_HEADER\n"));
        _tprintf(_T("Signature(0x%x) : %x // Magic number\n"), (uint8*)&ntHeader->Signature - (uint8*)ntHeader, ntHeader->Signature);

        IMAGE_FILE_HEADER* imgFileHeader = &ntHeader->FileHeader;
        _tprintf(_T("\nIAMGE_FILE_HEADER\n"));

        _tprintf(_T("Machine : %x\n"), imgFileHeader->Machine);
        _tprintf(_T("NumberOfSections : %d\n"), imgFileHeader->NumberOfSections);

        SYSTEMTIME st;
        FILETIME ft;
        ULARGE_INTEGER li;

        memset(&st, 0x00, sizeof(st));
        st.wYear = 1970, st.wMonth = 1, st.wDay = 1, st.wHour = 9;
        SystemTimeToFileTime(&st, &ft);

        unsigned __int64* ptr = (unsigned __int64*)&ft;
        *ptr += (unsigned __int64)imgFileHeader->TimeDateStamp * 10000000;
        FileTimeToSystemTime(&ft, &st);

        _tprintf(_T("TimeDateStamp : %x (%04d/%02d/%02d-%02d:%02d:%02d)\n"), 
            imgFileHeader->TimeDateStamp, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

        _tprintf(_T("PointerToSymbolTable : 0x%x\n"), imgFileHeader->PointerToSymbolTable);
        _tprintf(_T("NumberOfSymbols : %d\n"), imgFileHeader->NumberOfSymbols);
        _tprintf(_T("SizeOfOptionalHeader : %d\n"), imgFileHeader->SizeOfOptionalHeader);
        _tprintf(_T("Characteristics : 0x%x\n"), imgFileHeader->Characteristics);

        _tprintf(_T("\nIAMGE_OPTIONAL_HEADER\n"));
        IMAGE_OPTIONAL_HEADER32* imgOptHeader = &ntHeader->OptionalHeader;
        IMAGE_DATA_DIRECTORY* imgDataDirectory = nullptr;

        if (imgOptHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            _tprintf(_T("LinverVersion %d.%d\n"), imgOptHeader->MajorLinkerVersion, imgOptHeader->MinorLinkerVersion);
            _tprintf(_T("SizeOfCode %u, 0x%x\n"), imgOptHeader->SizeOfCode, imgOptHeader->SizeOfCode);
            _tprintf(_T("SizeOfInitializedData %u, 0x%x\n"), imgOptHeader->SizeOfInitializedData, imgOptHeader->SizeOfInitializedData);
            _tprintf(_T("SizeOfUninitializedData %u, 0x%x\n"), imgOptHeader->SizeOfUninitializedData, imgOptHeader->SizeOfUninitializedData);
            _tprintf(_T("AddressOfEntryPoint 0x%x\n"), imgOptHeader->AddressOfEntryPoint);
            _tprintf(_T("BaseOfCode 0x%x\n"), imgOptHeader->BaseOfCode);
            _tprintf(_T("BaseOfData 0x%x\n"), imgOptHeader->BaseOfData);
            _tprintf(_T("ImageBase 0x%x\n"), imgOptHeader->ImageBase);
            _tprintf(_T("SectionAlignment %u, 0x%x\n"), imgOptHeader->SectionAlignment, imgOptHeader->SectionAlignment);
            _tprintf(_T("FileAlignment %u, 0x%x\n"), imgOptHeader->FileAlignment, imgOptHeader->FileAlignment);
            _tprintf(_T("OSVersion %d, %d\n"), imgOptHeader->MajorOperatingSystemVersion, imgOptHeader->MinorOperatingSystemVersion);
            _tprintf(_T("ImageVersion %d, %d\n"), imgOptHeader->MajorImageVersion, imgOptHeader->MinorImageVersion);
            _tprintf(_T("SubsystemVersion %d, %d\n"), imgOptHeader->MajorSubsystemVersion, imgOptHeader->MinorSubsystemVersion);
            _tprintf(_T("Win32VersionValue %u\n"), imgOptHeader->Win32VersionValue);
            _tprintf(_T("SizeOfImage %u\n"), imgOptHeader->SizeOfImage);
            _tprintf(_T("SizeOfHeaders %u\n"), imgOptHeader->SizeOfHeaders);
            _tprintf(_T("CheckSum 0x%x\n"), imgOptHeader->CheckSum);
            _tprintf(_T("SubSystem %u\n"), imgOptHeader->Subsystem);
            _tprintf(_T("DllCharacteristics 0x%x\n"), imgOptHeader->DllCharacteristics);
            _tprintf(_T("LoaderFlags 0x%x\n"), imgOptHeader->DllCharacteristics);
            _tprintf(_T("NumberOfRvaAndSizes %u\n"), imgOptHeader->NumberOfRvaAndSizes);

            imgDataDirectory = imgOptHeader->DataDirectory;

        } else if (imgOptHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            // 
        }


        LPCTSTR imageDirEntryNames[] = {
            _T("IMAGE_DIRECTORY_ENTRY_EXPORT"),
            _T("IMAGE_DIRECTORY_ENTRY_IMPORT"),
            _T("IMAGE_DIRECTORY_ENTRY_RESOURCE"),
            _T("IMAGE_DIRECTORY_ENTRY_EXCEPTION"),
            _T("IMAGE_DIRECTORY_ENTRY_SECURITY"),
            _T("IMAGE_DIRECTORY_ENTRY_BASERELOC"),
            _T("IMAGE_DIRECTORY_ENTRY_DEBUG"),
            _T("IMAGE_DIRECTORY_ENTRY_ARCHITECTURE"),
            _T("IMAGE_DIRECTORY_ENTRY_GLOBALPTR"),
            _T("IMAGE_DIRECTORY_ENTRY_TLS"),
            _T("IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG"),
            _T("IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT"),
            _T("IMAGE_DIRECTORY_ENTRY_IAT"),
            _T("IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT"),
            _T("IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR"),
            _T("IMAGE_DIRECTORY_ENTRY_NULL"),
        };

        for (int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; ++i) {
            _tprintf(_T("%s va(0x%x), size(%d)\n"), imageDirEntryNames[i], imgDataDirectory[i].VirtualAddress, imgDataDirectory[i].Size);
        }


        //IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(startOfImage + dosHeader->e_lfanew);
        IMAGE_SECTION_HEADER* imageSectionHeader = nullptr;
        
        if (imgOptHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            imageSectionHeader = (IMAGE_SECTION_HEADER*)(startOfImage + dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
        } else if (imgOptHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            //
        }
        

        for (int i = 0; i < imgFileHeader->NumberOfSections; ++i) {
            _tprintf(_T("\nIMAGE_SECTION_HEADER\n"));
            IMAGE_SECTION_HEADER& h = imageSectionHeader[i];
            printf("name %s\n", h.Name);
            _tprintf(_T("VirtualSize %u\n"), h.Misc.VirtualSize);
            _tprintf(_T("VirtualAddress 0x%x\n"), h.VirtualAddress);
            _tprintf(_T("SizeOfRawData %u\n"), h.SizeOfRawData);
            _tprintf(_T("PointerToRawData 0x%x\n"), h.PointerToRawData);
            _tprintf(_T("PointerToRelocations 0x%x\n"), h.PointerToRelocations);
            _tprintf(_T("PointerToLinenumbers 0x%x\n"), h.PointerToLinenumbers);
            _tprintf(_T("NumberOfRelocations %u\n"), h.NumberOfRelocations);
            _tprintf(_T("NumberOfLinenumbers %u\n"), h.NumberOfLinenumbers);
            _tprintf(_T("Characteristics 0x%x\n"), h.Characteristics);
            _tprintf(_T("====================================================\n"), h.Characteristics);
        }

        
    }
    

    CloseHandle(mapObj);
    CloseHandle(file);




	return 0;
}
