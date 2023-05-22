#pragma once

#include <stdio.h>

namespace utils
{
namespace misc
{

    void
    printCallStack(
        FILE *out,
        unsigned int depth);

}
}