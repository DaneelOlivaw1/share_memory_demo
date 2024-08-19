#include <windows.h>
#include <iostream>

#define SHARED_MEM_NAME L"MySharedMemory"
#define SHARED_SIZE_NAME L"MySharedSize"
#define MAX_SHARED_MEM_SIZE 1 * 1024 * 1024 // 1MB, adjust as needed

int main() {
    const wchar_t* filePath = L"C:\\Users\\daneel\\Documents\\YOLOV8\\YOLOv8ql\\YOLOv8-CUDA11.3\\1\\datasets\\cf_redname\\images\\210.jpg";

    // Open the file
    HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Cannot open file. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Get file size
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize > MAX_SHARED_MEM_SIZE) {
        std::cerr << "File is too large or invalid. Max size: " << MAX_SHARED_MEM_SIZE << " bytes" << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    // Create file mapping object for file content
    HANDLE hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        fileSize,
        SHARED_MEM_NAME);

    if (hMapFile == NULL) {
        std::cerr << "Could not create file mapping object. Error: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    // Map view of file for file content
    LPVOID pSharedMem = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        fileSize);

    if (pSharedMem == NULL) {
        std::cerr << "Could not map view of file. Error: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Create file mapping object for file size
    HANDLE hMapSize = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(DWORD),
        SHARED_SIZE_NAME);

    if (hMapSize == NULL) {
        std::cerr << "Could not create file mapping object for size. Error: " << GetLastError() << std::endl;
        UnmapViewOfFile(pSharedMem);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Map view of file for file size
    LPDWORD pSharedSize = (LPDWORD)MapViewOfFile(
        hMapSize,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(DWORD));

    if (pSharedSize == NULL) {
        std::cerr << "Could not map view of file for size. Error: " << GetLastError() << std::endl;
        CloseHandle(hMapSize);
        UnmapViewOfFile(pSharedMem);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Read file directly into shared memory
    DWORD bytesRead;
    if (!ReadFile(hFile, pSharedMem, fileSize, &bytesRead, NULL)) {
        std::cerr << "Failed to read file. Error: " << GetLastError() << std::endl;
        UnmapViewOfFile(pSharedSize);
        CloseHandle(hMapSize);
        UnmapViewOfFile(pSharedMem);
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // Write file size to shared memory
    *pSharedSize = bytesRead;

    std::cout << "File written to shared memory. Size: " << bytesRead << " bytes" << std::endl;
    std::cout << "File size written to separate shared memory." << std::endl;
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();

    // Clean up
    UnmapViewOfFile(pSharedSize);
    CloseHandle(hMapSize);
    UnmapViewOfFile(pSharedMem);
    CloseHandle(hMapFile);
    CloseHandle(hFile);

    return 0;
}