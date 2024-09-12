#ifndef NANOERROR_H_
#define NANOERROR_H_

enum ERRLevel {
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};

enum ERR { OK, NOT_INITIALIZED, NOT_FOUND, WRONG_ARGUMENT, UNDEFINED, INVALID };

#endif // NANOERROR_H_
