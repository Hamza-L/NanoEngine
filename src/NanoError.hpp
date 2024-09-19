#ifndef NANOERROR_H_
#define NANOERROR_H_

enum class ERRLevel {
    FATAL,
    WARNING,
    INFO,
    DEBUG,
};

enum class ERR { OK, NOT_INITIALIZED, NOT_FOUND, WRONG_ARGUMENT, UNDEFINED, INVALID };

#endif // NANOERROR_H_
