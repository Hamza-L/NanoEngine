#ifndef NANOERROR_H_
#define NANOERROR_H_

#include <iostream>

#ifndef NDEBUG
#   define ASSERT(Expr, Msg) \
    __Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define ASSERT(Expr, Msg) ;
#endif

inline void __Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}

enum class ERRLevel {
    FATAL,
    WARNING,
    INFO,
    DEBUG,
};

enum class ERR { OK, NOT_INITIALIZED, NOT_FOUND, WRONG_ARGUMENT, UNDEFINED, INVALID };

#endif // NANOERROR_H_
