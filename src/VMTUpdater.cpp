#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include "VTFLib.h"
#include "IL\il.h"
#include "VMTNodeUpdater.h"
#include "ImageSplitter.h"

using namespace std;

int main(int argc, char *argv[])
{
    printf("VMT Updater\n");

    string workingDir;

    if (argc <= 1)
    {
        printf("Input work folder:\n");
        cin >> workingDir;
    }
    else
    {
        workingDir = (argv[1]);
    }

    if (!filesystem::is_directory(workingDir))
    {
        printf("Given path '%s' is not a directory", workingDir.c_str());
        return -1;
    }
    printf("Working dir is '%s'\n", workingDir.c_str());

    //Init DevIL
    ilInit();
    //Init VTFLib
    vlInitialize();

    int32_t processedFilesCount = 0;
    filesystem::path workingDirPath(workingDir);
    workingDirPath = workingDirPath.lexically_normal();

    //splitter test
    /* 
    OutputInfo testOutputs[1];
    testOutputs[0].inputChannel = A;
    testOutputs[0].outputFormat = IL_PNG;
    testOutputs[0].outputChannels = 3;
    testOutputs[0].outputImageName = L"pistol.png";
    filesystem::path inputPath = workingDirPath;
    inputPath /= filesystem::path("c_pistol.vtf");
    
    auto pathAsStr = inputPath.string();
    ImageSplitter testSplitter(testOutputs, 1, pathAsStr.c_str(), 4);
    testSplitter.Split();
    return 0;
    */

    //Create updated dir
    auto updatedDir = workingDirPath;
    updatedDir /= "updated";
    filesystem::create_directory(updatedDir);

    for (auto const& dir_entry : filesystem::directory_iterator{workingDirPath}) 
    {
        auto extension = dir_entry.path().extension();
        if (extension.compare(".vmat") == 0 || extension.compare(".vmt") == 0)
        {
            printf("- Found VMT '%s'\n", dir_entry.path().string().c_str());

            UpdateVMTNodes(dir_entry);

            processedFilesCount++;
        }
    }

    printf("Processed #%i files", processedFilesCount);
    vlShutdown();
    ilShutDown();
}

