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
**  SRC-MODULE: dlt-system-processes.c                                                  **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Lassi Marttala <lassi.lm.marttala@partner.bmw.de>             **
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


#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dlt-system.h"

// Modes of sending
#define SEND_MODE_OFF  0
#define SEND_MODE_ONCE 1
#define SEND_MODE_ON   2

extern DltSystemThreads threads;

DLT_IMPORT_CONTEXT(dltsystem)
DLT_DECLARE_CONTEXT(procContext)

void send_process(LogProcessOptions popts, int n)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-processes, send process info."));
	FILE * pFile;
	struct dirent *dp;
	char filename[256];
	char buffer[1024];
	int bytes;
	int found = 0;

	/* go through all process files in directory */
	DIR *dir = opendir("/proc");
	if(dir != NULL)
	{
		while ((dp=readdir(dir)) != NULL)
		{
			if(isdigit(dp->d_name[0]))
			{
				buffer[0] = 0;
				sprintf(filename, "/proc/%s/cmdline",dp->d_name);
				pFile = fopen(filename, "r");
				if(pFile != NULL)
				{
					bytes = fread(buffer, 1, sizeof(buffer)-1, pFile);
					fclose(pFile);
				}
				if((strcmp(popts.Name[n], "*")==0) ||
				   (strcmp(buffer, popts.Name[n])==0))
				{
					found = 1;
					sprintf(filename, "/proc/%s/%s", dp->d_name,popts.Filename[n]);
					pFile = fopen(filename, "r");
					if(pFile != NULL)
					{
						bytes = fread(buffer, 1, sizeof(buffer)-1, pFile);
						fclose(pFile);

						if(bytes>0)
						{
							buffer[bytes] = 0;
							DLT_LOG(procContext, DLT_LOG_INFO, DLT_INT(atoi(dp->d_name)), DLT_STRING(popts.Filename[n]), DLT_STRING(buffer));
						}
					}
					if(strcmp(popts.Name[n], "*") !=0)
						break;
				}
			}
		}
		closedir(dir);
	}
	else
	{
		DLT_LOG(dltsystem, DLT_LOG_ERROR,
				DLT_STRING("dlt-system-processes, failed to open /proc."));
	}

	if(!found) {
			DLT_LOG(procContext, DLT_LOG_INFO, DLT_STRING("Process"), DLT_STRING(popts.Name[n]),DLT_STRING("not running!"));
	}
}

void logprocess_thread(void *v_conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-processes, in thread."));

	DltSystemConfiguration *conf = (DltSystemConfiguration *) v_conf;
	DLT_REGISTER_CONTEXT(procContext, conf->LogProcesses.ContextId, "Log Processes");

	int process_delays[DLT_SYSTEM_LOG_PROCESSES_MAX];
	int i;
	for(i = 0;i < conf->LogProcesses.Count;i++)
		process_delays[i] = conf->LogProcesses.TimeDelay[i];

	while(!threads.shutdown)
	{
		sleep(1);
		for(i = 0;i < conf->LogProcesses.Count;i++)
		{
			if(conf->LogProcesses.Mode[i] == SEND_MODE_OFF)
				continue;

			if(process_delays[i] <= 0)
			{
				send_process(conf->LogProcesses, i);
				process_delays[i] = conf->LogProcesses.TimeDelay[i];
				if(conf->LogProcesses.Mode[i] == SEND_MODE_ONCE)
					conf->LogProcesses.Mode[i] = SEND_MODE_OFF;
			}
			else
			{
				process_delays[i]--;
			}
		}
	}
}

void start_logprocess(DltSystemConfiguration *conf)
{
	DLT_LOG(dltsystem, DLT_LOG_DEBUG,
			DLT_STRING("dlt-system-processes, starting process log."));
	static pthread_attr_t t_attr;
	static pthread_t pt;
	pthread_create(&pt, &t_attr, (void *)logprocess_thread, conf);
	threads.threads[threads.count++] = pt;
}
