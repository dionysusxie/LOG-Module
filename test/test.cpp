/*
 * test.cpp
 *
 *  Created on: Aug 22, 2012
 *      Author: xieliang
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <cassert>
#include <iostream>

#include "../output/log.h"


using namespace std;


extern void print_usage(const char*);

const int DEF_INTERVAL = 1; // in seconds
const int DEF_LOG_LEVEL = 1; // INFO
const int DEF_REPEAT_TIMES = 10;


int main(int argc, char **argv) {
    //
    // get command-line arguments
    //

    bool forceToDebug = false;  // 将 LOG 模块的级别强制设为 DEBUG？

    int log_interval = DEF_INTERVAL;
    int log_level = DEF_LOG_LEVEL;
    int log_repeat_times = DEF_REPEAT_TIMES;

    int next_option;
    const char* const short_options = "hdi:l:r:";
    const struct option long_options[] = {
        { "help", 0, NULL, 'h' },
        { "debug", 0, NULL, 'd' },
        { "interal", 0, NULL, 'i' },
        { "log_level", 0, NULL, 'l' },
        { "repeat_times", 0, NULL, 'r' },
        { NULL, 0, NULL, 'o' },
    };

    while (0 < (next_option = getopt_long(argc, argv, short_options, long_options, NULL))) {
        switch (next_option) {
            case 'd':
                forceToDebug = true;
                break;

            case 'i':
                log_interval = atoi(optarg);
                break;

            case 'l':
                log_level = atoi(optarg);
                break;

            case 'r':
                log_repeat_times = atoi(optarg);
                break;

            default:
                print_usage(argv[0]);
                exit(0);
                break;
        }
    }

    assert(log_interval > 0);
    assert(log_level >= 0 && log_level < LOG_LEVEL_MAX);
    assert(log_repeat_times >= 0);
    ////////////////////////////////////////////////////////////////////////////


    // initilize LOG system
    if (!LOG_SYS_INIT("log_config.conf")) {
        return 1;
    }

    // reset the log level to DEBUG
    if (forceToDebug) {
        LOG_SET_LEVEL(LOG_LEVEL_DEBUG);
    }

    LOG_INFO("enter main()");

    // seed random number generation with something reasonably unique
    srand(time(NULL) ^ getpid());


    //
    // the loop
    //

    int count = 0;
    while (true) {
        count++;

        switch (log_level) {
        case LOG_LEVEL_DEBUG:
            LOG_DEBUG("log %d", count);
            break;

        case LOG_LEVEL_INFO:
            LOG_INFO("log %d", count);
            break;

        case LOG_LEVEL_WARNING:
            LOG_WARNING("log %d", count);
            break;

        case LOG_LEVEL_ERROR:
            LOG_ERROR("log %d", count);
            break;

        default:
            break;
        }

        if (log_repeat_times != 0 && count >= log_repeat_times) {
            break;
        }

        // sleep
        sleep(log_interval);
    }


    LOG_INFO("app exit now");
    return 0;
}



void print_usage(const char* program_name) {
    cout << "Usage: " << program_name << " [-h] [-i interval(seconds)] [-l log_level] [-r repeat_times]" << endl;
    cout << "Default values: -i " << DEF_INTERVAL << " -l " << DEF_LOG_LEVEL << " -r " << DEF_REPEAT_TIMES << endl;
    cout << "Means: a log with " << get_log_level_txt(ENUM_LOG_LEVEL(DEF_LOG_LEVEL))
            << " level will be logged every " << DEF_INTERVAL << " seconds for "
            << DEF_REPEAT_TIMES << " times" << endl;
    cout << "Note: -r 0 means LOG forever!" << endl;
}
