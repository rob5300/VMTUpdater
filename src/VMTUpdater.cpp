#include <iostream>
#include <string>
#include <filesystem>
#include "VTFLib.h"
#include "IL\il.h"
#include "VMTNodeUpdater.h"

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

