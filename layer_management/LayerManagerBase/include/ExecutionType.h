/***************************************************************************
*
* Copyright 2010,2011 BMW Car IT GmbH
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

#ifndef EXECUTIONTYPES_H_
#define EXECUTIONTYPES_H_

enum ExecutionType
{
    ExecuteSynchronous = 0,
    ExecuteAsynchronous
};

enum ExecutionResult
{
    ExecutionFailed = 0,        // Execution failed, redraw
    ExecutionFailedRedraw = 1,  // Unsuccessful execution, command still causes screen to be redrawn
    ExecutionSuccess = 2,       // Successful execution, but no redraw is needed
    ExecutionSuccessRedraw = 3  // Successful execution, command causes screen to be redrawn
};

#endif /* EXECUTIONTYPES_H_ */
