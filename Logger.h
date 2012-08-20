/*
 * Logger.h
 *
 *  Created on: Aug 15, 2012
 *      Author: xieliang
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "log.h"
#include "log_config.h"
#include "common.h"


class Logger {
public:
    // static:
    static boost::shared_ptr<Logger> createLoggerInterface(ENUM_LOG_TYPE type)
        throw (std::runtime_error);

    virtual ~Logger();

    virtual bool config(const LogConfig& conf);
    virtual bool open() = 0;
    virtual bool close() = 0;

    bool log(const std::string& msg, ENUM_LOG_LEVEL level);
    ENUM_LOG_LEVEL getLevel() const;
    void setLevel(ENUM_LOG_LEVEL level);

protected:
    // constructors
    Logger();
    Logger(ENUM_LOG_LEVEL level, unsigned long flush_num);

    virtual bool logImpl(const std::string& msg) = 0;

    ENUM_LOG_LEVEL level_;
    unsigned long max_flush_num_;
    unsigned long not_flushed_num_;
};


//
// class FileLogger
//
class FileLogger: public Logger {
public:

    friend class RollingFileLogger;

    FileLogger();
    FileLogger(const std::string& path,
            const std::string& base_name,
            const std::string& suffix,
            ENUM_LOG_LEVEL level,
            unsigned long flush_num,
            bool thread_safe );

    virtual ~FileLogger();

    virtual bool config(const LogConfig& conf);
    virtual bool open();
    virtual bool close();

protected:
    virtual bool logImpl(const std::string& msg);
    std::string getFullFileName() const;

private:
    FileLogger(const FileLogger& rhs);
    const FileLogger& operator=(const FileLogger& rhs);
    bool writeLog(const std::string& msg);

    std::string file_path_;
    std::string file_base_name_;
    std::string file_suffix_;

    std::fstream file_;

    const bool is_thread_safe_;
    boost::mutex mutex_;
};


//
// class StdErrLogger
//
class StdErrLogger: public Logger {
public:
    StdErrLogger();
    virtual ~StdErrLogger();

    virtual bool config(const LogConfig& conf);
    virtual bool open();
    virtual bool close();

protected:
    virtual bool logImpl(const std::string& msg);

private:
    StdErrLogger(const StdErrLogger& rhs);
    const StdErrLogger& operator=(const StdErrLogger& rhs);
};


//
// class RollingFileLogger
//
class RollingFileLogger : public Logger {
public:
    RollingFileLogger();
    virtual ~RollingFileLogger();

    virtual bool config(const LogConfig& conf);
    virtual bool open();
    virtual bool close();

protected:
    virtual bool logImpl(const std::string& msg);
    void rotateFile();

private:
    RollingFileLogger(const RollingFileLogger& rhs);
    const RollingFileLogger& operator=(const RollingFileLogger& rhs);

    void getCurrentDate(struct tm& date);
    std::string getFileNameByDate(const struct tm& date);

    std::string file_path_;
    std::string file_base_name_;
    std::string file_suffix_;

    // Rolling file logger uses a "file logger" to write log
    boost::shared_ptr<FileLogger> file_logger_;

    // the mutex
    boost::mutex mutex_;

    struct tm last_created_time_;
};

#endif /* LOGGER_H_ */
