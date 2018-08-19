// demo.cpp : 定义控制台应用程序的入口点。
//

#include <hikyuu_utils/iniparser/IniParser.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace hku;

int main(int argc, char* argv[])
{
    IniParser ini_parser;
    std::string test_filename("test_iniparser_read.ini");
    std::ofstream testini(test_filename, std::ofstream::trunc);
    //testini.open(test_filename, std::ofstream::trunc);
    testini << "[section]\n" << "= value";
    testini.close();
    
    std::cout << "test" << std::endl;
    
    try {
        ini_parser.read(test_filename);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

	return 0;
}