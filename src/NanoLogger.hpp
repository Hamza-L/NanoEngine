#ifndef NANOLOGGER_H_
#define NANOLOGGER_H_

#include "NanoError.hpp"

#include "glm/glm.hpp"

#include <cstdarg>
#include <iostream>
#include <sstream>
#include <vector>

#define LOG_SCOPED(level, ...)                                                                                                                       \
    Logger::get_instance()->LogVerbose((level), __FILE__, __LINE__, __func__, __VA_ARGS__);                                                                 \
    ScopeTracker tracker { level }

#define LOG_MSG_VERBOSE(level, ...) Logger::get_instance()->LogVerbose((level), __FILE__, __LINE__, __func__, __VA_ARGS__);
#define LOG_MSG(level, ...) Logger::get_instance()->Log((level), __VA_ARGS__);

#define LOG_VAR(level, variable) Logger::get_instance()->LogVar((level), #variable ": %s", Logger::ToString(variable).c_str())

#define VK_CHECK(vkFunc)                                                                                                                             \
    if (vkFunc != VK_SUCCESS) {                                                                                                                      \
        LOG_MSG(ERRLevel::ERROR, "");                                                                                                              \
    }

class ScopeTracker {
  private:
  public:
    ScopeTracker(ERRLevel severity) : m_severity(severity) { indentTracker++; }
    ~ScopeTracker();

    static int indentTracker;
    ERRLevel m_severity;
};

class Logger { // singleton
  private:
    Logger(const int num) { uniqueID = num; }

    static Logger *oneAndOnlyInstance;
    static int uniqueID;
    static ERRLevel severity;
    static std::vector<std::string> logMessages;

  public:
    static Logger *get_instance(const int num = 0) {
        if (oneAndOnlyInstance == nullptr) {
            std::cout << "creating a new logger instance" << std::endl;
            oneAndOnlyInstance = new Logger(num);
        }
        // std::cout<< "returning instance with unique ID: " << uniqueID << std::endl;
        return oneAndOnlyInstance;
    }

    static void setSeverity(ERRLevel severityLevel) { severity = severityLevel; }
    static ERRLevel getSeverity() { return severity; }

    static std::string ToString(int variable) { return std::to_string(variable); }
    static std::string ToString(double variable) { return std::to_string(variable); }
    static std::string ToString(float variable) { return std::to_string(variable); }
    static std::string ToString(std::string variable) { return variable; }
    static std::string ToString(glm::vec4 variable) {
        std::stringstream ss;
        ss << "{ " << variable.x << " , " << variable.y << " , " << variable.z << " , " << variable.w << " }";
        return ss.str();
    }
    static std::string ToString(glm::mat4 variable) {
        std::stringstream ss;
        std::stringstream indent;
        for (int i = 0; i < ScopeTracker::indentTracker; i++) {
            indent << "\t";
        }

        ss << "\n";
        ss << indent.str() << "{ " << variable[0][0] << " , " << variable[1][0] << " , " << variable[2][0] << " , " << variable[3][0] << " \n";
        ss << indent.str() << "  " << variable[0][1] << " , " << variable[1][1] << " , " << variable[2][1] << " , " << variable[3][1] << " \n";
        ss << indent.str() << "  " << variable[0][2] << " , " << variable[1][2] << " , " << variable[2][2] << " , " << variable[3][2] << " \n";
        ss << indent.str() << "  " << variable[0][3] << " , " << variable[1][3] << " , " << variable[2][3] << " , " << variable[3][3] << " }";
        return ss.str();
    }

    static void Log(ERRLevel messageSeverity, const char *fmt, ...);
    static void LogVerbose(ERRLevel messageSeverity, const char *fileName, const int lineNumber, const char *func, const char *fmt, ...);
    static void LogVar(ERRLevel messageSeverity, const char *fmt, ...);

    void operator=(const Logger &) = delete;
    Logger(Logger &other) = delete;

    void PrintUniqueID() { std::cout << "Current instance's unique ID: " << this->uniqueID << std::endl; }
};

#endif // NANOLOGGER_H_
