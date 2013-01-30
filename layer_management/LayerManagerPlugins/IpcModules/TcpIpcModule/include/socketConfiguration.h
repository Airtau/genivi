/***************************************************************************
 *
 * Copyright 2012 BMW Car IT GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef _TCPSOCKETCONFIGURATION_H_
#define _TCPSOCKETCONFIGURATION_H_

//=============================================================================
// tcp socket configuration
//=============================================================================
#define SOCKET_TCP_HOST                 "localhost"
#define SOCKET_TCP_PORT                 22232
#define SOCKET_MAX_MESSAGE_SIZE         1024
#define SOCKET_MAX_PENDING_CONNECTIONS  10

#define SOCKET_MESSAGE_TYPE_INT          'i'
#define SOCKET_MESSAGE_TYPE_UINT         'u'
#define SOCKET_MESSAGE_TYPE_BOOL         'b'
#define SOCKET_MESSAGE_TYPE_DOUBLE       'd'
#define SOCKET_MESSAGE_TYPE_STRING       's'
#define SOCKET_MESSAGE_TYPE_ARRAY        'a'

#define ENV_TCP_HOST                    "LM_TCP_HOST"
#define ENV_TCP_PORT                    "LM_TCP_PORT"

#endif // _TCPSOCKETCONFIGURATION_H_
