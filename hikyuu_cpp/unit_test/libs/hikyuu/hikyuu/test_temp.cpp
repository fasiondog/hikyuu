#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_base
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/HistoryFinanceReader.h>
#include <hikyuu/utilities/thread/ThreadPool.h>

using namespace hku;

int Fibon(int n) {
	int num1 = 1;
	int num2 = 1;
	int tmp = 0;
	int i = 0;
	if (n < 3)
	{
		return 1;
	}
	else
	{
		for (i = 0; i <= n-3; i++)
		{
			tmp = num1 + num2;
			num1 = num2;
			num2 = tmp;
		}
		return tmp;
	}
}

BOOST_AUTO_TEST_CASE( test_temp ) {
    ThreadPool tg;
    for (int i = 0; i < 40; i++) {
        tg.submit([=]() {
            int x = Fibon(i+1);
            std::stringstream buf;
            buf << i+1 << ": " << x << std::endl;
            std::cout << buf.str();
        });
    }
    tg.join();
    std::cout << "*********************" << std::endl;
}
