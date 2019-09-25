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

int64 Fibon(int64 n) {
	int64 num1 = 1;
	int64 num2 = 1;
	int64 tmp = 0;
	int64 i = 0;
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

double test_func() {
	double x = 0;
	for (int i = 0; i < 20000; i++) {
		x += i;
	}
	return x;
}

BOOST_AUTO_TEST_CASE( test_temp ) {
    {
		SPEND_TIME(test_temp);
		ThreadPool tg;
		for (int64 i = 0; i < 10; i++) {
			tg.submit([=]() {
				std::cout << fmt::format("{}---------------------\n", i);
			});
		}
		tg.join();
	}
}
