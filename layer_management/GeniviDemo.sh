#!/bin/sh
############################################################################
# 
# Copyright 2010, 2011 BMW Car IT GmbH
# 
# 
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License. 
# You may obtain a copy of the License at 
#
#       http://www.apache.org/licenses/LICENSE-2.0 
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
# See the License for the specific language governing permissions and 
# limitations under the License.
#
############################################################################

export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib

# set plugin path, if installation path is not used
#export LM_PLUGIN_PATH=/usr/local/lib/layermanager

export DISPLAY=:0.0
PIDSURFACE1=/tmp/surface1.pid
PIDSURFACE2=/tmp/surface2.pid
PIDLM=/tmp/surface3.pid

start_egl_example_application()
{
    EGLX11ApplicationExample &
    pidofdlt=`ps aux | grep EGLX11ApplicationExample | grep -v grep | awk '{print $2}'`
    echo $pidofdlt > $PIDSURFACE1
}

start_mock_example_application()
{
    EGLX11MockNavigation -layer 3000 -surface 20 &
    pidofdlt=`ps aux | grep EGLX11MockNavigation  | grep -v grep | awk '{print $2}'`
    echo $pidofdlt > $PIDSURFACE2
}


start_layermanager()
{
    LayerManagerService -c4 -f0 -w1280 -h480 &
    pidofdlt=`ps aux | grep LayerManagerService  | grep -v grep | awk '{print $2}'`
    echo $pidofdlt > $PIDLM
}

killprocess()
{
    if [ -f $1 ]; then
        kill -9 `cat $1`
        rm -f $1
    fi
}
stop() 
{
    killprocess $PIDSURFACE1
    killprocess $PIDSURFACE2
}

stop_layermanager() 
{
    killprocess $PIDLM
}


start()
{
    start_egl_example_application
    start_mock_example_application
}

case "$1" in
    start)
        start
        ;;
    start_lm)
        start_layermanager
        ;;
    start_example_application)
        start_example_application
        ;;
    stop)
        stop
        ;;
    stop_lm)
        stop_layermanager
        ;;       
    *)
        echo "Usage: $0 {start|start_lm|start_example_application|stop|stop_lm}"
        ;;
esac
exit 0
