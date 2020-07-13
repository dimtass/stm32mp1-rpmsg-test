#include "log.h"
#include "tty_tester.h"

void print_help(char * prog);


int main(int argc, char **argv)
{

    /* Check for tty in arguments */
    if (argc < 2) {
        print_help(argv[0]);
        return(1);
    }

    FILELog::ReportingLevel() = linfo;
    L_(linfo) << "Application started";

    TTY_Tester tester(argv[1], 4000000);

    L_(linfo) << "---- Creating tests ----";
    tester.add_test(512, 1);
    tester.add_test(512, 2);
    tester.add_test(512, 4);
    tester.add_test(512, 8);
    tester.add_test(1024, 1);
    tester.add_test(1024, 2);
    tester.add_test(1024, 4);
    tester.add_test(1024, 5);
    tester.add_test(2048, 1);
    tester.add_test(2048, 2);
    // tester.add_test(2048, 4);
    tester.add_test(4096, 1);
    // tester.add_test(4096, 2);
    // tester.add_test(4096, 4);
    // tester.add_test(8192, 1);
    // tester.add_test(8192, 2);
    // tester.add_test(8192, 4);
    // tester.add_test(16384, 1);
    // tester.add_test(16384, 2);
    // tester.add_test(32768, 1);

    L_(linfo) << "---- Starting tests ----";
    size_t n_tests = tester.get_num_of_tests();

    struct TTY_Tester::test_item item;
    for (size_t i; i<n_tests; i++) {
        tester.run_test(i, item);
    }

    return(0);
}


void print_help(char * prog)
{
    printf(" \
    This program benchmarks the /dev/ttyRPMSGx interface on the STM32MP1. \n \
    Usage: \n \
         %s [device] \n \
    Example: \n \
        %s /dev/ttyRPMSG0 \n \
    ", prog, prog);
}
