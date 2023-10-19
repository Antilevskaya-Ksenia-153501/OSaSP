#include <iostream>
#include <vector>
#include<algorithm>
#include <windows.h>

struct ThreadData
{
    std::vector<int>* data;
    int start;
    int end;
};

DWORD WINAPI SortThread(LPVOID lpParam)
{
    ThreadData* threadData = reinterpret_cast<ThreadData*>(lpParam);
    std::vector<int>& data = *(threadData->data);
    int start = threadData->start;
    int end = threadData->end;

    std::sort(data.begin() + start, data.begin() + end);

    delete threadData;

    return 0;
}

std::vector<int> MergeSortedArrays(const std::vector<std::vector<int>>& sortedArrays)
{
    std::vector<int> mergedArray;
    std::size_t numArrays = sortedArrays.size();
    std::vector<std::size_t> indices(numArrays, 0);

    while (true)
    {
        int minValue = INT_MAX;
        std::size_t minIndex = numArrays;

        for (std::size_t i = 0; i < numArrays; ++i)
        {
            if (indices[i] < sortedArrays[i].size() && sortedArrays[i][indices[i]] < minValue)
            {
                minValue = sortedArrays[i][indices[i]];
                minIndex = i;
            }
        }

        if (minIndex == numArrays)
        {
            break;
        }

        mergedArray.push_back(minValue);
        indices[minIndex]++;
    }

    return mergedArray;
}

void MultiThreadedSort(std::vector<int>& data, int numThreads)
{
    int chunkSize = data.size() / numThreads;

    HANDLE* threads = new HANDLE[numThreads];

    for (int i = 0; i < numThreads; ++i)
    {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? data.size() : start + chunkSize;

        ThreadData* threadData = new ThreadData;
        threadData->data = &data;
        threadData->start = start;
        threadData->end = end;

        threads[i] = CreateThread(nullptr, 0, SortThread, threadData, 0, nullptr);
    }

    WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);

    std::vector<std::vector<int>> sortedArrays(numThreads);

    for (int i = 0; i < numThreads; ++i)
    {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? data.size() : start + chunkSize;

        sortedArrays[i].insert(sortedArrays[i].end(), data.begin() + start, data.begin() + end);
    }

    for (int i = 0; i < numThreads; ++i)
    {
        CloseHandle(threads[i]);
    }

    delete[] threads;

    std::vector<int> mergedArray = MergeSortedArrays(sortedArrays);

    std::copy(mergedArray.begin(), mergedArray.end(), data.begin());
}

int main()
{
    std::vector<int> data = { 8, 2, 9, 1, 7, 3, 6, 5, 4 };

    int numThreads = 4;

    MultiThreadedSort(data, numThreads);

    for (int num : data)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}