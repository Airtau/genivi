Diagnostic Log and Trace  - Readme
==================================
Alexander Wenzel <Alexander.AW.Wenzel@bmw.de>

Overview
--------
This component provides a standardised log and trace interface, based on the
standardised protocol specified in the AUTOSAR standard 4.0 DLT.
This component can be used by GENIVI components and other applications as
logging facility providing

- the DLT shared library
- the DLT daemon
- the DLT daemon adaptors
- the DLT client console utilities
- the DLT test applications

The DLT daemon is the central component in GENIVI, which gathers all 
logs and traces from the DLT user applications. The logs and traces 
are stored optionally directly in a file in the ECU. The DLT daemon 
forwards all logs and traces to a connected DLT client.
The DLT client can send control messages to the daemon, e.g. to set 
individual log levels of applications and contexts or get the list of 
applications and contexts registered in the DLT daemon.

Documentation
-------------
- DLT Release Notes: ReleaseNotes.txt
- DLT Installation: INSTALL.txt
- DLT User Manual: doc/dlt_user_manual.txt
- DLT Cheatsheet: doc/dlt_cheatsheet.txt
- DLT Design Specification :doc/dlt_design_specification.txt

API Documentation
-----------------
See INSTALL.txt regarding doxygen API documentation generation.

Manpages
--------
- dlt-daemon(1)
- dlt.conf(5)
- dlt-system(1)
- dlt-system.conf(5)
- dlt-receive(1)
- dlt-convert(9)

Known issues
------------
- DLT library: Usage of dlt_user_log_write_float64() and DLT_FLOAT64() leads to "Illegal instruction (core dumped)" on ARM target.
- DLT library: Nested calls to DLT_LOG_ ... are not supported, and will lead to a deadlock.

Software/Hardware
-----------------
Developped and tested with Ubuntu Linux 12.04 32-bit / Intel PC

License
-------
Full information on the license for this software is available in the "LICENSE.txt" file. 

Source Code
-----------
git://git.projects.genivi.org/dlt-daemon.git
http://git.projects.genivi.org/dlt-daemon.git
ssh://git-genivi@git.projects.genivi.org/dlt-daemon.git

Homepage
--------
http://projects.genivi.org/diagnostic-log-trace

Mailinglist
-----------
https://lists.genivi.org/mailman/listinfo/genivi-diagnostic-log-and-trace

Contact
-------
Alexander Wenzel <Alexander.AW.Wenzel@bmw.de> +
Christian Muck <christian.muck@bmw.de>


