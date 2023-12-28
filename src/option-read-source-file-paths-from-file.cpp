#include "option-read-source-file-paths-from-file.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cstring>

auto const optRegex = std::regex(R"|(^--source-list="*([^"]+)"*$)|");

bool isOptSourceList(std::string_view const& arg){
    return std::regex_match(arg.data(),optRegex);
}

std::vector<std::string_view> getOptionsSources(std::vector<std::string_view >const& args){
    auto result = std::vector<std::string_view >{};
    std::copy_if(std::cbegin(args), std::cend(args), std::back_inserter(result), isOptSourceList);
    return result;
}

auto intoFileName(std::string_view const& arg) ->std::string{
    if (std::cmatch file; std::regex_search(arg.data(), file, optRegex)) {
        return file[1];
    }
    return {};
}

std::vector<std::string> getSourceListFiles(std::vector<std::string_view> const& vector1)
{
    auto result = std::vector<std::string>{};

    std::transform(std::cbegin(vector1), std::cend(vector1), std::back_inserter(result), intoFileName);
    std::sort(std::begin(result), std::end(result));

    return {std::begin(result), std::unique(std::begin(result), std::end(result))};
}

std::vector<std::unique_ptr<std::fstream>> openFiles(std::vector<std::string> const& filenames){
    auto result        = std::vector<std::unique_ptr<std::fstream>>{};
    auto const exists  = [](std::string const& fn){return std::filesystem::exists(fn);};
    auto const missing = std::find_if_not(std::cbegin(filenames), std::cend(filenames), exists);

    if(missing != std::cend(filenames)){
        std::cerr << "File does not exist: " << *missing << '\n';
    }
    else{
        auto intoStream = [](std::string const& fn){ return std::make_unique<std::fstream>(fn, std::ios_base::in);};
        std::transform(std::cbegin(filenames), std::cend(filenames), std::back_inserter(result), intoStream);
    }

    return result;
}

auto readLinesInto(std::vector<std::string>& dest){
    return [&dest](std::unique_ptr<std::fstream>const& fs){
        for(std::string line;std::getline(*fs, line);){
            dest.push_back(std::move(line));
        }
    };
}

std::vector<std::string> readSourceFilesFrom(std::vector<std::string> const& vector1)
{
    auto result  = std::vector<std::string>{};
    auto streams = openFiles(vector1);
    std::for_each(std::begin(streams), std::end(streams), readLinesInto(result));

    std::sort(std::begin(result), std::end(result));

    return {std::begin(result), std::unique(std::begin(result), std::end(result))};
}

std::vector<std::string_view> getRegularArguments(std::vector<std::string_view> const& vector1)
{
    auto result = std::vector<std::string_view>{};
    std::remove_copy_if(std::cbegin(vector1), std::cend(vector1), std::back_inserter(result), isOptSourceList);
    return result;
}

template<typename T>
auto intoArg (T const& t){
    auto pt = std::make_unique<char[]>(t.length()+1);
    std::strncpy(pt.get(), t.data(), t.length() );
    return pt;
}

FixedCmdLine addOptionToReadSourceFilesFromFile(int argc, const char **argv){
    auto const arguments       = std::vector<std::string_view >(argv, argv+argc);
    auto const sourceListOpts  = getOptionsSources(arguments);
    auto const sourceListFiles = getSourceListFiles(sourceListOpts);
    auto const sourceFiles     = readSourceFilesFrom(sourceListFiles);
    auto const regularArguments= getRegularArguments(arguments);
    auto const intoRawPointer  = [](std::unique_ptr<char[]>const& item){return item.get();};

    auto result                = FixedCmdLine{};

    std::transform(std::cbegin(regularArguments), std::cend(regularArguments), std::back_inserter(result.args), intoArg<std::string_view>);
    std::transform(std::cbegin(sourceFiles)     , std::cend(sourceFiles)     , std::back_inserter(result.args), intoArg<std::string>     );

    result.argv = std::make_unique<const char*[]>(result.args.size());
    result.argc = result.args.size();

    std::transform(std::begin(result.args), std::end(result.args), result.argv.get(), intoRawPointer);
    return result;
}
