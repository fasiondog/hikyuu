// demo.cpp : 定义控制台应用程序的入口点。
//

#include <hikyuu/hikyuu.h>

using namespace hku;

int main(int argc, char* argv[])
{
    //配置文件的位置自行修改
    hikyuu_init("c:\\stock\\hikyuu_win.ini");

    StockManager& sm = StockManager::instance();

    Stock stk = sm.getStock("sh000001");
    std::cout << stk << std::endl;
    
    auto k = stk.getKData(KQuery(-10));
    std::cout << k << std::endl;

    for (auto i=0; i < k.size(); i++) {
        std::cout << k[i] << std::endl;
    }

	return 0;
}