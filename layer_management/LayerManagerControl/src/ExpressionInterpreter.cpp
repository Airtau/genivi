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
#include "ExpressionInterpreter.h"
#include "Expression.h"
#include "ilm_client.h"
#include <string>
#include <sstream>
#include <algorithm> // transform
#include <ctype.h> // tolower

Expression* ExpressionInterpreter::mpRoot = NULL;

ExpressionInterpreter::ExpressionInterpreter()
: mErrorText("No error.")
{
}

bool ExpressionInterpreter::addExpression(callback funcPtr, string command)
{
    bool result = false;

    string text;
    stringstream ss;
    ss << command;

    if (!mpRoot)
    {
        mpRoot = new Expression("[root]", NULL);
    }

    Expression* currentWord = mpRoot;

    while (!ss.eof())
    {
        ss >> text;
        transform(text.begin(), text.end(), text.begin(), ::tolower);
        string name = currentWord->getName();

        Expression* nextWord = currentWord->getNextExpression(text);

        if (!nextWord)
        {
            nextWord = new Expression(text, currentWord);
            currentWord->addNextExpression(nextWord);
        }

        currentWord = nextWord;
    }

    currentWord->setFunc(funcPtr);

    return result;
}

CommandResult ExpressionInterpreter::interpretCommand(string userInput)
{
    CommandResult result = CommandSuccess;
    string text;
    stringstream ss;
    ss << userInput;

    Expression* currentWord = mpRoot;

    while (result == CommandSuccess && !ss.eof())
    {
        ss >> text;
        transform(text.begin(), text.end(), text.begin(), ::tolower);

        Expression* nextWord = currentWord->getNextExpression(text);

        if (nextWord)
        {
            currentWord = nextWord;
        }
        else
        {
            mErrorText = "'" + text + "' not recognized.";
            result = CommandInvalid;
        }
    }

    if (result == CommandSuccess)
    {
        if (currentWord->isExecutable())
        {
            if (ILM_SUCCESS != ilm_init())
            {
                mErrorText = "Could not connect to LayerManagerService.";
                result = CommandExecutionFailed;
            }

            else
            {
                currentWord->execute();
                ilm_destroy();
            }
        }
        else
        {
            mErrorText = "command is incomplete.";
            result = CommandIncomplete;
        }
    }
    return result;
}

void ExpressionInterpreter::printExpressionTree()
{
    mpRoot->printTree();
}

void ExpressionInterpreter::printExpressionList()
{
    mpRoot->printList();
}

string ExpressionInterpreter::getLastError()
{
    string tmp = mErrorText;
    mErrorText = "no error.";
    return tmp;
}
