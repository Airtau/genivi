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
#include "Expression.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>  // memcpy

Expression::Expression(string name, Expression* parent)
: mName(name)
, mPreviousWord(parent)
, mFuncPtr(NULL)
{
}

void Expression::setVarValue(string value)
{
    mVarValue = value;
}

bool Expression::isVar()
{
    return mName[0] == '<';
}

string Expression::getString(string name)
{
    string varName;
    varName += "<";
    varName += name;
    varName += ">";

    if (mName != varName)
    {
        if (mPreviousWord)
        {
            return mPreviousWord->getString(name);
        }
        else
        {
            return "";
        }
    }
    return mVarValue;
}

unsigned int Expression::getUint(string name)
{
    string stringVal = getString(name);

    unsigned int value = 0;
    sscanf(stringVal.c_str(), "%u", &value);

    if (!value)
    {
        sscanf(stringVal.c_str(), "0x%x", &value);
    }
    return value;
}

void Expression::getUintArray(string name, unsigned int** array, unsigned int* count)
{
    stringstream ss;
    ss << getString(name);

    unsigned int buffer[256]; // more than enough for all cases
    *count = 0;

    string stringVal;
    while (getline( ss, stringVal, ',' ))
    {
        sscanf(stringVal.c_str(), "%u", &buffer[*count]);

        if (!buffer[*count])
        {
            sscanf(stringVal.c_str(), "0x%x", &buffer[*count]);
        }
        ++(*count);
    }

    *array = new unsigned int[*count];
    memcpy(*array, buffer, sizeof(unsigned int) * (*count));
}

int Expression::getInt(string name)
{
    string stringVal = getString(name);

    int value = 0;
    sscanf(stringVal.c_str(), "%d", &value);

    if (!value)
    {
        sscanf(stringVal.c_str(), "0x%x", &value);
    }
    return value;
}

double Expression::getDouble(string name)
{
    string stringVal = getString(name);

    double value = 0;
    sscanf(stringVal.c_str(), "%lf", &value);
    return value;
}

bool Expression::getBool(string name)
{
    string stringVal = getString(name);
    int value = 0;
    return sscanf(stringVal.c_str(), "%d", &value) && value;
}

string Expression::getName()
{
    return mName;
}

bool ExpressionCompare(Expression* a, Expression* b)
{
    return a->getName() < b->getName();
}

void Expression::addNextExpression(Expression* word)
{
    mNextWords.push_back(word);
    mNextWords.sort(ExpressionCompare);
}

Expression* Expression::getNextExpression(string text)
{
    Expression* varMatch = NULL;
    Expression* nameMatch = NULL;

    ExpressionList::const_iterator iter = mNextWords.begin();
    ExpressionList::const_iterator end = mNextWords.end();
    for (; iter != end; ++iter)
    {
        Expression* expr = *iter;

        if (expr->getName() == text)
        {
            nameMatch = expr;
        }

        if (expr->isVar())
        {
            varMatch = expr;
            varMatch->setVarValue(text);
        }
    }

    return nameMatch ? nameMatch : (varMatch ? varMatch : NULL);
}

void Expression::printTree(int level)
{
    for (int i = 0; i < level; ++i)
    {
        cout << ((i + 1 != level) ? "|  " : "|--");
    }

    stringstream name;
    name << mName;

    if (isExecutable())
    {
        name << "*";
    }

    cout << name.str() << endl;

    ExpressionList::const_iterator iter = mNextWords.begin();
    ExpressionList::const_iterator end = mNextWords.end();
    for (; iter != end; ++iter)
    {
        (*iter)->printTree(level + 1);
    }
}

void Expression::printList(string list)
{
    if (mName != "[root]")
    {
        list += mName;
        list += " ";
        if (isExecutable())
        {
            cout << list << "\n";
        }
    }

    ExpressionList::const_iterator iter = mNextWords.begin();
    ExpressionList::const_iterator end = mNextWords.end();
    for (; iter != end; ++iter)
    {
        (*iter)->printList(list);
    }
}

bool Expression::isExecutable()
{
    return mFuncPtr;
}

void Expression::execute()
{
    (*mFuncPtr)(this);
}

void Expression::setFunc(callback funcPtr)
{
    mFuncPtr = funcPtr;
}

Expression* Expression::getPreviousExpression()
{
    return mPreviousWord;
}
