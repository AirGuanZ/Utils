#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char* argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
                  | _CRTDBG_LEAK_CHECK_DF);
#endif

    return Catch::Session().run(argc, argv);
}
