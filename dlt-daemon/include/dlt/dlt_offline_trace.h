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
 * \file dlt_offline_trace.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_offline_trace.h                                           **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
*******************************************************************************/

#ifndef DLT_OFFLINE_TRACE_H
#define DLT_OFFLINE_TRACE_H

typedef struct
{
    char directory[256]; /**< (String) Store DLT messages to local directory */
    char filename[256]; /**< (String) Filename of currently used log file */
    int  fileSize;	/**< (int) Maximum size in bytes of one trace file (Default: 1000000) */
    int  maxSize;	/**< (int) Maximum size of all trace files (Default: 4000000) */
    
    int ohandle;
} DltOfflineTrace;

/**
 * Initialise the offline trace
 * This function call opens the currently used log file.
 * A check of the complete size of the offline trace is done during startup.
 * Old files are deleted, if there is not enough space left to create new file.
 * This function must be called before using further offline trace functions.
 * @param trace pointer to offline trace structure
 * @param directory directory where to store offline trace files
 * @param fileSize maximum size of one offline trace file.
 * @param maxSize maximum size of complete offline trace in bytes.
 * @return negative value if there was an error
 */
extern int dlt_offline_trace_init(DltOfflineTrace *trace,const char *directory,int fileSize,int maxSize);

/**
 * Uninitialise the offline trace
 * This function call closes currently used log file.
 * This function must be called after usage of offline trace
 * @param trace pointer to offline trace structure
 * @return negative value if there was an error
 */
extern int dlt_offline_trace_free(DltOfflineTrace *buf);

/**
 * Write data into offline trace
 * If the current used log file exceeds the max file size, new log file is created. 
 * A check of the complete size of the offline trace is done before new file is created.
 * Old files are deleted, if there is not enough space left to create new file.
 * @param trace pointer to offline trace structure
 * @param data1 pointer to first data block to be written, null if not used
 * @param size1 size in bytes of first data block to be written, 0 if not used
 * @param data2 pointer to second data block to be written, null if not used
 * @param size2 size in bytes of second data block to be written, 0 if not used
 * @param data3 pointer to third data block to be written, null if not used
 * @param size3 size in bytes of third data block to be written, 0 if not used
 * @return negative value if there was an error
 */
extern int dlt_offline_trace_write(DltOfflineTrace *trace,unsigned char *data1,int size1,unsigned char *data2,int size2,unsigned char *data3,int size3);

/**
 * Get size of currently used offline trace buffer
 * @return size in bytes
 */
extern unsigned long dlt_offline_trace_get_total_size(DltOfflineTrace *trace);

#endif /* DLT_OFFLINE_TRACE_H */
