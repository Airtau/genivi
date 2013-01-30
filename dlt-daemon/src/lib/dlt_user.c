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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_user.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user.c                                                    **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
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

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1670 $
 * $LastChangedDate: 2011-04-08 15:12:06 +0200 (Fr, 08. Apr 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */
#include <stdlib.h> /* for getenv(), free(), atexit() */
#include <string.h> /* for strcmp(), strncmp(), strlen(), memset(), memcpy() */
#include <signal.h> /* for signal(), SIGPIPE, SIG_IGN */

#if !defined (__WIN32__)
#include <syslog.h> /* for LOG_... */
#include <semaphore.h>
#include <pthread.h>    /* POSIX Threads */
#endif

#include <sys/time.h>
#include <math.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/uio.h> /* writev() */

#include <limits.h>

#include "dlt_user.h"
#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"
#include "dlt_user_cfg.h"

static DltUser dlt_user;
static int dlt_user_initialised = 0;

static char str[DLT_USER_BUFFER_LENGTH];

static sem_t dlt_mutex;
static pthread_t dlt_receiverthread_handle;
static pthread_attr_t dlt_receiverthread_attr;

/* Segmented Network Trace */
#define DLT_MAX_TRACE_SEGMENT_SIZE 1024
#define DLT_MESSAGE_QUEUE_NAME "/dlt_message_queue"
#define DLT_DELAYED_RESEND_INDICATOR_PATTERN 0xFFFF

/* Mutex to wait on while message queue is not initialized */
pthread_mutex_t mq_mutex;
pthread_cond_t  mq_init_condition;

/* Structure to pass data to segmented thread */
typedef struct {
	DltContext 			*handle;
	uint16_t			id;
	DltNetworkTraceType	nw_trace_type;
	uint16_t 			header_len;
	void 				*header;
	uint16_t 			payload_len;
	void 				*payload;
} s_segmented_data;

/* Function prototypes for internally used functions */
static void dlt_user_receiverthread_function(void *ptr);
static void dlt_user_atexit_handler(void);
static int dlt_user_log_init(DltContext *handle, DltContextData *log);
static DltReturnValue dlt_user_log_send_log(DltContextData *log, int mtype);
static int dlt_user_log_send_register_application(void);
static int dlt_user_log_send_unregister_application(void);
static int dlt_user_log_send_register_context(DltContextData *log);
static int dlt_user_log_send_unregister_context(DltContextData *log);
static int dlt_send_app_ll_ts_limit(const char *appid, DltLogLevelType loglevel, DltTraceStatusType tracestatus);
static int dlt_user_log_send_log_mode(DltUserLogMode mode);
static int dlt_user_print_msg(DltMessage *msg, DltContextData *log);
static int dlt_user_log_check_user_message(void);
static void dlt_user_log_reattach_to_daemon(void);
static int dlt_user_log_send_overflow(void);
static void dlt_user_trace_network_segmented_thread(void *unused);
static int dlt_user_queue_resend(void);

int dlt_user_check_library_version(const char *user_major_version,const char *user_minor_version){

	char str[200];
	char lib_major_version[DLT_USER_MAX_LIB_VERSION_LENGTH];
	char lib_minor_version[DLT_USER_MAX_LIB_VERSION_LENGTH];

	dlt_get_major_version( lib_major_version);
	dlt_get_minor_version( lib_minor_version);

	if( (strcmp(lib_major_version,user_major_version)!=0) || (strcmp(lib_minor_version,user_minor_version)!=0))
	{
		sprintf(str,"DLT Library version check failed! Installed DLT library version is %s.%s - Application using DLT library version %s.%s\n",lib_major_version,lib_minor_version,user_major_version,user_minor_version);
		dlt_log(LOG_WARNING, str);
		return -1;
	}
	return 0;
}

int dlt_init(void)
{
    char filename[DLT_USER_MAX_FILENAME_LENGTH];
    int ret;

    dlt_user_initialised = 1;

    /* Initialize common part of dlt_init()/dlt_init_file() */
    if (dlt_init_common()==-1)
    {
        dlt_user_initialised = 0;
        return -1;
    }

    dlt_user.dlt_is_file = 0;
    dlt_user.overflow = 0;
#ifdef DLT_SHM_ENABLE
	memset(&(dlt_user.dlt_shm),0,sizeof(DltShm));
#endif

    /* create and open DLT user FIFO */
    sprintf(filename,"%s/dlt%d",DLT_USER_DIR,getpid());
     
    /* Try to delete existing pipe, ignore result of unlink */
    unlink(filename);
    
    ret=mkfifo(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH  | S_IWOTH );
    if (ret==-1)
    {
        sprintf(str,"Loging disabled, FIFO user %s cannot be created!\n",filename);
        dlt_log(LOG_WARNING, str);
        /* return 0; */ /* removed to prevent error, when FIFO already exists */
    }

    dlt_user.dlt_user_handle = open(filename, O_RDWR | O_CLOEXEC);
    if (dlt_user.dlt_user_handle == DLT_FD_INIT)
    {
        sprintf(str,"Loging disabled, FIFO user %s cannot be opened!\n",filename);
        dlt_log(LOG_WARNING, str);
        unlink(filename);
        return 0;
    }

    /* open DLT output FIFO */
    dlt_user.dlt_log_handle = open(DLT_USER_FIFO, O_WRONLY | O_NONBLOCK | O_CLOEXEC );
    if (dlt_user.dlt_log_handle==-1)
    {
        sprintf(str,"Loging disabled, FIFO %s cannot be opened with open()!\n",DLT_USER_FIFO);
        dlt_log(LOG_WARNING, str);
        //return 0;
    }
	else
	{
#ifdef DLT_SHM_ENABLE
		/* init shared memory */
		if (dlt_shm_init_client(&(dlt_user.dlt_shm),DLT_SHM_KEY) < 0)
		{
			sprintf(str,"Loging disabled, Shared memory %d cannot be created!\n",DLT_SHM_KEY);
			dlt_log(LOG_WARNING, str);
			//return 0; 
		}   
#endif
	}
		

    if (dlt_receiver_init(&(dlt_user.receiver),dlt_user.dlt_user_handle, DLT_USER_RCVBUF_MAX_SIZE)==-1)
	{
        dlt_user_initialised = 0;
        return -1;
    }

    /* Start receiver thread */
    if (pthread_create(&(dlt_receiverthread_handle),
                       0,
                       (void *) &dlt_user_receiverthread_function,
                       0)!=0)
	{
		if (pthread_attr_destroy(&dlt_receiverthread_attr)!=0)
		{
			dlt_log(LOG_WARNING, "Can't destroy thread attributes!\n");
		}

		dlt_log(LOG_CRIT, "Can't create receiver thread!\n");
		dlt_user_initialised = 0;
        return -1;
	}

    if (pthread_attr_destroy(&dlt_receiverthread_attr)!=0)
	{
		dlt_log(LOG_WARNING, "Can't destroy thread attributes!\n");
	}

    /* These will be lazy initialized only when needed */
    dlt_user.dlt_segmented_queue_read_handle = -1;
    dlt_user.dlt_segmented_queue_write_handle = -1;

    /* Wait mutext for segmented thread */
    pthread_mutex_init(&mq_mutex, NULL);
    pthread_cond_init(&mq_init_condition, NULL);

    /* Start the segmented thread */
	if(pthread_create(&(dlt_user.dlt_segmented_nwt_handle), NULL,
	   (void *)dlt_user_trace_network_segmented_thread, NULL))
	{
    	dlt_log(LOG_CRIT, "Can't start segmented thread!\n");
    	return -1;
	}

	return 0;
}

int dlt_init_file(const char *name)
{
    dlt_user_initialised = 1;

    /* Initialize common part of dlt_init()/dlt_init_file() */
    if (dlt_init_common()==-1)
    {
        dlt_user_initialised = 0;
        return -1;
    }

    dlt_user.dlt_is_file = 1;

    /* open DLT output file */
    dlt_user.dlt_log_handle = open(name,O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); /* mode: wb */
    if (dlt_user.dlt_log_handle == -1)
    {
        sprintf(str,"Log file %s cannot be opened!\n",name);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    return 0;
}

int dlt_init_message_queue(void)
{
	pthread_mutex_lock(&mq_mutex);
	if(dlt_user.dlt_segmented_queue_read_handle >= 0 &&
	   dlt_user.dlt_segmented_queue_write_handle >= 0)
	{
		// Already intialized
		pthread_mutex_unlock(&mq_mutex);
		return 0;
	}

    /* Generate per process name for queue */
    char queue_name[NAME_MAX];
    sprintf(queue_name, "%s.%d", DLT_MESSAGE_QUEUE_NAME, getpid());

    /* Maximum queue size is 10, limit to size of pointers */
    struct mq_attr mqatr;
    mqatr.mq_flags		= 0;
    mqatr.mq_maxmsg		= 10;
    mqatr.mq_msgsize	= sizeof(s_segmented_data *);
    mqatr.mq_curmsgs	= 0;

    /**
     * Create the message queue. It must be newly created
     * if old one was left by a crashing process.
     * */
    dlt_user.dlt_segmented_queue_read_handle = mq_open(queue_name, O_CREAT| O_RDONLY | O_EXCL,
    		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, &mqatr);
    if(dlt_user.dlt_segmented_queue_read_handle < 0)
    {
    	if(errno == EEXIST)
    	{
    		dlt_log(LOG_WARNING, "Old message queue exists, trying to delete.\n");
    		if(mq_unlink(queue_name) < 0)
    		{
    			dlt_log(LOG_CRIT, "Could not delete existing message queue!\n");
    			dlt_log(LOG_CRIT, strerror(errno));
    		}
    		else // Retry
    		{
    			dlt_user.dlt_segmented_queue_read_handle = mq_open(queue_name, O_CREAT| O_RDONLY | O_EXCL,
    			    		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, &mqatr);
    		}
    	}
    	if(dlt_user.dlt_segmented_queue_read_handle < 0)
    	{
    		dlt_log(LOG_CRIT, "Can't create message queue read handle!\n");
    		dlt_log(LOG_CRIT, strerror(errno));
    		pthread_mutex_unlock(&mq_mutex);
    		return -1;
    	}
    }

    dlt_user.dlt_segmented_queue_write_handle = mq_open(queue_name, O_WRONLY|O_NONBLOCK);
    if(dlt_user.dlt_segmented_queue_write_handle < 0)
    {
    	dlt_log(LOG_CRIT, "Can't open message queue write handle!\n");
    	dlt_log(LOG_CRIT, strerror(errno));
    	pthread_mutex_unlock(&mq_mutex);
    	return -1;
    }

    pthread_cond_signal(&mq_init_condition);
    pthread_mutex_unlock(&mq_mutex);
    return 0;
}

int dlt_init_common(void)
{
    char *env_local_print;

    /* Binary semaphore for threads */
    if (sem_init(&dlt_mutex, 0, 1)==-1)
    {
        dlt_user_initialised = 0;
        return -1;
    }

	/* set to unknown state of connected client */
	dlt_user.log_state = -1;

    dlt_user.dlt_log_handle=-1;
    dlt_user.dlt_user_handle=DLT_FD_INIT;

    dlt_set_id(dlt_user.ecuID,DLT_USER_DEFAULT_ECU_ID);
    dlt_set_id(dlt_user.appID,"");

    dlt_user.application_description = 0;

    /* Verbose mode is enabled by default */
    dlt_user.verbose_mode = 1;

    /* Local print is disabled by default */
    dlt_user.enable_local_print = 0;

    dlt_user.local_print_mode = DLT_PM_UNSET;

    env_local_print = getenv(DLT_USER_ENV_LOCAL_PRINT_MODE);
    if (env_local_print)
    {
        if (strcmp(env_local_print,"AUTOMATIC")==0)
        {
            dlt_user.local_print_mode = DLT_PM_AUTOMATIC;
        }
        else if (strcmp(env_local_print,"FORCE_ON")==0)
        {
            dlt_user.local_print_mode = DLT_PM_FORCE_ON;
        }
        else if (strcmp(env_local_print,"FORCE_OFF")==0)
        {
            dlt_user.local_print_mode = DLT_PM_FORCE_OFF;
        }
    }

    /* Initialize LogLevel/TraceStatus field */
    dlt_user.dlt_ll_ts = 0;
    dlt_user.dlt_ll_ts_max_num_entries = 0;
    dlt_user.dlt_ll_ts_num_entries = 0;

    if (dlt_buffer_init_dynamic(&(dlt_user.startup_buffer), DLT_USER_RINGBUFFER_MIN_SIZE, DLT_USER_RINGBUFFER_MAX_SIZE, DLT_USER_RINGBUFFER_STEP_SIZE)==-1)
    {
		dlt_user_initialised = 0;
        return -1;
    }

    signal(SIGPIPE,SIG_IGN);                  /* ignore pipe signals */

    atexit(dlt_user_atexit_handler);

#ifdef DLT_TEST_ENABLE
    dlt_user.corrupt_user_header = 0;
    dlt_user.corrupt_message_size = 0;
    dlt_user.corrupt_message_size_size = 0;
#endif

    return 0;
}

void dlt_user_atexit_handler(void)
{

	/* Try to resend potential log messages in the user buffer */
	int count = dlt_user_atexit_blow_out_user_buffer();

	if(count != 0){
		char tmp[256];
		sprintf(tmp,"Lost log messages in user buffer when exiting: %i\n",count);
		dlt_log(LOG_ERR, tmp);
	}

    /* Unregister app (this also unregisters all contexts in daemon) */
    /* Ignore return value */
    dlt_unregister_app();

    /* Cleanup */
    /* Ignore return value */
    dlt_free();
}

int dlt_user_atexit_blow_out_user_buffer(void){

	int count,ret;

	uint32_t exitTime = dlt_uptime() + DLT_USER_ATEXIT_RESEND_BUFFER_EXIT_TIMEOUT;

	while(dlt_uptime() < exitTime ){

		ret = dlt_user_log_resend_buffer();

		if(ret == 0)
		{
				return 0;
		}

		usleep(DLT_USER_ATEXIT_RESEND_BUFFER_SLEEP);
	}

	DLT_SEM_LOCK();
	count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
	DLT_SEM_FREE();

	return count;
}

int dlt_free(void)
{
    uint32_t i;
	char filename[DLT_USER_MAX_FILENAME_LENGTH];

    if (dlt_user_initialised==0)
    {
        return -1;
    }

    if (dlt_receiverthread_handle)
    {
    	/* Ignore return value */
        pthread_cancel(dlt_receiverthread_handle);
    }

    if (dlt_user.dlt_user_handle!=DLT_FD_INIT)
    {
        sprintf(filename,"%s/dlt%d",DLT_USER_DIR,getpid());

        close(dlt_user.dlt_user_handle);
        dlt_user.dlt_user_handle=DLT_FD_INIT;

        unlink(filename);
    }

#ifdef DLT_SHM_ENABLE
	/* free shared memory */
	dlt_shm_free_client(&dlt_user.dlt_shm);
#endif

    if (dlt_user.dlt_log_handle!=-1)
    {
        /* close log file/output fifo to daemon */
        close(dlt_user.dlt_log_handle);
        dlt_user.dlt_log_handle = -1;
    }

	/* Ignore return value */
    dlt_receiver_free(&(dlt_user.receiver));

	/* Ignore return value */
    dlt_buffer_free_dynamic(&(dlt_user.startup_buffer));

    if (dlt_user.dlt_ll_ts)
    {
        for (i=0;i<dlt_user.dlt_ll_ts_max_num_entries;i++)
        {
            if (dlt_user.dlt_ll_ts[i].injection_table!=0)
            {
                free(dlt_user.dlt_ll_ts[i].injection_table);
                dlt_user.dlt_ll_ts[i].injection_table = 0;
            }
            dlt_user.dlt_ll_ts[i].nrcallbacks     = 0;
        }

        free(dlt_user.dlt_ll_ts);
        dlt_user.dlt_ll_ts = 0;
        dlt_user.dlt_ll_ts_max_num_entries = 0;
        dlt_user.dlt_ll_ts_num_entries = 0;
    }

    if (dlt_user.dlt_segmented_nwt_handle)
    {
    	pthread_cancel(dlt_user.dlt_segmented_nwt_handle);
    }

    char queue_name[NAME_MAX];
    sprintf(queue_name, "%s.%d", DLT_MESSAGE_QUEUE_NAME, getpid());

    /**
     * Ignore errors from these, to not to spam user if dlt_free
     * is accidentally called multiple times.
     */
    mq_close(dlt_user.dlt_segmented_queue_write_handle);
    mq_close(dlt_user.dlt_segmented_queue_read_handle);
    mq_unlink(queue_name);

    dlt_user_initialised = 0;

    return 0;
}

int dlt_check_library_version(const char * user_major_version,const char * user_minor_version)
{
	return dlt_user_check_library_version(user_major_version, user_minor_version);
}

int dlt_register_app(const char *appid, const char * description)
{
    int ret;

    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    if ((appid==0) || (appid[0]=='\0'))
    {
        return -1;
    }

    /* check if application already registered */
    /* if yes do not register again */
    if(appid[1]==0)
    {
       if(appid[0]==dlt_user.appID[0])
             return 0;
    }
    else if(appid[2]==0)
    {
       if(appid[0]==dlt_user.appID[0] &&
          appid[1]==dlt_user.appID[1])
             return 0;
    }
    else if(appid[3]==0)
    {
       if(appid[0]==dlt_user.appID[0] &&
          appid[1]==dlt_user.appID[1] &&
          appid[2]==dlt_user.appID[2])
             return 0;
    }
    else
    {
       if(appid[0]==dlt_user.appID[0] &&
          appid[1]==dlt_user.appID[1] &&
          appid[2]==dlt_user.appID[2] &&
          appid[3]==dlt_user.appID[3])
             return 0;
    }

    DLT_SEM_LOCK();

    /* Store locally application id and application description */
    dlt_set_id(dlt_user.appID, appid);

    if (dlt_user.application_description!=0)
    {
        free(dlt_user.application_description);
    }

    dlt_user.application_description = 0;

    if (description!=0)
    {
        size_t desc_len = strlen(description);
        dlt_user.application_description= malloc(desc_len+1);
        if (dlt_user.application_description){
            strncpy(dlt_user.application_description, description, desc_len);

            /* Terminate transmitted string with 0 */
            dlt_user.application_description[desc_len]='\0';
        }
        else
        {
        	DLT_SEM_FREE();
        	return -1;
        }
    }

    DLT_SEM_FREE();

    ret = dlt_user_log_send_register_application();

    return ret;
}
int dlt_register_context(DltContext *handle, const char *contextid, const char * description)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    DLT_SEM_LOCK();

    if (dlt_user.appID[0]=='\0')
    {
        dlt_log(LOG_ERR, "no application registered!\n");

        DLT_SEM_FREE();
        return -1;
    }

    if ((contextid==0) || (contextid[0]=='\0'))
    {
        DLT_SEM_FREE();
        return -1;
    }

    DLT_SEM_FREE();

    return dlt_register_context_ll_ts(handle, contextid, description, DLT_USER_LOG_LEVEL_NOT_SET, DLT_USER_TRACE_STATUS_NOT_SET);
}

int dlt_register_context_ll_ts(DltContext *handle, const char *contextid, const char * description, int loglevel, int tracestatus)
{
    DltContextData log;
    uint32_t i;
    int registered,ret;
    char ctid[DLT_ID_SIZE+1];

    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    DLT_SEM_LOCK();

    if (dlt_user.appID[0]=='\0')
    {
        dlt_log(LOG_ERR, "no application registered!\n");

        DLT_SEM_FREE();
        return -1;
    }

    DLT_SEM_FREE();

    if ((contextid==0) || (contextid[0]=='\0'))
    {
        return -1;
    }

    if ((loglevel<DLT_USER_LOG_LEVEL_NOT_SET) || (loglevel>DLT_LOG_VERBOSE) || (loglevel==DLT_LOG_DEFAULT))
    {
        return -1;
    }

    if ((tracestatus<DLT_USER_TRACE_STATUS_NOT_SET) || (tracestatus>DLT_TRACE_STATUS_ON) || (tracestatus==DLT_TRACE_STATUS_DEFAULT))
    {
        return -1;
    }

    if (dlt_user_log_init(handle, &log)==-1)
    {
    	return -1;
    }

    /* Reset message counter */
    handle->mcnt = 0;

    /* Store context id in log level/trace status field */

    /* Check if already registered, else register context */
    DLT_SEM_LOCK();

    registered=0;
    for (i=0;i<dlt_user.dlt_ll_ts_num_entries;i++)
    {
        if (dlt_user.dlt_ll_ts)
        {
            if (memcmp(dlt_user.dlt_ll_ts[i].contextID, contextid,DLT_ID_SIZE)==0)
            {
                registered=1;

                memset(ctid,0,(DLT_ID_SIZE+1));
                dlt_print_id(ctid, contextid);

                sprintf(str,"context '%s' already registered!\n",ctid);
                dlt_log(LOG_WARNING, str);

                break;
            }
        }
    }

    if (registered==0)
    {
        /* Allocate or expand context array */
        if (dlt_user.dlt_ll_ts == 0)
        {
            dlt_user.dlt_ll_ts = (dlt_ll_ts_type*) malloc(sizeof(dlt_ll_ts_type)*DLT_USER_CONTEXT_ALLOC_SIZE);
            if (dlt_user.dlt_ll_ts==0)
            {
                DLT_SEM_FREE();
                return -1;
            }

            dlt_user.dlt_ll_ts_max_num_entries = DLT_USER_CONTEXT_ALLOC_SIZE;

            /* Initialize new entries */
            for (i=0;i<dlt_user.dlt_ll_ts_max_num_entries;i++)
            {
                dlt_set_id(dlt_user.dlt_ll_ts[i].contextID,"");

                /* At startup, logging and tracing is locally enabled */
                /* the correct log level/status is set after received from daemon */
                dlt_user.dlt_ll_ts[i].log_level    = DLT_USER_INITIAL_LOG_LEVEL;
                dlt_user.dlt_ll_ts[i].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

                dlt_user.dlt_ll_ts[i].context_description = 0;

                dlt_user.dlt_ll_ts[i].injection_table = 0;
                dlt_user.dlt_ll_ts[i].nrcallbacks     = 0;
            }
        }
        else
        {
            if ((dlt_user.dlt_ll_ts_num_entries%DLT_USER_CONTEXT_ALLOC_SIZE)==0)
            {
                /* allocate memory in steps of DLT_USER_CONTEXT_ALLOC_SIZE, e.g. 500 */
                dlt_ll_ts_type *old_ll_ts;
                uint32_t old_max_entries;

                old_ll_ts = dlt_user.dlt_ll_ts;
                old_max_entries = dlt_user.dlt_ll_ts_max_num_entries;

                dlt_user.dlt_ll_ts_max_num_entries = ((dlt_user.dlt_ll_ts_num_entries/DLT_USER_CONTEXT_ALLOC_SIZE)+1)*DLT_USER_CONTEXT_ALLOC_SIZE;
                dlt_user.dlt_ll_ts = (dlt_ll_ts_type*) malloc(sizeof(dlt_ll_ts_type)*
                                     dlt_user.dlt_ll_ts_max_num_entries);
                if (dlt_user.dlt_ll_ts==0)
                {
                	dlt_user.dlt_ll_ts = old_ll_ts;
                	dlt_user.dlt_ll_ts_max_num_entries = old_max_entries;
                    DLT_SEM_FREE();
                    return -1;
                }

                memcpy(dlt_user.dlt_ll_ts,old_ll_ts,sizeof(dlt_ll_ts_type)*dlt_user.dlt_ll_ts_num_entries);
                free(old_ll_ts);

                /* Initialize new entries */
                for (i=dlt_user.dlt_ll_ts_num_entries;i<dlt_user.dlt_ll_ts_max_num_entries;i++)
                {
                    dlt_set_id(dlt_user.dlt_ll_ts[i].contextID,"");

                    /* At startup, logging and tracing is locally enabled */
                    /* the correct log level/status is set after received from daemon */
                    dlt_user.dlt_ll_ts[i].log_level    = DLT_USER_INITIAL_LOG_LEVEL;
                    dlt_user.dlt_ll_ts[i].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

                    dlt_user.dlt_ll_ts[i].context_description = 0;

                    dlt_user.dlt_ll_ts[i].injection_table = 0;
                    dlt_user.dlt_ll_ts[i].nrcallbacks     = 0;
                }
            }
        }

        /* Store locally context id and context description */
        dlt_set_id(dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].contextID, contextid);

        if (dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description!=0)
        {
            free(dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description);
        }

        dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description = 0;

        if (description!=0)
        {
        	size_t desc_len = strlen(description);
            dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description = malloc(desc_len+1);
            if(dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description == 0)
            {
            	DLT_SEM_FREE();
            	return -1;
            }

            strncpy(dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description, description, desc_len);

            /* Terminate transmitted string with 0 */
            dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description[desc_len]='\0';
        }

        if (loglevel!=DLT_USER_LOG_LEVEL_NOT_SET)
        {
            dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].log_level = loglevel;
        }

        if (tracestatus!=DLT_USER_TRACE_STATUS_NOT_SET)
        {
            dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].trace_status = tracestatus;
        }

        /* Prepare transfer struct */
        //dlt_set_id(log->appID, dlt_user.appID);
        dlt_set_id(handle->contextID, contextid);
        handle->log_level_pos = dlt_user.dlt_ll_ts_num_entries;

        log.context_description = dlt_user.dlt_ll_ts[dlt_user.dlt_ll_ts_num_entries].context_description;

        if (loglevel!=DLT_USER_LOG_LEVEL_NOT_SET)
        {
            log.log_level = loglevel;
        }
        else
        {
            log.log_level = DLT_USER_LOG_LEVEL_NOT_SET;
        }

        if (tracestatus!=DLT_USER_TRACE_STATUS_NOT_SET)
        {
            log.trace_status = tracestatus;
        }
        else
        {
            log.trace_status = DLT_USER_TRACE_STATUS_NOT_SET;
        }

        dlt_user.dlt_ll_ts_num_entries++;

        DLT_SEM_FREE();

        ret=dlt_user_log_send_register_context(&log);
    }
    else
    {
        DLT_SEM_FREE();

        ret=-1;
    }

    return ret;
}

int dlt_unregister_app(void)
{
    int ret;

    if (dlt_user_initialised==0)
    {
        return -1;
    }

    /* Inform daemon to unregister application and all of its contexts */
    ret = dlt_user_log_send_unregister_application();

    DLT_SEM_LOCK();

    /* Clear and free local stored application information */
    dlt_set_id(dlt_user.appID, "");

    if (dlt_user.application_description!=0)
    {
        free(dlt_user.application_description);
    }

    dlt_user.application_description = 0;

    DLT_SEM_FREE();

    return ret;
}

int dlt_unregister_context(DltContext *handle)
{
    DltContextData log;
    int ret;

    if (dlt_user_initialised==0)
    {
        return -1;
    }

    if (dlt_user_log_init(handle, &log) == -1)
    {
		return -1;
    }

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts)
    {
        /* Clear and free local stored context information */
        dlt_set_id(dlt_user.dlt_ll_ts[handle->log_level_pos].contextID, "");

        dlt_user.dlt_ll_ts[handle->log_level_pos].log_level = DLT_USER_INITIAL_LOG_LEVEL;
        dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status = DLT_USER_INITIAL_TRACE_STATUS;

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].context_description!=0)
        {
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].context_description);
        }

        dlt_user.dlt_ll_ts[handle->log_level_pos].context_description = 0;

        if (dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table)
        {
            free(dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table);
            dlt_user.dlt_ll_ts[handle->log_level_pos].injection_table = 0;
        }

        dlt_user.dlt_ll_ts[handle->log_level_pos].nrcallbacks     = 0;
    }

    DLT_SEM_FREE();

    /* Inform daemon to unregister context */
    ret = dlt_user_log_send_unregister_context(&log);

    return ret;
}

int dlt_set_application_ll_ts_limit(DltLogLevelType loglevel, DltTraceStatusType tracestatus)
{
    uint32_t i;
    int ret;

    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    /* Removed because of DltLogLevelType and DltTraceStatusType

    if ((loglevel<DLT_LOG_DEFAULT) || (loglevel>DLT_LOG_VERBOSE))
    {
        return -1;
    }

    if ((tracestatus<DLT_TRACE_STATUS_DEFAULT) || (tracestatus>DLT_TRACE_STATUS_ON))
    {
        return -1;
    }

    if (dlt_user.dlt_ll_ts==0)
    {
        return -1;
    }

    */

    DLT_SEM_LOCK();

    /* Update local structures */
    for (i=0; i<dlt_user.dlt_ll_ts_num_entries;i++)
    {
        dlt_user.dlt_ll_ts[i].log_level = loglevel;
        dlt_user.dlt_ll_ts[i].trace_status = tracestatus;
    }

    DLT_SEM_FREE();

    /* Inform DLT server about update */
    ret = dlt_send_app_ll_ts_limit(dlt_user.appID, loglevel, tracestatus);

    return ret;
}

int dlt_get_log_state()
{
	return dlt_user.log_state;
}

int dlt_set_log_mode(DltUserLogMode mode)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

	return dlt_user_log_send_log_mode(mode);
}

int dlt_forward_msg(void *msgdata,size_t size)
{
    DltUserHeader userheader;
    DltReturnValue ret;

    if ((msgdata==0) || (size==0))
    {
        return -1;
    }

    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG)==-1)
    {
    	/* Type of internal user message; same value for Trace messages */
    	return -1;
    }

    if (dlt_user.dlt_is_file)
    {
        /* log to file */
        ret = dlt_user_log_out2(dlt_user.dlt_log_handle, msgdata, size, 0, 0);
        return ((ret==DLT_RETURN_OK)?0:-1);
    }
    else
    {
        /* Reattach to daemon if neccesary */
        dlt_user_log_reattach_to_daemon();

        if (dlt_user.overflow)
        {
            if (dlt_user_log_send_overflow()==0)
            {
                dlt_user.overflow=0;
            }
        }

        /* log to FIFO */
        ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
                                &(userheader), sizeof(DltUserHeader),
                                msgdata, size, 0, 0);

        /* store message in ringbuffer, if an error has occured */
        if (ret!=DLT_RETURN_OK)
        {
            DLT_SEM_LOCK();

            if (dlt_buffer_push3(&(dlt_user.startup_buffer),
                                (unsigned char *)&(userheader), sizeof(DltUserHeader),
                                 msgdata, size, 0, 0)==-1)
			{
				dlt_log(LOG_ERR,"Storing message to history buffer failed! Message discarded.\n");
			}

            DLT_SEM_FREE();

            if(dlt_user_queue_resend() < 0 && dlt_user.dlt_log_handle >= 0)
            {
            	dlt_log(LOG_WARNING, "Failed to queue resending.\n");
            }
        }

        switch (ret)
        {
        case DLT_RETURN_PIPE_FULL:
        {
            /* data could not be written */
            dlt_user.overflow = 1;
            return -1;
        }
        case DLT_RETURN_PIPE_ERROR:
        {
            /* handle not open or pipe error */
            close(dlt_user.dlt_log_handle);
            dlt_user.dlt_log_handle = -1;

            return -1;
        }
        case DLT_RETURN_ERROR:
        {
            /* other error condition */
            return -1;
        }
        case DLT_RETURN_OK:
        {
        	return 0;
        }
		default:
		{
			/* This case should not occur */
			return -1;
		}
        }
    }

    return 0;
}

/* ********************************************************************************************* */

inline int dlt_user_log_write_start(DltContext *handle, DltContextData *log,DltLogLevelType loglevel)
{
    return dlt_user_log_write_start_id(handle,log,loglevel,DLT_USER_DEFAULT_MSGID);
}

int dlt_user_log_write_start_id(DltContext *handle, DltContextData *log,DltLogLevelType loglevel, uint32_t messageid)
{
	if(dlt_user_initialised==0)
	{
		if (dlt_init()<0)
		{
			return -1;
		}
	}
    if (log==0)
    {
        return -1;
    }

    if (dlt_user_log_init(handle, log)==-1)
    {
		return -1;
    }

    if (dlt_user.dlt_ll_ts==0)
    {
        return -1;
    }

    DLT_SEM_LOCK();

    if ((loglevel<=(int)(dlt_user.dlt_ll_ts[handle->log_level_pos].log_level) ) && (loglevel!=0))
    {
        DLT_SEM_FREE();
		log->args_num = 0;
    	log->log_level = loglevel;

        /* In non-verbose mode, insert message id */
        if (dlt_user.verbose_mode==0)
        {
            if ((sizeof(uint32_t))>DLT_USER_BUF_MAX_SIZE)
            {
                return -1;
            }
            /* Write message id */
            memcpy(log->buffer,&(messageid),sizeof(uint32_t));
            log->size = sizeof(uint32_t);

            /* as the message id is part of each message in non-verbose mode,
               it doesn't increment the argument counter in extended header (if used) */
        }
        else log->size=0;
        return 1;
    }
    else
    {
        DLT_SEM_FREE();
        return 0;
    }

    return -1;
}

int dlt_user_log_write_finish(DltContextData *log)
{
    if (log==0)
    {
        return -1;
    }

    return dlt_user_log_send_log(log, DLT_TYPE_LOG) < 0 ? -1 : 0;
}

int dlt_user_log_write_raw(DltContextData *log,void *data,uint16_t length)
{
    uint16_t arg_size;
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+length+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+length+sizeof(uint32_t)+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        /* Transmit type information */
        type_info = DLT_TYPE_INFO_RAWD;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);

    }

    /* First transmit length of raw data, then the raw data itself */
    arg_size = (uint16_t)length;

    memcpy((log->buffer)+log->size,&(arg_size),sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    memcpy((log->buffer)+log->size,data,arg_size);
    log->size += arg_size;

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_float32(DltContextData *log, float32_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if (sizeof(float32_t)!=4)
    {
        return -1;
    }

    if ((log->size+sizeof(float32_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(float32_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_FLOA | DLT_TYLE_32BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data, sizeof(float32_t));
    log->size += sizeof(float32_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_float64(DltContextData *log, float64_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if (sizeof(float64_t)!=8)
    {
        return -1;
    }

    if ((log->size+sizeof(float64_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(float64_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_FLOA | DLT_TYLE_64BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data, sizeof(float64_t));
    log->size += sizeof(float64_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_uint( DltContextData *log, unsigned int data)
{
    if (log==0)
    {
        return -1;
    }

    switch (sizeof(unsigned int))
    {
    case 1:
    {
        return dlt_user_log_write_uint8(log, (uint8_t)data);
        break;
    }
    case 2:
    {
        return dlt_user_log_write_uint16(log, (uint16_t)data);
        break;
    }
    case 4:
    {
        return dlt_user_log_write_uint32(log, (uint32_t)data);
        break;
    }
    case 8:
    {
        return dlt_user_log_write_uint64(log, (uint64_t)data);
		break;
    }
    default:
    {
        return -1;
        break;
    }
    }

    return 0;
}

int dlt_user_log_write_uint8(DltContextData *log, uint8_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(uint8_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(uint8_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_8BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(uint8_t));
    log->size += sizeof(uint8_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_uint16(DltContextData *log, uint16_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_uint32(DltContextData *log, uint32_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(uint32_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(uint32_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data, sizeof(uint32_t));
    log->size += sizeof(uint32_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_uint64(DltContextData *log, uint64_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(uint64_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(uint64_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_64BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size +=sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(uint64_t));
    log->size += sizeof(uint64_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_int(DltContextData *log, int data)
{
    if (log==0)
    {
        return -1;
    }

    switch (sizeof(int))
    {
    case 1:
    {
        return dlt_user_log_write_int8(log, (int8_t)data);
        break;
    }
    case 2:
    {
        return dlt_user_log_write_int16(log, (int16_t)data);
        break;
    }
    case 4:
    {
        return dlt_user_log_write_int32(log, (int32_t)data);
        break;
    }
    case 8:
    {
        return dlt_user_log_write_int64(log, (int64_t)data);
		break;
    }
    default:
    {
        return -1;
        break;
    }
    }

    return 0;
}

int dlt_user_log_write_int8(DltContextData *log, int8_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(int8_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(int8_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_8BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(int8_t));
    log->size += sizeof(int8_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_int16(DltContextData *log, int16_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(int16_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(int16_t))>DLT_USER_BUF_MAX_SIZE)
		{
            return -1;
		}

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_16BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(int16_t));
    log->size += sizeof(int16_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_int32(DltContextData *log, int32_t data)
{
    uint32_t type_info;

    if (log==0)
    {
    	return -1;
    }

    if ((log->size+sizeof(int32_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(int32_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_32BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data, sizeof(int32_t));
    log->size += sizeof(int32_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_int64(DltContextData *log, int64_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(int64_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(int64_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_SINT | DLT_TYLE_64BIT;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(int64_t));
    log->size += sizeof(int64_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_bool(DltContextData *log, uint8_t data)
{
    uint32_t type_info;

    if (log==0)
    {
        return -1;
    }

    if ((log->size+sizeof(uint8_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+sizeof(uint32_t)+sizeof(uint8_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_BOOL;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    memcpy((log->buffer)+log->size,&data,sizeof(uint8_t));
    log->size += sizeof(uint8_t);

    log->args_num ++;

    return 0;
}

int dlt_user_log_write_string(DltContextData *log, const char *text)
{
    uint16_t arg_size;
    uint32_t type_info;

    if ((log==0) || (text==0))
    {
        return -1;
    }

    if ((log->size+(strlen(text)+1)+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
    {
        return -1;
    }

    if (dlt_user.verbose_mode)
    {
        if ((log->size+(strlen(text)+1)+sizeof(uint32_t)+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
        {
            return -1;
        }

        type_info = DLT_TYPE_INFO_STRG;

        memcpy((log->buffer)+log->size,&(type_info),sizeof(uint32_t));
        log->size += sizeof(uint32_t);
    }

    arg_size = strlen(text) + 1;

    memcpy((log->buffer)+log->size,&(arg_size),sizeof(uint16_t));
    log->size += sizeof(uint16_t);

    memcpy((log->buffer)+log->size,text,arg_size);
    log->size += arg_size;

    log->args_num ++;

    return 0;
}

int dlt_register_injection_callback(DltContext *handle, uint32_t service_id,
                                    int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length))
{
    DltContextData log;
    uint32_t i,j,k;
    int found = 0;

	DltUserInjectionCallback *old;

    if (handle==0)
    {
        return -1;
    }

    if (dlt_user_log_init(handle, &log)==-1)
    {
		return -1;
    }

    if (service_id<DLT_USER_INJECTION_MIN)
    {
        return -1;
    }
    /* This function doesn't make sense storing to local file is choosen;
       so terminate this function */
    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts==0)
    {
        DLT_SEM_FREE();
        return 0;
    }

    /* Insert callback in corresponding table */
    i=handle->log_level_pos;

    /* Insert each service_id only once */
    for (k=0;k<dlt_user.dlt_ll_ts[i].nrcallbacks;k++)
    {
        if ((dlt_user.dlt_ll_ts[i].injection_table) &&
                (dlt_user.dlt_ll_ts[i].injection_table[k].service_id == service_id))
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        j = k;
    }
    else
    {
        j=dlt_user.dlt_ll_ts[i].nrcallbacks;

        /* Allocate or expand injection table */
        if (dlt_user.dlt_ll_ts[i].injection_table == 0)
        {
            dlt_user.dlt_ll_ts[i].injection_table = (DltUserInjectionCallback*) malloc(sizeof(DltUserInjectionCallback));
            if(dlt_user.dlt_ll_ts[i].injection_table == 0)
            {
            	DLT_SEM_FREE();
            	return -1;
            }
        }
        else
        {
            old = dlt_user.dlt_ll_ts[i].injection_table;
            dlt_user.dlt_ll_ts[i].injection_table = (DltUserInjectionCallback*) malloc(sizeof(DltUserInjectionCallback)*(j+1));
            if(dlt_user.dlt_ll_ts[i].injection_table == 0)
            {
            	dlt_user.dlt_ll_ts[i].injection_table = old;
            	DLT_SEM_FREE();
            	return -1;
            }
            memcpy(dlt_user.dlt_ll_ts[i].injection_table,old,sizeof(DltUserInjectionCallback)*j);
            free(old);
        }

        dlt_user.dlt_ll_ts[i].nrcallbacks++;
    }

    /* Store service_id and corresponding function pointer for callback function */
    dlt_user.dlt_ll_ts[i].injection_table[j].service_id = service_id;
    dlt_user.dlt_ll_ts[i].injection_table[j].injection_callback = dlt_injection_callback;

    DLT_SEM_FREE();
    return 0;
}

/**
 * NW Trace related
 */


int check_buffer()
{
	int total_size, used_size;
	dlt_user_check_buffer(&total_size, &used_size);

	if((total_size - used_size) < (total_size/2))
	{
		return -1;
	}
	return 1;
}

/**
 * Send the start of a segment chain.
 * Returns -1 on failure
 */
int dlt_user_trace_network_segmented_start(uint16_t *id, DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len)
{

    DltContextData log;
	struct timeval tv;

    if (handle==NULL)
    {
        return -1;
    }

    if (dlt_user_log_init(handle, &log)==-1)
    {
		return -1;
    }

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts==0)
    {
        DLT_SEM_FREE();
        return -1;
    }

    if (dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status==DLT_TRACE_STATUS_ON)
    {
        DLT_SEM_FREE();

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

    	gettimeofday(&tv, NULL);
    	*id = tv.tv_usec;

    	/* Write identifier */
        if(dlt_user_log_write_string(&log, "NWST") < 0)
        {
        	return -1;
        }

        /* Write stream handle */
        if(dlt_user_log_write_uint16(&log, *id) < 0)
        {
        	return -1;
        }

        /* Write header */
        if(dlt_user_log_write_raw(&log, header, header_len) < 0)
        {
        	return -1;
        }

        /* Write size of payload */
        if(dlt_user_log_write_uint16(&log, payload_len) < 0)
        {
        	return -1;
        }

        /* Write expected segment count */
        uint16_t segment_count = payload_len/DLT_MAX_TRACE_SEGMENT_SIZE+1;

        /* If segments align perfectly with segment size, avoid sending empty segment */
        if((payload_len % DLT_MAX_TRACE_SEGMENT_SIZE) == 0)
        {
        	segment_count--;
        }

        if(dlt_user_log_write_uint16(&log, segment_count) < 0)
        {
        	return -1;
        }

        /* Write length of one segment */
        if(dlt_user_log_write_uint16(&log, DLT_MAX_TRACE_SEGMENT_SIZE) < 0)
        {
        	return -1;
        }

        /* Send log */
        return dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
    }
    else
    {
        DLT_SEM_FREE();
    }
    return 0;
}

int dlt_user_trace_network_segmented_segment(uint16_t id, DltContext *handle, DltNetworkTraceType nw_trace_type, int sequence, uint16_t payload_len, void *payload)
{
	while(check_buffer() < 0)
	{
		usleep(1000*50); // Wait 50ms
		dlt_user_log_resend_buffer();
	}

    if (handle==NULL)
    {
        return -1;
    }

    DltContextData log;

    if (dlt_user_log_init(handle, &log)==-1)
    {
		return -1;
    }

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts==0)
    {
        DLT_SEM_FREE();
        return -1;
    }

    if (dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status==DLT_TRACE_STATUS_ON)
    {
        DLT_SEM_FREE();

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        /* Write identifier */
        if(dlt_user_log_write_string(&log, "NWCH") < 0)
        {
        	return -1;
        }

        /* Write stream handle */
        if(dlt_user_log_write_uint16(&log, id) < 0)
        {
        	return -1;
        }

        /* Write segment sequence number */
        if(dlt_user_log_write_uint16(&log, sequence) < 0)
        {
        	return -1;
        }

        /* Write data */
        if(dlt_user_log_write_raw(&log, payload, payload_len) < 0)
        {
        	return -1;
        }

        /* Send log */
        return dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
    }
    else
    {
        DLT_SEM_FREE();
    }

    /* Allow other threads to log between chunks */
	pthread_yield();
    return 0;
}

int dlt_user_trace_network_segmented_end(uint16_t id, DltContext *handle, DltNetworkTraceType nw_trace_type)
{
    DltContextData log;

    if (handle==0)
    {
        return -1;
    }

    if (dlt_user_log_init(handle, &log)==-1)
    {
		return -1;
    }



    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts==0)
    {
        DLT_SEM_FREE();
        return -1;
    }

    if (dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status==DLT_TRACE_STATUS_ON)
    {
        DLT_SEM_FREE();

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        /* Write identifier */
        if(dlt_user_log_write_string(&log, "NWEN") < 0)
        {
        	return -1;
        }

        /* Write stream handle */
        if(dlt_user_log_write_uint(&log, id) < 0)
        {
        	return -1;
        }

        /* Send log */
        return dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
    }
    else
    {
        DLT_SEM_FREE();
    }
    return 0;
}

void dlt_user_trace_network_segmented_thread(void *unused)
{
	/* Unused on purpose. */
	(void) unused;

	s_segmented_data *data;

	while(1)
	{
		// Wait untill message queue is initialized
		pthread_mutex_lock(&mq_mutex);
		if(dlt_user.dlt_segmented_queue_read_handle < 0)
		{
			pthread_cond_wait(&mq_init_condition, &mq_mutex);
		}
		pthread_mutex_unlock(&mq_mutex);

		ssize_t read = mq_receive(dlt_user.dlt_segmented_queue_read_handle, (char *)&data,
					sizeof(s_segmented_data * ), NULL);

		if(read != sizeof(s_segmented_data *))
		{
			dlt_log(LOG_ERR, "NWTSegmented: Error while reading queue.\n");
			dlt_log(LOG_ERR, strerror(errno));
			continue;
		}

		/* Indicator just to try to flush the buffer */
		if(data->payload_len == DLT_DELAYED_RESEND_INDICATOR_PATTERN)
		{
			// Sleep 100ms, to allow other process to read FIFO
			usleep(100*1000);
			if(dlt_user_log_resend_buffer() < 0)
			{
				// Requeue if still not empty
				dlt_user_queue_resend();
			}
			free(data);
			continue;
		}

		/* Segment the data and send the chunks */
		void *ptr 			= NULL;
		uint16_t offset		= 0;
		uint16_t sequence	= 0;
		do
		{
			uint16_t len = 0;
			if(offset + DLT_MAX_TRACE_SEGMENT_SIZE > data->payload_len)
			{
				len = data->payload_len - offset;
			}
			else
			{
				len = DLT_MAX_TRACE_SEGMENT_SIZE;
			}
			/* If payload size aligns perfectly with segment size, avoid sendind empty segment */
			if(len == 0)
			{
				break;
			}

			ptr = data->payload + offset;
			DltReturnValue err = dlt_user_trace_network_segmented_segment(data->id, data->handle, data->nw_trace_type, sequence++, len, ptr);
			if(err == DLT_RETURN_BUFFER_FULL || err == DLT_RETURN_ERROR)
			{
				dlt_log(LOG_ERR,"NWTSegmented: Could not send segment. Aborting.\n");
				break; // Inner loop
			}
			offset += len;
		}while(ptr < data->payload + data->payload_len);

		/* Send the end message */
		DltReturnValue err = dlt_user_trace_network_segmented_end(data->id, data->handle, data->nw_trace_type);
		if(err == DLT_RETURN_BUFFER_FULL || err == DLT_RETURN_ERROR)
		{
			dlt_log(LOG_ERR,"NWTSegmented: Could not send end segment.\n");
		}

		/* Free resources */
		free(data->header);
		free(data->payload);
		free(data);
	}
}

int dlt_user_trace_network_segmented(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload)
{
	/* Send as normal trace if possible */
	if(header_len+payload_len+sizeof(uint16_t) < DLT_USER_BUF_MAX_SIZE) {
		return dlt_user_trace_network(handle, nw_trace_type, header_len, header, payload_len, payload);
	}

	/* Allocate Memory */
	s_segmented_data *thread_data = malloc(sizeof(s_segmented_data));
	if(thread_data == NULL)
	{
		return -1;
	}
	thread_data->header = malloc(header_len);
	if(thread_data->header == NULL)
	{
		free(thread_data);
		return -1;
	}
	thread_data->payload = malloc(payload_len);
	if(thread_data->payload == NULL)
	{
		free(thread_data->header);
		free(thread_data);
		return -1;
	}

	/* Copy data */
	thread_data->handle = handle;
	thread_data->nw_trace_type = nw_trace_type;
	thread_data->header_len = header_len;
	memcpy(thread_data->header, header, header_len);
	thread_data->payload_len = payload_len;
	memcpy(thread_data->payload, payload, payload_len);

	/* Send start message */
	DltReturnValue err = 	dlt_user_trace_network_segmented_start(&(thread_data->id),
							thread_data->handle, thread_data->nw_trace_type,
							thread_data->header_len, thread_data->header,
							thread_data->payload_len);
	if(err == DLT_RETURN_BUFFER_FULL || err == DLT_RETURN_ERROR)
	{
		dlt_log(LOG_ERR,"NWTSegmented: Could not send start segment. Aborting.\n");
		free(thread_data->header);
		free(thread_data->payload);
		free(thread_data);
		return -1;
	}

	/* Open queue if it is not open */
	if(dlt_init_message_queue() < 0)
	{
		dlt_log(LOG_ERR, "NWTSegmented: Could not open queue.\n");
		return -1;
	}

	/* Add to queue */
	if(mq_send(dlt_user.dlt_segmented_queue_write_handle,
			(char *)&thread_data, sizeof(s_segmented_data *), 1) < 0)
	{
		if(errno == EAGAIN)
		{
			dlt_log(LOG_ERR, "NWTSegmented: Queue full. Message discarded.\n");
		}
		free(thread_data->header);
		free(thread_data->payload);
		free(thread_data);
		dlt_log(LOG_ERR, "NWTSegmented: Could not write into queue.\n");
		dlt_log(LOG_ERR, strerror(errno));
		return -1;
	}
	return 0;
}

int dlt_user_trace_network(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload)
{
	return dlt_user_trace_network_truncated(handle, nw_trace_type, header_len, header, payload_len, payload, 1);
}

int dlt_user_trace_network_truncated(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload, int allow_truncate)
{
    DltContextData log;

    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    if (dlt_user_log_init(handle, &log)==-1)
    {
		return -1;
    }

    if (handle==0)
    {
        return -1;
    }

    /* Commented out because of DltNetworkTraceType:

    if ((nw_trace_type<=0) || (nw_trace_type>0x15))
    {
        return -1;
    }

    */

    DLT_SEM_LOCK();

    if (dlt_user.dlt_ll_ts==0)
    {
        DLT_SEM_FREE();
        return -1;
    }

    if (dlt_user.dlt_ll_ts[handle->log_level_pos].trace_status==DLT_TRACE_STATUS_ON)
    {
        DLT_SEM_FREE();

        log.args_num = 0;
        log.trace_status = nw_trace_type;
        log.size = 0;

        if (header==0)
        {
            header_len=0;
        }

        /* If truncation is allowed, check if we must do it */
        if(allow_truncate > 0 && (header_len+payload_len+sizeof(uint16_t))>DLT_USER_BUF_MAX_SIZE)
        {
        	/* Identify as truncated */
        	if(dlt_user_log_write_string(&log, "NWTR") < 0)
        	{
        		return -1;
        	}

            /* Write header and its length */
            if (dlt_user_log_write_raw(&log, header, header_len) < 0)
            {
            	return -1;
            }

        	/* Write original size of payload */
            if(dlt_user_log_write_uint16(&log, payload_len) < 0)
            {
            	return -1;
            }

        	/**
        	 *  Calculate maximum avaialble space in sending buffer after headers.
        	 */

        	int truncated_payload_len = DLT_USER_BUF_MAX_SIZE -
        			log.size - sizeof(uint16_t) - sizeof(uint32_t);

            /* Write truncated payload */
            if (dlt_user_log_write_raw(&log, payload, truncated_payload_len) < 0)
            {
    			return -1;
            }
        }
        else /* Truncation not allowed or data short enough */
        {
            /* Write header and its length */
            if (dlt_user_log_write_raw(&log, header, header_len)==-1)
            {
            	return -1;
            }

            if (payload==0)
            {
                payload_len=0;
            }

            /* Write payload and its length */
            if (dlt_user_log_write_raw(&log, payload, payload_len)==-1)
            {
    			return -1;
            }
        }

        /* Send log */
        return dlt_user_log_send_log(&log, DLT_TYPE_NW_TRACE);
    }
    else
    {
        DLT_SEM_FREE();
    }

    return 0;
}

int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if ((handle==0) || (text==0))
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_string(&log,text)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
        	return -1;
        }
    }

    return 0;
}

int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if ((handle==0) || (text==0))
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_string(&log,text)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_int(&log,data)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
			return -1;
        }
    }

    return 0;
}

int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if ((handle==0) || (text==0))
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_string(&log,text)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_uint(&log,data)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
			return -1;
        }
    }

    return 0;
}

int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if (handle==0)
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_int(&log,data)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
			return -1;
        }
    }

    return 0;
}

int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if (handle==0)
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_uint(&log,data)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
			return -1;
        }
    }

    return 0;
}

int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length)
{
    DltContextData log;

    if (dlt_user.verbose_mode==0)
    {
        return -1;
    }

    if (handle==0)
    {
        return -1;
    }

    if (dlt_user_log_write_start(handle,&log,loglevel)>0)
    {
        if (dlt_user_log_write_raw(&log,data,length)==-1)
        {
			return -1;
        }
        if (dlt_user_log_write_finish(&log)==-1)
        {
			return -1;
        }
    }

    return 0;
}

int dlt_verbose_mode(void)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    /* Switch to verbose mode */
    dlt_user.verbose_mode = 1;

    return 0;
}

int dlt_nonverbose_mode(void)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    /* Switch to non-verbose mode */
    dlt_user.verbose_mode = 0;

    return 0;
}

int dlt_enable_local_print(void)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    dlt_user.enable_local_print = 1;

    return 0;
}

int dlt_disable_local_print(void)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    dlt_user.enable_local_print = 0;

    return 0;
}

void dlt_user_receiverthread_function(__attribute__((unused)) void *ptr)
{
    while (1)
    {
        /* Check for new messages from DLT daemon */
        if (dlt_user_log_check_user_message()==-1)
        {
			/* Critical error */
			dlt_log(LOG_CRIT,"Receiver thread encountered error condition\n");
        }

        usleep(DLT_USER_RECEIVE_DELAY); /* delay */
    }
}

/* Private functions of user library */

int dlt_user_log_init(DltContext *handle, DltContextData *log)
{
    if (dlt_user_initialised==0)
    {
        if (dlt_init()<0)
        {
            return -1;
        }
    }

    log->handle = handle;

    return 0;
}

int dlt_user_queue_resend(void)
{
	if(dlt_user.dlt_log_handle < 0)
	{
		// Fail silenty. FIFO is not open yet
		return -1;
	}
    /**
     * Ask segmented thread to try emptying the buffer soon.
     * This will be freed in dlt_user_trace_network_segmented_thread
     * */
    s_segmented_data *resend_data = malloc(sizeof(s_segmented_data));
    resend_data->payload_len = DLT_DELAYED_RESEND_INDICATOR_PATTERN;

    /* Open queue if it is not open */
	if(dlt_init_message_queue() < 0)
	{
		dlt_log(LOG_ERR, "NWTSegmented: Could not open queue.\n");
		return -1;
	}

    if(mq_send(dlt_user.dlt_segmented_queue_write_handle, (char *)&resend_data, sizeof(s_segmented_data *), 1) < 0)
    {
    	dlt_log(LOG_ERR,"Could not request resending.\n");
    	dlt_log(LOG_ERR, strerror(errno));
    	free(resend_data);
    	DLT_SEM_FREE();
    	return -1;
    }
    return 0;
}

DltReturnValue dlt_user_log_send_log(DltContextData *log, int mtype)
{
    DltMessage msg;
    DltUserHeader userheader;
    int32_t len;

    DltReturnValue ret = DLT_RETURN_OK;

    if (log==0)
    {
        return DLT_RETURN_ERROR;
    }

    if (log->handle==0)
    {
        return DLT_RETURN_ERROR;
    }

    if (dlt_user.appID[0]=='\0')
    {
        return DLT_RETURN_ERROR;
    }

    if (log->handle->contextID[0]=='\0')
    {
        return DLT_RETURN_ERROR;
    }

    if ((mtype<DLT_TYPE_LOG) || (mtype>DLT_TYPE_CONTROL))
    {
        return DLT_RETURN_ERROR;
    }

    /* also for Trace messages */
#ifdef DLT_SHM_ENABLE
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_SHM)==-1)
#else
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG)==-1)
#endif
    {
		return DLT_RETURN_ERROR;
    }

    if (dlt_message_init(&msg,0)==-1)
    {
    	return DLT_RETURN_ERROR;
    }

    msg.storageheader = (DltStorageHeader*)msg.headerbuffer;

    if (dlt_set_storageheader(msg.storageheader,dlt_user.ecuID)==-1)
    {
		return DLT_RETURN_ERROR;
    }

    msg.standardheader = (DltStandardHeader*)(msg.headerbuffer + sizeof(DltStorageHeader));
    msg.standardheader->htyp = DLT_HTYP_WEID | DLT_HTYP_WTMS | DLT_HTYP_PROTOCOL_VERSION1 ;

    if (dlt_user.verbose_mode)
    {
        /* In verbose mode, send extended header */
        msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_UEH );
    }
    else
    {
        /* In non-verbose, send extended header if desired */
#if (DLT_USER_USE_EXTENDED_HEADER_FOR_NONVERBOSE==1)
        msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_UEH );
#endif
    }

#if (BYTE_ORDER==BIG_ENDIAN)
    msg.standardheader->htyp = (msg.standardheader->htyp | DLT_HTYP_MSBF);
#endif

    msg.standardheader->mcnt = log->handle->mcnt++;

    /* Set header extra parameters */
    dlt_set_id(msg.headerextra.ecu,dlt_user.ecuID);
    //msg.headerextra.seid = 0;
    msg.headerextra.tmsp = dlt_uptime();

    if (dlt_message_set_extraparameters(&msg,0)==-1)
    {
    	return DLT_RETURN_ERROR;
    }

    /* Fill out extended header, if extended header should be provided */
    if (DLT_IS_HTYP_UEH(msg.standardheader->htyp))
    {
        /* with extended header */
        msg.extendedheader = (DltExtendedHeader*)(msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp)  );

        switch (mtype)
        {
        case DLT_TYPE_LOG:
        {
            msg.extendedheader->msin = (DLT_TYPE_LOG << DLT_MSIN_MSTP_SHIFT) | ((log->log_level << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN) ; /* messsage info */
            break;
        }
        case DLT_TYPE_NW_TRACE:
        {
            msg.extendedheader->msin = (DLT_TYPE_NW_TRACE << DLT_MSIN_MSTP_SHIFT) | ((log->trace_status << DLT_MSIN_MTIN_SHIFT) & DLT_MSIN_MTIN) ; /* messsage info */
            break;
        }
        default:
        {
        	    /* This case should not occur */
            return DLT_RETURN_ERROR;
            break;
        }
        }

        /* If in verbose mode, set flag in header for verbose mode */
        if (dlt_user.verbose_mode)
        {
            msg.extendedheader->msin |= DLT_MSIN_VERB;
        }

        msg.extendedheader->noar = log->args_num;              /* number of arguments */
        dlt_set_id(msg.extendedheader->apid,dlt_user.appID);       /* application id */
        dlt_set_id(msg.extendedheader->ctid,log->handle->contextID);   /* context id */

        msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    }
    else
    {
        /* without extended header */
        msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + DLT_STANDARD_HEADER_EXTRA_SIZE(msg.standardheader->htyp);
    }

    len=msg.headersize - sizeof(DltStorageHeader) +log->size;
    if (len>UINT16_MAX)
    {
        dlt_log(LOG_CRIT,"Huge message discarded!\n");
        return DLT_RETURN_ERROR;
    }

    msg.standardheader->len = DLT_HTOBE_16(len);

    /* print to std out, if enabled */
    if ((dlt_user.local_print_mode != DLT_PM_FORCE_OFF) &&
            (dlt_user.local_print_mode != DLT_PM_AUTOMATIC))
    {
        if ((dlt_user.enable_local_print) || (dlt_user.local_print_mode == DLT_PM_FORCE_ON))
        {
            if (dlt_user_print_msg(&msg, log)==-1)
            {
				return DLT_RETURN_ERROR;
            }
        }
    }

    if (dlt_user.dlt_is_file)
    {
        /* log to file */
        ret=dlt_user_log_out2(dlt_user.dlt_log_handle, msg.headerbuffer, msg.headersize, log->buffer, log->size);
        return ret;
    }
    else
    {
        /* Reattach to daemon if neccesary */
        dlt_user_log_reattach_to_daemon();

        if (dlt_user.overflow)
        {
            if (dlt_user_log_send_overflow()==0)
            {
                dlt_user.overflow=0;
            }
        }

		/* try to resent old data first */
		ret = DLT_RETURN_OK;
		if(dlt_user.dlt_log_handle!=-1)
			ret = dlt_user_log_resend_buffer();
		if(ret==DLT_RETURN_OK)
		{
			/* resend ok or nothing to resent */
#ifdef DLT_SHM_ENABLE
			if(dlt_user.dlt_log_handle!=-1)
				dlt_shm_push(&dlt_user.dlt_shm,msg.headerbuffer+sizeof(DltStorageHeader), msg.headersize-sizeof(DltStorageHeader),
											log->buffer, log->size,0,0);                   

			/* log to FIFO */
			ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
									&(userheader), sizeof(DltUserHeader),
									0, 0,
									0, 0);
#else
			/* log to FIFO */
#ifdef DLT_TEST_ENABLE
			if(dlt_user.corrupt_user_header) {
				userheader.pattern[0]=0xff;
				userheader.pattern[1]=0xff;
				userheader.pattern[2]=0xff;
				userheader.pattern[3]=0xff;
			}
			if(dlt_user.corrupt_message_size) {
				msg.standardheader->len = DLT_HTOBE_16(dlt_user.corrupt_message_size_size);
			}
#endif
			ret = dlt_user_log_out3(dlt_user.dlt_log_handle,
									&(userheader), sizeof(DltUserHeader),
									msg.headerbuffer+sizeof(DltStorageHeader), msg.headersize-sizeof(DltStorageHeader),
									log->buffer, log->size);		
#endif        		
		}
		
        /* store message in ringbuffer, if an error has occured */
        if (ret!=DLT_RETURN_OK)
        {
            DLT_SEM_LOCK();

            if (dlt_buffer_push3(&(dlt_user.startup_buffer),
                                (unsigned char *)&(userheader), sizeof(DltUserHeader),
                                msg.headerbuffer+sizeof(DltStorageHeader), msg.headersize-sizeof(DltStorageHeader),
                                log->buffer, log->size)==-1)
			{
				dlt_log(LOG_ERR,"Storing message to history buffer failed! Message discarded.\n");
				ret = DLT_RETURN_BUFFER_FULL;
			}

            DLT_SEM_FREE();

        	// Fail silenty if FIFO is not open
            if(dlt_user_queue_resend() < 0 && dlt_user.dlt_log_handle >= 0)
            {
            	dlt_log(LOG_WARNING, "Failed to queue resending.\n");
            }
        }

        switch (ret)
        {
        case DLT_RETURN_BUFFER_FULL:
        {
        	/* Buffer full */
        	return DLT_RETURN_BUFFER_FULL;
        }
        case DLT_RETURN_PIPE_FULL:
        {
            /* data could not be written */
            dlt_user.overflow = 1;
            return DLT_RETURN_PIPE_FULL;
        }
        case DLT_RETURN_PIPE_ERROR:
        {
            /* handle not open or pipe error */
            close(dlt_user.dlt_log_handle);
            dlt_user.dlt_log_handle = -1;

#ifdef DLT_SHM_ENABLE
			/* free shared memory */
			dlt_shm_free_client(&dlt_user.dlt_shm);
#endif

            if (dlt_user.local_print_mode == DLT_PM_AUTOMATIC)
            {
                dlt_user_print_msg(&msg, log);
            }

            return DLT_RETURN_PIPE_ERROR;
        }
        case DLT_RETURN_ERROR:
        {
            /* other error condition */
            return DLT_RETURN_ERROR;
        }
		case DLT_RETURN_OK:
        {
        	return DLT_RETURN_OK;
        }
		default:
		{
			/* This case should never occur. */
			return DLT_RETURN_ERROR;
		}
        }
    }

    return DLT_RETURN_OK;
}

int dlt_user_log_send_register_application(void)
{
    DltUserHeader userheader;
    DltUserControlMsgRegisterApplication usercontext;

    DltReturnValue ret;

    if (dlt_user.appID[0]=='\0')
    {
        return -1;
    }

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_REGISTER_APPLICATION)==-1)
    {
		return -1;
    }

    /* set usercontext */
    dlt_set_id(usercontext.apid,dlt_user.appID);       /* application id */
    usercontext.pid = getpid();

    if (dlt_user.application_description!=0)
    {
        usercontext.description_length = strlen(dlt_user.application_description);
    }
    else
    {
        usercontext.description_length = 0;
    }

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out3(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(usercontext), sizeof(DltUserControlMsgRegisterApplication),dlt_user.application_description,usercontext.description_length);

    /* store message in ringbuffer, if an error has occured */
    if (ret!=DLT_RETURN_OK)
    {
        DLT_SEM_LOCK();

        if (dlt_buffer_push3(&(dlt_user.startup_buffer),
                            (unsigned char *)&(userheader), sizeof(DltUserHeader),
                            (const unsigned char*)&(usercontext), sizeof(DltUserControlMsgRegisterApplication),
                            (const unsigned char*)dlt_user.application_description, usercontext.description_length)==-1)
             {
                    dlt_log(LOG_ERR,"Storing message to history buffer failed! Message discarded.\n");
                    DLT_SEM_FREE();
                    return -1;
             }

        DLT_SEM_FREE();

        if(dlt_user_queue_resend() < 0 && dlt_user.dlt_log_handle >= 0)
        {
        	dlt_log(LOG_WARNING, "Failed to queue resending.\n");
        }
    }

    return 0;
}

int dlt_user_log_send_unregister_application(void)
{
    DltUserHeader userheader;
    DltUserControlMsgUnregisterApplication usercontext;

    DltReturnValue ret;

    if (dlt_user.appID[0]=='\0')
    {
        return -1;
    }

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_UNREGISTER_APPLICATION)==-1)
    {
    	return -1;
    }

    /* set usercontext */
    dlt_set_id(usercontext.apid,dlt_user.appID);       /* application id */
    usercontext.pid = getpid();

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out2(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(usercontext), sizeof(DltUserControlMsgUnregisterApplication));
    return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_user_log_send_register_context(DltContextData *log)
{
    DltUserHeader userheader;
    DltUserControlMsgRegisterContext usercontext;
    DltReturnValue ret;

    if (log==0)
    {
		return -1;
    }

    if (log->handle==0)
    {
        return -1;
    }

    if ((dlt_user.appID[0]=='\0') || (log->handle->contextID=='\0'))
    {
        return -1;
    }

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_REGISTER_CONTEXT)==-1)
    {
		return -1;
    }

    /* set usercontext */
    dlt_set_id(usercontext.apid,dlt_user.appID);       /* application id */
    dlt_set_id(usercontext.ctid,log->handle->contextID);       /* context id */
    usercontext.log_level_pos = log->handle->log_level_pos;
    usercontext.pid = getpid();

    usercontext.log_level = (int8_t)log->log_level;
    usercontext.trace_status = (int8_t)log->trace_status;

    if (log->context_description!=0)
    {
    	usercontext.description_length = strlen(log->context_description);
    }
    else
    {
		usercontext.description_length = 0;
    }

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out3(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(usercontext), sizeof(DltUserControlMsgRegisterContext),log->context_description,usercontext.description_length);

    /* store message in ringbuffer, if an error has occured */
    if (ret!=DLT_RETURN_OK)
    {
        DLT_SEM_LOCK();

        if (dlt_buffer_push3(&(dlt_user.startup_buffer),
                            (unsigned char *)&(userheader), sizeof(DltUserHeader),
                            (const unsigned char*)&(usercontext), sizeof(DltUserControlMsgRegisterContext),
                            (const unsigned char*)log->context_description, usercontext.description_length)==-1)
             {
                    dlt_log(LOG_ERR,"Storing message to history buffer failed! Message discarded.\n");
                    DLT_SEM_FREE();
                    return -1;
             }

        DLT_SEM_FREE();

        if(dlt_user_queue_resend() < 0 && dlt_user.dlt_log_handle >= 0)
        {
        	dlt_log(LOG_WARNING, "Failed to queue resending.\n");
        }
    }

    return 0;

}

int dlt_user_log_send_unregister_context(DltContextData *log)
{
    DltUserHeader userheader;
    DltUserControlMsgUnregisterContext usercontext;
    DltReturnValue ret;

    if (log==0)
    {
        return -1;
    }

    if (log->handle==0)
    {
        return -1;
    }

    if ((dlt_user.appID[0]=='\0') || (log->handle->contextID=='\0'))
    {
    	return -1;
    }

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_UNREGISTER_CONTEXT)==-1)
    {
		return -1;
    }

    /* set usercontext */
    dlt_set_id(usercontext.apid,dlt_user.appID);       /* application id */
    dlt_set_id(usercontext.ctid,log->handle->contextID);       /* context id */
    usercontext.pid = getpid();

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out2(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(usercontext), sizeof(DltUserControlMsgUnregisterContext));
	return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_send_app_ll_ts_limit(const char *appid, DltLogLevelType loglevel, DltTraceStatusType tracestatus)
{
    DltUserHeader userheader;
    DltUserControlMsgAppLogLevelTraceStatus usercontext;
	DltReturnValue ret;

    if ((appid==0) || (appid[0]=='\0'))
    {
        return -1;
    }

    /* Removed because of DltLogLevelType and DltTraceStatusType

    if ((loglevel<DLT_LOG_DEFAULT) || (loglevel>DLT_LOG_VERBOSE))
    {
        return -1;
    }

    if ((tracestatus<DLT_TRACE_STATUS_DEFAULT) || (tracestatus>DLT_TRACE_STATUS_ON))
    {
        return -1;
    }

    */

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_APP_LL_TS)==-1)
    {
		return -1;
    }

    /* set usercontext */
    dlt_set_id(usercontext.apid,appid);       /* application id */
    usercontext.log_level = loglevel;
    usercontext.trace_status = tracestatus;

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out2(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(usercontext), sizeof(DltUserControlMsgAppLogLevelTraceStatus));
    return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_user_log_send_log_mode(DltUserLogMode mode)
{
    DltUserHeader userheader;
    DltUserControlMsgLogMode logmode;

    DltReturnValue ret;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_LOG_MODE)==-1)
    {
    	return -1;
    }

    /* set data */
    logmode.log_mode = (unsigned char) mode;

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out2(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), &(logmode), sizeof(DltUserControlMsgLogMode));
    return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_user_print_msg(DltMessage *msg, DltContextData *log)
{
    uint8_t *databuffer_tmp;
    int32_t datasize_tmp;
    int32_t databuffersize_tmp;
    static char text[DLT_USER_TEXT_LENGTH];

    if ((msg==0) || (log==0))
    {
        return -1;
    }

    /* Save variables before print */
    databuffer_tmp = msg->databuffer;
    datasize_tmp = msg->datasize;
    databuffersize_tmp = msg->databuffersize;

    /* Act like a receiver, convert header back to host format */
    msg->standardheader->len = DLT_BETOH_16(msg->standardheader->len);
    dlt_message_get_extraparameters(msg,0);

    msg->databuffer = log->buffer;
    msg->datasize = log->size;
    msg->databuffersize = log->size;

    /* Print message as ASCII */
    if (dlt_message_print_ascii(msg,text,DLT_USER_TEXT_LENGTH,0)==-1)
    {
		return -1;
    }

    /* Restore variables and set len to BE*/
    msg->databuffer = databuffer_tmp;
    msg->databuffersize = databuffersize_tmp;
    msg->datasize =  datasize_tmp;

    msg->standardheader->len = DLT_HTOBE_16(msg->standardheader->len);

    return 0;
}

int dlt_user_log_check_user_message(void)
{
    int offset=0;
    int leave_while=0;

    uint32_t i;

    DltUserHeader *userheader;
    DltReceiver *receiver = &(dlt_user.receiver);

    DltUserControlMsgLogLevel *usercontextll;
    DltUserControlMsgInjection *usercontextinj;
    DltUserControlMsgLogState *userlogstate;
    unsigned char *userbuffer;

    /* For delayed calling of injection callback, to avoid deadlock */
    DltUserInjectionCallback 	delayed_injection_callback;
    unsigned char				*delayed_inject_buffer = 0;
    uint32_t					delayed_inject_data_length = 0;

    /* Ensure that callback is null before searching for it */
    delayed_injection_callback.injection_callback = 0;
    delayed_injection_callback.service_id = 0;

    if (dlt_user.dlt_user_handle!=DLT_FD_INIT)
    {
        while (1)
        {
            if (dlt_receiver_receive_fd(receiver)<=0)
            {
                /* No new message available */
                return 0;
            }

            /* look through buffer as long as data is in there */
            while (1)
            {
                if (receiver->bytesRcvd < (int32_t)sizeof(DltUserHeader))
                {
                    break;
                }

                /* resync if necessary */
                offset=0;
                do
                {
                    userheader = (DltUserHeader*) (receiver->buf+offset);

                    /* Check for user header pattern */
                    if (dlt_user_check_userheader(userheader))
                    {
                        break;
                    }
                    offset++;

                }
                while ((int32_t)(sizeof(DltUserHeader)+offset)<=receiver->bytesRcvd);

                /* Check for user header pattern */
                if (dlt_user_check_userheader(userheader)==0)
                {
                    break;
                }

                /* Set new start offset */
                if (offset>0)
                {
                    receiver->buf+=offset;
                    receiver->bytesRcvd-=offset;
                }

                switch (userheader->message)
                {
                case DLT_USER_MESSAGE_LOG_LEVEL:
                {
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogLevel)))
                    {
                        leave_while=1;
                        break;
                    }

                    usercontextll = (DltUserControlMsgLogLevel*) (receiver->buf+sizeof(DltUserHeader));

                    /* Update log level and trace status */
                    if (usercontextll!=0)
                    {
                        DLT_SEM_LOCK();

                        if ((usercontextll->log_level_pos >= 0) && (usercontextll->log_level_pos < (int32_t)dlt_user.dlt_ll_ts_num_entries))
                        {
                            // printf("Store ll, ts\n");
                            if (dlt_user.dlt_ll_ts)
                            {
                                dlt_user.dlt_ll_ts[usercontextll->log_level_pos].log_level = usercontextll->log_level;
                                dlt_user.dlt_ll_ts[usercontextll->log_level_pos].trace_status = usercontextll->trace_status;
                            }
                        }

                        DLT_SEM_FREE();
                    }

                    /* keep not read data in buffer */
                    if (dlt_receiver_remove(receiver,sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogLevel))==-1)
                    {
                    	return -1;
                    }
                }
                break;
                case DLT_USER_MESSAGE_INJECTION:
                {
                    /* At least, user header, user context, and service id and data_length of injected message is available */
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgInjection)))
                    {
                        leave_while = 1;
                        break;
                    }

                    usercontextinj = (DltUserControlMsgInjection*) (receiver->buf+sizeof(DltUserHeader));
                    userbuffer = (unsigned char*) (receiver->buf+sizeof(DltUserHeader)+sizeof(DltUserControlMsgInjection));

                    if (userbuffer!=0)
                    {

                        if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgInjection)+usercontextinj->data_length_inject))
                        {
                            leave_while = 1;
                            break;
                        }

                        DLT_SEM_LOCK();

                        if ((usercontextinj->data_length_inject>0) && (dlt_user.dlt_ll_ts))
                        {
                            /* Check if injection callback is registered for this context */
                            for (i=0; i<dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].nrcallbacks;i++)
                            {
                                if ((dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table) &&
                                        (dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].service_id == usercontextinj->service_id))
                                {
                                	/* Prepare delayed injection callback call */
									if (dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].injection_callback!=0)
									{
										delayed_injection_callback.injection_callback = dlt_user.dlt_ll_ts[usercontextinj->log_level_pos].injection_table[i].injection_callback;
										delayed_injection_callback.service_id = usercontextinj->service_id;
										delayed_inject_data_length = usercontextinj->data_length_inject;
										delayed_inject_buffer = malloc(delayed_inject_data_length);
										if(delayed_inject_buffer != 0) {
											memcpy(delayed_inject_buffer, userbuffer, delayed_inject_data_length);
										}

									}
                                    break;
                                }
                            }
                        }

                        DLT_SEM_FREE();

                        /* Delayed injection callback call */
                        if(delayed_inject_buffer != 0 && delayed_injection_callback.injection_callback != 0) {
                        	delayed_injection_callback.injection_callback(delayed_injection_callback.service_id, delayed_inject_buffer, delayed_inject_data_length);
                        	free(delayed_inject_buffer);
                        }

                        /* keep not read data in buffer */
                        if (dlt_receiver_remove(receiver,(sizeof(DltUserHeader)+sizeof(DltUserControlMsgInjection)+usercontextinj->data_length_inject))==-1)
						{
							return -1;
						}
                    }
                }
                break;
                case DLT_USER_MESSAGE_LOG_STATE:
                {
                    /* At least, user header, user context, and service id and data_length of injected message is available */
                    if (receiver->bytesRcvd < (int32_t)(sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogState)))
                    {
                        leave_while = 1;
                        break;
                    }

                    userlogstate = (DltUserControlMsgLogState*) (receiver->buf+sizeof(DltUserHeader));
					dlt_user.log_state = userlogstate->log_state;

					/* keep not read data in buffer */
					if (dlt_receiver_remove(receiver,(sizeof(DltUserHeader)+sizeof(DltUserControlMsgLogState)))==-1)
					{
						return -1;
					}
                }
                break;
                default:
                {
                    dlt_log(LOG_ERR,"Invalid user message type received!\n");
                    /* Ignore result */
                    dlt_receiver_remove(receiver,sizeof(DltUserHeader));
					/* In next invocation of while loop, a resync will be triggered if additional data was received */
                }
                break;
                } /* switch() */

                if (leave_while==1)
                {
                    leave_while=0;
                    break;
                }

            } /* while buffer*/

            if (dlt_receiver_move_to_begin(receiver)==-1)
            {
				return -1;
            }
        } /* while receive */
    } /* if */

    return DLT_RETURN_OK;
}

int dlt_user_log_resend_buffer(void)
{
	int num,count;
    int size;
	DltReturnValue ret;
	
	/* Send content of ringbuffer */
	DLT_SEM_LOCK();
	count = dlt_buffer_get_message_count(&(dlt_user.startup_buffer));
	DLT_SEM_FREE();

	for (num=0;num<count;num++)
	{

		DLT_SEM_LOCK();
		size = dlt_buffer_copy(&(dlt_user.startup_buffer),dlt_user.resend_buffer,sizeof(dlt_user.resend_buffer));

		if (size>0)
		{
#ifdef DLT_SHM_ENABLE						
			dlt_shm_push(&dlt_user.dlt_shm,dlt_user.resend_buffer+sizeof(DltUserHeader),size-sizeof(DltUserHeader),0,0,0,0);

			/* log to FIFO */
			ret = dlt_user_log_out3(dlt_user.dlt_log_handle, dlt_user.resend_buffer,sizeof(DltUserHeader),0,0,0,0);
#else
			/* log to FIFO */
			ret = dlt_user_log_out3(dlt_user.dlt_log_handle, dlt_user.resend_buffer,size,0,0,0,0);
#endif

			/* in case of error, keep message in ringbuffer */                        
			if (ret==DLT_RETURN_OK)
			{
				dlt_buffer_remove(&(dlt_user.startup_buffer));
			}
			else
			{
				/* keep message in ringbuffer */   
				DLT_SEM_FREE();
				return -1;
			}
		}
		DLT_SEM_FREE();
	}
	
	return 0;
}

void dlt_user_log_reattach_to_daemon(void)
{
	uint32_t num,reregistered=0;

	DltContext handle;
	DltContextData log_new;

    if (dlt_user.dlt_log_handle<0)
    {
        dlt_user.dlt_log_handle=-1;

        /* try to open pipe to dlt daemon */
        dlt_user.dlt_log_handle = open(DLT_USER_FIFO, O_WRONLY | O_NONBLOCK);
        if (dlt_user.dlt_log_handle > 0)
        {
            if (dlt_user_log_init(&handle,&log_new)==-1)
            {
            	return;
            }

#ifdef DLT_SHM_ENABLE
			/* init shared memory */
			if (dlt_shm_init_client(&dlt_user.dlt_shm,DLT_SHM_KEY) < 0)
			{
				sprintf(str,"Loging disabled, Shared memory %d cannot be created!\n",DLT_SHM_KEY);
				dlt_log(LOG_WARNING, str);
				//return 0; 
			}   
#endif

            dlt_log(LOG_NOTICE, "Logging re-enabled!\n");

            /* Re-register application */
            if (dlt_user_log_send_register_application()==-1)
            {
            	return;
            }

            DLT_SEM_LOCK();

            /* Re-register all stored contexts */
            for (num=0; num<dlt_user.dlt_ll_ts_num_entries; num++)
            {
                /* Re-register stored context */
                if ((dlt_user.appID[0]!='\0') && (dlt_user.dlt_ll_ts[num].contextID[0]!='\0') && (dlt_user.dlt_ll_ts))
                {
                    //dlt_set_id(log_new.appID, dlt_user.appID);
                    dlt_set_id(handle.contextID, dlt_user.dlt_ll_ts[num].contextID);
                    handle.log_level_pos = num;
                    log_new.context_description = dlt_user.dlt_ll_ts[num].context_description;

                    log_new.log_level = DLT_USER_LOG_LEVEL_NOT_SET;
                    log_new.trace_status = DLT_USER_TRACE_STATUS_NOT_SET;

                    if (dlt_user_log_send_register_context(&log_new)==-1)
                    {
                    	DLT_SEM_FREE();
                    	return;
                    }

                    reregistered=1;
                }
            }

            DLT_SEM_FREE();

            if (reregistered==1)
            {
				dlt_user_log_resend_buffer();
            }
        }
    }
}

int dlt_user_log_send_overflow(void)
{
    DltUserHeader userheader;
    DltReturnValue ret;

    /* set userheader */
    if (dlt_user_set_userheader(&userheader, DLT_USER_MESSAGE_OVERFLOW)==-1)
    {
    	return -1;
    }

    if (dlt_user.dlt_is_file)
    {
        return 0;
    }

    /* log to FIFO */
    ret=dlt_user_log_out2(dlt_user.dlt_log_handle, &(userheader), sizeof(DltUserHeader), 0, 0);
    return ((ret==DLT_RETURN_OK)?0:-1);
}

int dlt_user_check_buffer(int *total_size, int *used_size)
{
#ifdef DLT_SHM_ENABLE
	*total_size = dlt_shm_get_total_size(&(dlt_user.dlt_shm));
	*used_size = dlt_shm_get_used_size(&(dlt_user.dlt_shm));
#else
	*total_size = dlt_buffer_get_total_size(&(dlt_user.startup_buffer));
	*used_size = dlt_buffer_get_used_size(&(dlt_user.startup_buffer));
#endif
	
	return 0; /* ok */
}

#ifdef DLT_TEST_ENABLE
void dlt_user_test_corrupt_user_header(int enable)
{
    dlt_user.corrupt_user_header = enable;
}
void dlt_user_test_corrupt_message_size(int enable,int16_t size)
{
    dlt_user.corrupt_message_size = enable;
    dlt_user.corrupt_message_size_size = size;
}
#endif


