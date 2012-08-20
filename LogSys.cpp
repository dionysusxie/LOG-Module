/*
 * LogSys.cpp
 *
 *  Created on: Aug 15, 2012
 *      Author: xieliang
 */

#include "LogSys.h"
#include "log.h"
#include "common.h"


using namespace std;
using namespace boost;


boost::shared_ptr<LogSys> LogSys::s_pLogSys;

bool LogSys::initialize(const string& config_file) {
    try{
        if (NULL == s_pLogSys) {
            s_pLogSys = boost::shared_ptr<LogSys>(new LogSys(config_file));

            if (NULL == s_pLogSys) {
                LOG_TO_STDERR("Failed to creating the log system!");
                return false;
            }
        }
    }
    catch( std::exception& ex ) {
        LOG_TO_STDERR("Exception: %s", ex.what());
        return false;
    }

    return true;
}

boost::shared_ptr<LogSys> LogSys::getInstance() {
    return s_pLogSys;
}

LogSys::LogSys(const string& config_file) throw (runtime_error) {
    if(config_file.empty()) {
        LOG_TO_STDERR("No log config file specified. The default setting will be used!");
    }
    else {
        LOG_TO_STDERR("Opening file <%s> to get log config...", config_file.c_str());

        if(!config_.parseConfig(config_file)) {
            runtime_error ex("Errors happened when read the log config file!");
            throw ex;
        }
    }

    unsigned long dest = static_cast<unsigned long>(LOG_DEFAULT_LOG_DEST);
    config_.getUnsigned(TEXT_LOG_DESTINATION, dest);

    logger_ = Logger::createLoggerInterface( ENUM_LOG_TYPE(dest) );
    if(NULL == logger_) {
        runtime_error ex("Failed to create the logger interface!");
        throw ex;
    }

    if (!logger_->config(config_)) {
        runtime_error ex("Failed to config the log system");
        throw ex;
    }

    if (!logger_->open()) {
        runtime_error ex("Failed to open the log system");
        throw ex;
    }

    LOG_TO_STDERR("Log system initialized OK!");
}

LogSys::~LogSys() {
    if(logger_) {
        logger_.reset();
    }
}

void LogSys::log(const string& msg, ENUM_LOG_LEVEL level) {
    if(logger_)
        logger_->log(msg, level);
}

void LogSys::setLevel(ENUM_LOG_LEVEL level) {
    if(logger_)
        logger_->setLevel(level);
}
