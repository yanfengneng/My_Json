#include <gtest/gtest.h>
#include "../Source/include/json.h"
#include <memory>

using namespace std;
using namespace yfn;

int main(int argc, char* argv[])
{
    /* main 函数是必须的，作为测试程序的入口 */
    testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    return 0;
}