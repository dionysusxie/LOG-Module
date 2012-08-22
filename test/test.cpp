/*
 * test.cpp
 *
 *  Created on: Aug 22, 2012
 *      Author: xieliang
 */

#include "../output/log.h"


int main(int argc, char **argv) {
    if (!LOG_SYS_INIT("log_config.conf")) {
        return 1;
    }

    LOG_INFO("enter main()");

    LOG_INFO("app exit now");
    return 0;
}
