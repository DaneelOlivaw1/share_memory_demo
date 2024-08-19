#include <windows.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#define SHARED_MEM_NAME L"MySharedMemory"
#define SHARED_SIZE_NAME L"MySharedSize"
#define MAX_SHARED_MEM_SIZE 1 * 1024 * 1024 // 1MB, adjust as needed

int main() {
    // Open the existing file mapping object for file content
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        SHARED_MEM_NAME);

    if (hMapFile == NULL) {
        std::cerr << "Could not open file mapping object. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Open the existing file mapping object for file size
    HANDLE hMapSize = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        SHARED_SIZE_NAME);

    if (hMapSize == NULL) {
        std::cerr << "Could not open file mapping object for size. Error: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
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
        CloseHandle(hMapFile);
        return 1;
    }

    // Read the file size
    DWORD fileSize = *pSharedSize;

    // Map view of file for file content
    LPVOID pSharedMem = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        fileSize);

    if (pSharedMem == NULL) {
        std::cerr << "Could not map view of file. Error: " << GetLastError() << std::endl;
        UnmapViewOfFile(pSharedSize);
        CloseHandle(hMapSize);
        CloseHandle(hMapFile);
        return 1;
    }

    // Read the file content from shared memory
    std::vector<char> buffer(fileSize);
    memcpy(buffer.data(), pSharedMem, fileSize);

    std::cout << "File read from shared memory. Size: " << fileSize << " bytes" << std::endl;

    // Decode the image using OpenCV
    cv::Mat img = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Failed to decode image" << std::endl;
    }
    else {
        // Display the image
        cv::namedWindow("Image from Shared Memory", cv::WINDOW_NORMAL);
        cv::imshow("Image from Shared Memory", img);
        cv::waitKey(0);
    }

    // Clean up
    UnmapViewOfFile(pSharedSize);
    CloseHandle(hMapSize);
    UnmapViewOfFile(pSharedMem);
    CloseHandle(hMapFile);

    return 0;
}