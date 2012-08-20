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
    level_(LOG_DEFAULT_LOGLEVEL),
    max_flush_num_(LOG_DEFAULT_FLUSH_NUM),
    not_flushed_num_(0) {
}

Logger::Logger(ENUM_LOG_LEVEL level, unsigned long flush_num):
    level_(level),
    max_flush_num_(flush_num),
    not_flushed_num_(0) {
}

// destructor
Logger::~Logger() {
}

bool Logger::config(const LogConfig& conf) {

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
            LOG_TO_STDERR("Log level out of range!");
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
        LOG_TO_STDERR("reset num_logs_to_flush from 0 to 1");
    }
    LOG_TO_STDERR("num_logs_to_flush: %lu", max_flush_num_);

    return true;
}

bool Logger::log(const std::string& msg, ENUM_LOG_LEVEL level) {
    if (level >= this->level_)
        return logImpl(msg);
    else
        return false;
}

ENUM_LOG_LEVEL Logger::getLevel() const {
    return level_;
}

void Logger::setLevel(ENUM_LOG_LEVEL level) {
    if(level >= LOG_LEVEL_MAX) {
        LOG_TO_STDERR("Invalid log level!");
    }
    else if(level_ != level) {
        level_ = level;
        LOG_TO_STDERR("Log level has been reset to: %s", get_log_level_txt(level_));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calss FileLogger
//

FileLogger::FileLogger():   // set the default value
    Logger(),
    file_path_(LOG_DEFAULT_FILE_PATH),
    file_base_name_(LOG_DEFAULT_FILE_BASENAME),
    file_suffix_(LOG_DEFAULT_FILE_SUFFIX),
    is_thread_safe_(true) {
}

FileLogger::FileLogger(const string& path,
        const string& base_name,
        const string& suffix,
        ENUM_LOG_LEVEL level,
        unsigned long flush_num,
        bool thread_safe):
    Logger(level, flush_num),
    file_path_(path),
    file_base_name_(base_name),
    file_suffix_(suffix),
    is_thread_safe_(thread_safe) {
}

FileLogger::~FileLogger() {
    close();
    LOG_TO_STDERR("~FileLogger()");
}

bool FileLogger::config(const LogConfig& conf) {
    if (!Logger::config(conf))
        return false;

    conf.getString(TEXT_LOG_FILE_PATH,      file_path_);
    conf.getString(TEXT_LOG_FILE_BASE_NAME, file_base_name_);
    conf.getString(TEXT_LOG_FILE_SUFFIX,    file_suffix_);

    return true;
}

bool FileLogger::open() {

    // create the directory first
    try {
        if( !boost::filesystem::exists(file_path_) ) {
            if( boost::filesystem::create_directories(file_path_) ) {
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

    close();

    // open file for write in append mode
    ios_base::openmode mode = fstream::out | fstream::app;
    file_.open( getFullFileName().c_str(), mode );

    if( !file_.good() ) {
        LOG_TO_STDERR("Failed to open log file <%s>", getFullFileName().c_str());
        return false;
    }
    else {
        LOG_TO_STDERR("Opened log file <%s> to APPEND to", getFullFileName().c_str());
        return true;
    }
}

bool FileLogger::close() {
    if( file_.is_open() ) {
        file_.flush();
        file_.close();
    }

    return true;
}

bool FileLogger::logImpl(const std::string& msg) {
    try{
        if( is_thread_safe_ ) {
            lock_guard<mutex> write_lock(mutex_);
            return writeLog(msg);
        }
        else {
            return writeLog(msg);
        }
    }
    catch (std::exception& ex) {
        LOG_TO_STDERR("Exception: %s", ex.what());
        return false;
    }

    return true;
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

bool FileLogger::writeLog(const std::string& msg) {
    if (!file_.is_open())
        return false;

    file_ << msg;

    Logger::not_flushed_num_++;
    if (Logger::not_flushed_num_ >= Logger::max_flush_num_) {
        file_.flush();
        Logger::not_flushed_num_ = 0;
    }

    return !file_.bad();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calss StdErrLogger
//

StdErrLogger::StdErrLogger():
    Logger() {
}

StdErrLogger::~StdErrLogger() {
    close();
}

bool StdErrLogger::config(const LogConfig& conf) {
    return Logger::config(conf);
}

bool StdErrLogger::open() {
    return true;
}

bool StdErrLogger::close() {
    return true;
}

bool StdErrLogger::logImpl(const std::string& msg) {
    fprintf(stderr, "%s", msg.c_str());
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calss RollingFileLogger
//

RollingFileLogger::RollingFileLogger():
    Logger(),
    file_path_(LOG_DEFAULT_FILE_PATH),
    file_base_name_(LOG_DEFAULT_FILE_BASENAME),
    file_suffix_(LOG_DEFAULT_FILE_SUFFIX) {
}

RollingFileLogger::~RollingFileLogger() {
    close();
    LOG_TO_STDERR("~RollingFileLogger()");
}

bool RollingFileLogger::config(const LogConfig& conf) {
    if (! Logger::config(conf))
        return false;

    conf.getString(TEXT_LOG_FILE_PATH,      file_path_);
    conf.getString(TEXT_LOG_FILE_BASE_NAME, file_base_name_);
    conf.getString(TEXT_LOG_FILE_SUFFIX,    file_suffix_);

    return true;
}

bool RollingFileLogger::open() {
    getCurrentDate( last_created_time_ );
    string file_name = getFileNameByDate(last_created_time_);

    file_logger_ = boost::shared_ptr<FileLogger>( new FileLogger(file_path_, file_name, file_suffix_, getLevel(), Logger::max_flush_num_, false) );
    if (NULL == file_logger_) {
        LOG_TO_STDERR("Creating FileLogger failed! In RollingFileLogger::open()");
        return false;
    }

    return file_logger_->open();
}

bool RollingFileLogger::close() {
    if (file_logger_) {
        file_logger_.reset();
    }

    return true;
}

bool RollingFileLogger::logImpl(const std::string& msg) {
    try {
        lock_guard<mutex> write_lock(mutex_);

        // create a new file when a day passed
        struct tm date_now;
        getCurrentDate(date_now);
        if (last_created_time_.tm_mday != date_now.tm_mday) {
            rotateFile();
        }

        if (file_logger_)
            return file_logger_->logImpl(msg);
        else
            return false;
    }
    catch (std::exception& ex) {
        LOG_TO_STDERR("Exception: %s", ex.what());
        return false;
    }

    return true;
}

void RollingFileLogger::rotateFile() {
    close();
    open();
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
