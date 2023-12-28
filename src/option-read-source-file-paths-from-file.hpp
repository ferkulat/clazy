//
// Created by marcel on 12/27/23.
//

#ifndef CLAZY_OPTION_READ_SOURCE_FILE_PATHS_FROM_FILE_HPP
#define CLAZY_OPTION_READ_SOURCE_FILE_PATHS_FROM_FILE_HPP

#include <vector>
#include <memory>

struct FixedCmdLine{
    std::vector<std::unique_ptr<char[]>> args;
    std::unique_ptr<const char*[]>       argv;
    int                                  argc;
};

FixedCmdLine addOptionToReadSourceFilesFromFile(int argc, const char **argv);
#endif // CLAZY_OPTION_READ_SOURCE_FILE_PATHS_FROM_FILE_HPP
