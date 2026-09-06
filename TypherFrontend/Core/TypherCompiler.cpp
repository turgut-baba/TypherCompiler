// TypherCompiler.cpp : Defines the entry point for the application.
//

#include "TypherCompiler.h"
#include "Parser.h"
#include "MLIRGen.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include "Log/Diagnostics.h"
//#include "Checker.h"
#include "Comments.h"


std::string read_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path))
        throw std::runtime_error("File does not exist: " + path.string());

    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path.string());

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

bool check_file_end(std::string filename, const char* suffix)
{
    const size_t name_size = filename.size();
    if (name_size < 3) {
        return false;
    }
    char result_suffix[4];
    const size_t suffix_index = name_size - 3;
    for (int i = suffix_index; i < name_size; i++) {
        result_suffix[i - suffix_index] = filename[i];
    }
    result_suffix[3] = '\0';
    return(strcmp(result_suffix, suffix) == 0);
}

int main(int argc, char** argv)
{
    
    MemoryAllocator allocator {};
    
	DiagnosticEngine diags;
    
    allocator.dtorAlloc = std::make_shared<DtorMemAllocator>();
	allocator.slabAlloc = std::make_shared<SlabAllocator>(1024 * 1024);
	allocator.bumpAlloc = std::make_shared<BumpPtrAllocator>(1024 * 1024);
    
    if (argc < 2) 
    {
        std::cout << "Please enter a file name." << std::endl;
        return 1;
    }

    char* entry_file = argv[1];
    if (!check_file_end(entry_file, ".ty")) {
        std::cout << "Please enter a '.ty' file." << std::endl;
        return 1;
    }
    
    try {
        std::string file_buffer = read_file(entry_file);
        PreProcessor::strip_comments_in_place(file_buffer);
        Parser::Parser parse = Parser::Parser(file_buffer, diags, &allocator);
        parse.parse();

        parse.PrintAST(); // DEBUG
        auto ast = parse.AST();

        
        //checker::Checker checker = checker::Checker(diags, &allocator);
        //checker.StartChecker(ast);
        
        MLIR::Generator mlir = MLIR::Generator(&allocator);
        mlir.BuildModule(ast);

        diags.report<DiagLevel::Success>()
             << "Successfully compiled '" << entry_file << "' in 42ms.";

    }
    catch (std::exception& e) {
        diags.report<DiagLevel::Fatal>({})
             << "Failed to compile '" << e.what();
    }

    

	return 0;
}

