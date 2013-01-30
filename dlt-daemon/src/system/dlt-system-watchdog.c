/**
 * @licence app begin@
 * Copyright (C) 2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Lassi Marttala <lassi.lm.marttala@partner.bmw.de> BMW 2012
 *
 * \file dlt-system-logfile.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt-system-watchdog.c                                         **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Christian Muck <christian.muck@bmw.de>				          **
**              Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/
#if defined(DLT_SYSTEMD_WATCHDOG_ENABLE)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include "dlt.h"
#include "dlt-system.h"

#include "sd-daemon.h"


DLT_DECLARE_CONTEXT(watchdogContext)
DLT_IMPORT_CONTEXT(dltsystem)

extern DltSystemThreads threads;

typedef struct
{
    int timer_fd;
    unsigned long long wakeups_missed;
} PeriodicData;

void wait_period (PeriodicData *info)
{
    unsigned long long missed;

    read (info->timer_fd, &missed, sizeof (missed));

    if (missed > 0)
    {
        info->wakeups_missed += (missed - 1);
    }
}

int make_periodic(unsigned int period, PeriodicData *info)
{
    unsigned int ns;
    unsigned int sec;
    int fd;
    struct itimerspec itval;

    if (info==0)
    {
    	DLT_LOG(watchdogContext, DLT_LOG_ERROR,
    						DLT_STRING("Invalid function parameters used for function make_periodic.\n"));
        return -1;
    }

    /* Create the timer */
    fd = timerfd_create (CLOCK_MONOTONIC, 0);

    info->wakeups_missed = 0;
    info->timer_fd = fd;

    if (fd == -1)
    {
    	DLT_LOG(watchdogContext, DLT_LOG_ERROR,
    						DLT_STRING("Can't create timer filedescriptor.\n"));
        return -1;
    }

    /* Make the timer periodic */
    sec = period/1000000;
    ns = (period - (sec * 1000000)) * 1000;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;

    return timerfd_settime (fd, 0, &itval, NULL);
}


void watchdog_thread(void *v_conf)
{
	char str[512];
	char *watchdogUSec;
	unsigned int watchdogTimeoutSeconds;
	unsigned int notifiyPeriodNSec;
	PeriodicData info;

	DLT_REGISTER_CONTEXT(watchdogContext, "DOG","dlt system watchdog context.");

	sleep(1);

	DLT_LOG(watchdogContext, DLT_LOG_INFO,DLT_STRING("Watchdog thread started.\n"));

	if (v_conf==0)
	{
		DLT_LOG(watchdogContext, DLT_LOG_ERROR,
					DLT_STRING("Invalid function parameters used for function watchdog_thread.\n"));
		return;
	}


	watchdogUSec = getenv("WATCHDOG_USEC");

	if(watchdogUSec)
	{
		DLT_LOG(watchdogContext, DLT_LOG_DEBUG,DLT_STRING("watchdogusec: "),DLT_STRING(watchdogUSec));

		watchdogTimeoutSeconds = atoi(watchdogUSec);

		if( watchdogTimeoutSeconds > 0 ){

			// Calculate half of WATCHDOG_USEC in ns for timer tick
			notifiyPeriodNSec = watchdogTimeoutSeconds / 2 ;

			sprintf(str,"systemd watchdog timeout: %i nsec - timer will be initialized: %i nsec\n", watchdogTimeoutSeconds, notifiyPeriodNSec );
			DLT_LOG(watchdogContext, DLT_LOG_DEBUG,DLT_STRING(str));

			if (make_periodic (notifiyPeriodNSec, &info) < 0 )
			{
				DLT_LOG(watchdogContext, DLT_LOG_ERROR,DLT_STRING("Could not initialize systemd watchdog timer\n"));
				return;
			}

			while (1)
			{
				if(sd_notify(0, "WATCHDOG=1") < 0)
				{
					DLT_LOG(watchdogContext, DLT_LOG_ERROR,DLT_STRING("Could not reset systemd watchdog\n"));
				}

				DLT_LOG(watchdogContext, DLT_LOG_DEBUG,DLT_STRING("systemd watchdog waited periodic\n"));

				/* Wait for next period */
				wait_period(&info);
			}
		}
		else
		{
			sprintf(str,"systemd watchdog timeout incorrect: %i\n", watchdogTimeoutSeconds);
			DLT_LOG(watchdogContext, DLT_LOG_DEBUG,DLT_STRING(str));
		}
	}
	else
	{
		DLT_LOG(watchdogContext, DLT_LOG_ERROR,DLT_STRING("systemd watchdog timeout (WATCHDOG_USEC) is null\n"));
	}

}

void start_systemd_watchdog(DltSystemConfiguration *conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,DLT_STRING("Creating thread for systemd watchdog\n"));

	static pthread_attr_t t_attr;
	static pthread_t pt;

	if (pthread_create(&pt, &t_attr, (void *)watchdog_thread, conf) == 0)
	{
		threads.threads[threads.count++] = pt;
	}
	else
	{
		DLT_LOG(dltsystem, DLT_LOG_ERROR,DLT_STRING("Could not create thread for systemd watchdog\n"));
	}
}
#endif
