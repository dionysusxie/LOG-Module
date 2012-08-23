/*
 * Logger.cpp
 *
 *  Created on: Aug 15, 2012
 *      Author: xieliang
 */

//#include <pthread.h>
#include <boost/filesystem.hpp>
#include "Logger.h"


using namespace std;
using namespace boost;


boost::shared_ptr<Logger> Logger::createLoggerInterface(ENUM_LOG_TYPE type) throw (runtime_error) {
    switch(type) {
    case TO_STDERR:
        return boost::shared_ptr<Logger>( new StdErrLogger() );
        break;

    case TO_FILE:
        return boost::shared_ptr<Logger>( new FileLogger() );
        break;

    case TO_ROLLING_FILE:
        return boost::shared_ptr<Logger>( new RollingFileLogger() );
        break;

    default:
        runtime_error ex("Wrong log type!");
        throw ex;
        break;
    }

    return boost::shared_ptr<Logger>();
}

// constructor
Logger::Logger():
    not_flushed_num_(0),
    status_(CREATED) {
    setDefaultConf();
}

Logger::Logger(ENUM_LOG_LEVEL level, unsigned long flush_num):
    level_(level),
    max_flush_num_(flush_num),
    not_flushed_num_(0),
    status_(CREATED) {
}

// destructor
Logger::~Logger() {
    // Don't call close() here!!!
    // close();

    // JUST free the resources allocated in this Logger class!
    // ...

    LOG_TO_STDERR("~Logger()");
}

void Logger::setDefaultConf() {
    level_ = LOG_DEFAULT_LOGLEVEL;
    max_flush_num_ = LOG_DEFAULT_FLUSH_NUM;
}

// get the config values of all items;
// the default value will be used if not given
bool Logger::config(const LogConfig& conf) {
    lock_guard<mutex> write_lock(mutex_);

    if (OPENED == status_) {
        Assert(false, "You can't config a logger when it's already opened!");
        return false;
    }

    setDefaultConf();


    //
    // get log level
    //

    unsigned long int num = 0;
    if(conf.getUnsigned(TEXT_LOG_LEVEL, num))
    {
        if(num < static_cast<unsigned long int>(LOG_LEVEL_MAX)) {
            level_ = static_cast<ENUM_LOG_LEVEL>(num);
        }
        else {
            Assert(false, "Log level out of range!");
            return false;
        }
    }
    LOG_TO_STDERR("Log level: %s", get_log_level_txt(level_));


    //
    // num_logs_to_flush
    //

    conf.getUnsigned(TEXT_LOG_FLUSH_NUM, max_flush_num_);
    if (max_flush_num_ < 1) {
        max_flush_num_ = 1;
        Assert(false, "num_logs_to_flush > 0");
    }
    LOG_TO_STDERR("num_logs_to_flush: %lu", max_flush_num_);


    return configImpl(conf);
}

bool Logger::open() {
    lock_guard<mutex> write_lock(mutex_);

    if (OPENED == status_) {
        Assert(false, "The logger is already opened!");
        return true;
    }

    if (!openImpl()) {
        return false;
    }

    status_ = OPENED;
    return true;
}

void Logger::close() {
    lock_guard<mutex> write_lock(mutex_);

    if (status_ != OPENED) {
        LOG_TO_STDERR("The logger is already closed!");
        return;
    }

    closeImpl();
    status_ = CLOSED;
}

bool Logger::log(const std::string& msg, ENUM_LOG_LEVEL level) {
    lock_guard<mutex> write_lock(mutex_);

    if (status_ != OPENED) {
        Assert(false, "The logger is NOT ready for logging !!!");
        return false;
    }

    if (level < level_) {
        return false;
    }

    if (!logImpl(msg, level)) {
        return false;
    }

    not_flushed_num_++;
    if (not_flushed_num_ >= max_flush_num_) {
        flush();
        not_flushed_num_ = 0;
    }

    return true;
}

ENUM_LOG_LEVEL Logger::getLevel() const {
    return level_;
}

void Logger::setLevel(ENUM_LOG_LEVEL new_level) {
    lock_guard<mutex> write_lock(mutex_);

    if (new_level >= LOG_LEVEL_MAX) {
        Assert(false, "Invalid log level!");
    }
    else if(level_ != new_level) {
        level_ = new_level;
        setLevelImpl(new_level);
        LOG_TO_STDERR("Log level has been reset to: %s", get_log_level_txt(level_));
    }
}

unsigned long Logger::getMaxFlushNum() const
{
    return max_flush_num_;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calss FileLogger
//

FileLogger::FileLogger() {
    setDefaultConf();
}

FileLogger::FileLogger(const string& path, const string& base_name, const string& suffix, ENUM_LOG_LEVEL level, unsigned long flush_num):
    Logger(level, flush_num),
    file_path_(path),
    file_base_name_(base_name),
    file_suffix_(suffix) {
}

FileLogger::~FileLogger() {
    close();
    LOG_TO_STDERR("~FileLogger()");
}

void FileLogger::setDefaultConf() {
    file_path_ = LOG_DEFAULT_FILE_PATH;
    file_base_name_ = LOG_DEFAULT_FILE_BASENAME;
    file_suffix_ = LOG_DEFAULT_FILE_SUFFIX;
}

bool FileLogger::configImpl(const LogConfig& conf) {
    setDefaultConf();
    conf.getString(TEXT_LOG_FILE_PATH,      file_path_);
    conf.getString(TEXT_LOG_FILE_BASE_NAME, file_base_name_);
    conf.getString(TEXT_LOG_FILE_SUFFIX,    file_suffix_);
    return true;
}

bool FileLogger::openImpl() {
    //
    // create the directory first
    //

    try {
        if (!boost::filesystem::exists(file_path_)) {
            if (boost::filesystem::create_directories(file_path_)) {
                LOG_TO_STDERR("Created log directory <%s>", file_path_.c_str());
            }
            else {
                LOG_TO_STDERR("Failed to created log directory <%s>", file_path_.c_str());
                return false;
            }
        }
    }
    catch (const std::exception& e) {
        LOG_TO_STDERR("Exception: %s", e.what());
        return false;
    }


    //
    // open file for write in append mode
    //

    ios_base::openmode mode = fstream::out | fstream::app;
    file_.open(getFullFileName().c_str(), mode);

    if (!file_.good()) {
        LOG_TO_STDERR("Failed to open log file <%s>", getFullFileName().c_str());
        return false;
    }
    else {
        LOG_TO_STDERR("Opened log file <%s> to APPEND to", getFullFileName().c_str());
        return true;
    }
}

void FileLogger::closeImpl() {
    if (file_.is_open()) {
        file_.close();
    }
}

bool FileLogger::logImpl(const std::string& msg, ENUM_LOG_LEVEL level) {
    if (!file_.is_open()) {
        return false;
    }
    file_ << msg;
    return !file_.bad();
}

void FileLogger::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}

std::string FileLogger::getFullFileName() const {
    string full_name;

    if( !file_path_.empty() ) {
        full_name += file_path_;

        if( file_path_[ file_path_.size() - 1 ] != '/' )
            full_name += "/";
    }

    full_name += file_base_name_;

    if( !file_suffix_.empty() ) {
        if( file_suffix_[0] == '.' )
            full_name += file_suffix_;
        else
            full_name = full_name + "." + file_suffix_;
    }

    return full_name;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class StdErrLogger
//

StdErrLogger::StdErrLogger():
    Logger() {
}

StdErrLogger::~StdErrLogger() {
    close();
}

bool StdErrLogger::configImpl(const LogConfig& conf) {
    return true;
}

bool StdErrLogger::openImpl() {
    return true;
}

void StdErrLogger::closeImpl() {
}

bool StdErrLogger::logImpl(const std::string& msg, ENUM_LOG_LEVEL level) {
    fprintf(stderr, "%s", msg.c_str());
    return true;
}

void StdErrLogger::flush() {
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calss RollingFileLogger
//

RollingFileLogger::RollingFileLogger() {
    setDefaultConf();
}

RollingFileLogger::~RollingFileLogger() {
    close();
    LOG_TO_STDERR("~RollingFileLogger()");
}

void RollingFileLogger::setDefaultConf() {
    file_path_ = LOG_DEFAULT_FILE_PATH;
    file_base_name_ = LOG_DEFAULT_FILE_BASENAME;
    file_suffix_ = LOG_DEFAULT_FILE_SUFFIX;
}

bool RollingFileLogger::configImpl(const LogConfig& conf) {
    setDefaultConf();
    conf.getString(TEXT_LOG_FILE_PATH,      file_path_);
    conf.getString(TEXT_LOG_FILE_BASE_NAME, file_base_name_);
    conf.getString(TEXT_LOG_FILE_SUFFIX,    file_suffix_);
    return true;
}

bool RollingFileLogger::openImpl() {
    getCurrentDate(last_created_time_);
    string file_name = getFileNameByDate(last_created_time_);

    file_logger_ = boost::shared_ptr<Logger>(new FileLogger(file_path_, file_name, file_suffix_, getLevel(), getMaxFlushNum()));
    if (NULL == file_logger_) {
        Assert(false, "Creating FileLogger failed! In RollingFileLogger::open()");
        return false;
    }

    return file_logger_->open();
}

void RollingFileLogger::closeImpl() {
    if (file_logger_) {
        file_logger_->close();
        file_logger_.reset();
    }
}

bool RollingFileLogger::logImpl(const std::string& msg, ENUM_LOG_LEVEL level) {
    try {
        // create a new file when a day passed
        struct tm date_now;
        getCurrentDate(date_now);
        if (last_created_time_.tm_mday != date_now.tm_mday) {
            rotateFile();
        }

        if (file_logger_)
            return file_logger_->log(msg, level);
        else
            return false;
    }
    catch (std::exception& ex) {
        LOG_TO_STDERR("Exception: %s", ex.what());
        return false;
    }

    return true;
}

void RollingFileLogger::flush() {
}

void RollingFileLogger::setLevelImpl(ENUM_LOG_LEVEL new_level) {
    if (file_logger_) {
        file_logger_->setLevel(new_level);
    }
}

void RollingFileLogger::rotateFile() {
    closeImpl();
    openImpl();
}

void RollingFileLogger::getCurrentDate(struct tm& date) {
    time_t raw_time = time(NULL);
    localtime_r(&raw_time, &date);
}

string RollingFileLogger::getFileNameByDate(const struct tm& date) {
    ostringstream filename;
    filename << file_base_name_ << '-' << date.tm_year + 1900 << '-'
            << setw(2) << setfill('0') << date.tm_mon + 1 << '-'
            << setw(2) << setfill('0') << date.tm_mday;
    return filename.str();
}
