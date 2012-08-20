/*
 * LogSys.h
 *
 *  Created on: Aug 15, 2012
 *      Author: xieliang
 */

#ifndef LOGSYS_H_
#define LOGSYS_H_

#include "log_config.h"
#include "Logger.h"


class LogSys {
public:
    // static methods:
    static bool initialize(const std::string& config_file);
    static boost::shared_ptr<LogSys> getInstance();

    virtual ~LogSys();

    void log(const std::string& msg, ENUM_LOG_LEVEL level);
    void setLevel(ENUM_LOG_LEVEL level);

private:
    // static:
    static boost::shared_ptr<LogSys> s_pLogSys;

    // constructor
    LogSys(const std::string& config_file) throw (std::runtime_error);

    // non-static:
    LogConfig config_;
    boost::shared_ptr<Logger> logger_;
};

#endif /* LOGSYS_H_ */
