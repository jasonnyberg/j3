#include <gtest/gtest.h>
 
int testAll(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

int main(int argc, char *argv[]) {
    return testAll(argc, argv);
}
