#######
# Dlt - Diagnostic Log and Trace
# @licence make begin@
#
# Copyright (C) 2011-2012, BMW AG - Alexander Wenzel <alexander.aw.wenzel@bmw.de>
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# @licence end@
########

[Unit]
Description=GENIVI DLT example user. Generate DLT messages and store them to file or send them to daemon.
Wants=dlt.service

[Service]
Type=Simple
User=genivi
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/dlt-example-user "Hallo from GENIVI DLT example user application"
LimitCORE=infinity