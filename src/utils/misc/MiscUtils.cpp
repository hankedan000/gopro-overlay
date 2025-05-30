#include "GoProOverlay/utils/misc/MiscUtils.h"

#include <cstdlib>
#include <cxxabi.h>
#include <execinfo.h>
#include <vector>

namespace utils
{
namespace misc
{
    void
    printCallStack(
        FILE *out,
        unsigned int depth)
    {
        // storage array for stack trace address data
        std::vector<void *> addrlist(depth+1);

        // retrieve current stack addresses
        int addrlen = backtrace(addrlist.data(), static_cast<int>(addrlist.size()));

        if (addrlen == 0)
        {
            fprintf(out, "  <empty, possibly corrupt>\n");
            return;
        }

        // resolve addresses into strings containing "filename(function+address)",
        // this array must be free()-ed
        char** symbollist = backtrace_symbols(addrlist.data(), addrlen);

        // allocate string which will be filled with the demangled function name
        size_t funcnamesize = 256;
        auto funcname = (char*)malloc(funcnamesize);

        // iterate over the returned symbol lines. skip the first, it is the
        // address of this function.
        for (int i = 1; i < addrlen; i++)
        {
            char *begin_name = nullptr;
            char *begin_offset = nullptr;
            char *end_offset = nullptr;

            // find parentheses and +address offset surrounding the mangled name:
            // ./module(function+0x15c) [0x8048a6d]
            for (char *p = symbollist[i]; *p; ++p)
            {
                if (*p == '(')
                {
                    begin_name = p;
                }
                else if (*p == '+')
                {
                    begin_offset = p;
                }
                else if (*p == ')' && begin_offset)
                {
                    end_offset = p;
                    break;
                }
            }

            if (begin_name && begin_offset && end_offset
                && begin_name < begin_offset)
            {
                *begin_name++ = '\0';
                *begin_offset++ = '\0';
                *end_offset = '\0';

                // mangled name is now in [begin_name, begin_offset) and caller
                // offset in [begin_offset, end_offset). now apply __cxa_demangle():

                int status;
                char* ret = abi::__cxa_demangle(begin_name,
                                funcname, &funcnamesize, &status);
                if (status == 0)
                {
                    // use possibly realloc()-ed string
                    funcname = ret;
                    fprintf(out, "  #%-2d %s : %s+%s\n",
                        i, symbollist[i], funcname, begin_offset);
                }
                else
                {
                    // demangling failed. Output function name as a C function with
                    // no arguments.
                    fprintf(out, "  #%-2d %s : %s()+%s\n",
                        i, symbollist[i], begin_name, begin_offset);
                }
            }
            else
            {
                // couldn't parse the line? print the whole line.
                fprintf(out, "  %s\n", symbollist[i]);
            }
        }

        free(funcname);
        free(symbollist);
    }
}
}