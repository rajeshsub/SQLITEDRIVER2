/*
===============================================================================
Copyright (c) 2017, Commonwealth Scientific and Industrial Research
Organisation (CSIRO) ABN 41 687 119 230.

All rights reserved. Except where otherwise indicated for third party material,
CSIRO is willing to grant you a license to CSIRO SQLiteDriver2 on the terms of the
GNU Lesser General Public License version 2.1 as published by the Free Software
Foundation (https://www.gnu.org/licenses/lgpl-2.1.html) (LGPL V2.1) and
appearing in the file [LICENSE.LGPLv2.1] included in the packaging of this file.

In addition to the terms of the LGPL v2.1, CSIRO provides the following
additional rights where applicable:
APPLICABLE LEGISLATION SUCH AS THE AUSTRALIAN CONSUMER LAW MAY APPLY
REPRESENTATIONS, WARRANTIES, OR CONDITIONS, OR IMPOSES OBLIGATIONS OR LIABILITY
ON CSIRO THAT CANNOT BE EXCLUDED, RESTRICTED OR MODIFIED TO THE FULL EXTENT SET
OUT IN THE EXPRESS TERMS OF THIS CLAUSE ABOVE "CONSUMER GUARANTEES".  TO THE
EXTENT THAT SUCH CONSUMER GUARANTEES CONTINUE TO APPLY, THEN TO THE FULL EXTENT
PERMITTED BY THE APPLICABLE LEGISLATION, THE LIABILITY OF CSIRO UNDER THE
RELEVANT CONSUMER GUARANTEE IS LIMITED (WHERE PERMITTED AT CSIRO'S OPTION)
TO ONE OF FOLLOWING REMEDIES OR SUBSTANTIALLY EQUIVALENT REMEDIES:
(a) THE REPLACEMENT OF THE SOFTWARE, THE SUPPLY OF EQUIVALENT SOFTWARE,
OR SUPPLYING RELEVANT SERVICES AGAIN;
(b) THE REPAIR OF THE SOFTWARE;
(c) THE PAYMENT OF THE COST OF REPLACING THE SOFTWARE, OF ACQUIRING EQUIVALENT
SOFTWARE, HAVING THE RELEVANT SERVICES SUPPLIED AGAIN, OR HAVING THE
SOFTWARE REPAIRED.
IN THIS CLAUSE, CSIRO INCLUDES ANY THIRD PARTY AUTHOR OR OWNER OF ANY PART OF
THE SOFTWARE OR MATERIAL DISTRIBUTED WITH IT.  CSIRO MAY ENFORCE ANY RIGHTS
ON BEHALF OF THE RELEVANT THIRD PARTY.

Third Party Components
The following third party components are distributed with the Software.  You
agree to comply with the license terms for these components as part of
accessing the Software.  Other third party software may also be identified in
separate files distributed with the Software.

SQLite https://www.sqlite.org/index.html
Contact: https://www.sqlite.org/support.html
All of the code and documentation in SQLite has been dedicated to the public
domain by the authors. All code authors, and representatives of the companies
they work for, have signed affidavits dedicating their contributions to the
public domain and originals of those signed affidavits are stored in a firesafe
at the main offices of Hwaci. Anyone is free to copy, modify, publish, use,
compile, sell, or distribute the original SQLite code, either in source code
form or as a compiled binary, for any purpose, commercial or non-commercial,
and by any means.
===============================================================================
*/

#include <QString>
#include "sqlitedriver2_functions.h"

namespace SQLITE_CUSTOM_FUNCTIONS
{

// If bCheckMinimumArgsOnly is set to false (which is false by default):
// Verifies that the number of arguments supplied exactly equals the expected number of arguments.
// If this is correct, it returns true. Returns false otherwise.

// If bCheckMinimumArgsOnly is set to true:
// It checks to see that the number of arguments supplied
// is at least a minimum of numReqArgs, and returns true if so. Returns false otherwise.
bool checkNumOfArgs(const QString& funcName, const int numReqArgs, const int argc, sqlite3_context *context, const bool bCheckMinimumArgsOnly /*=false*/)
{
    if (bCheckMinimumArgsOnly && numReqArgs >= argc)
    {
        return true;
    }
    else if(numReqArgs == argc)
    {
        return true;
    }

    QString sError;
    sError = QString("%1 requires %2 argument(s). Number of argument(s) supplied: %3").arg(funcName).arg(numReqArgs).arg(argc);
    sqlite3_result_error(context, sError.toStdString().c_str(), errno);

    return false;
}

// Checks that each argument supplied is from a set of sqlite3_value that is considered valid
// If a supplied argument isn't present in the set, sets an error and returns false
// Returns true otherwise
static bool checkTypesOfArgs(const QString& funcName, const int argc, sqlite3_value **argv, const std::unordered_set<int>& validTypes, sqlite3_context *context)
{
    for (auto i = 0; i < argc; i++)
    {
        switch (sqlite3_value_type(argv[i]))
        {
            case SQLITE_NULL: //This case is handled specially so that the error could be more concise.
                {
                    //in some cases, NULL may be an acceptable input
                    if (validTypes.count(SQLITE_NULL))
                    {
                        break;
                    }
                    else
                    {
                        //If NULL is not an acceptable input type, then set an error and return
                        QString sError(QString("NULL was supplied as an input for %1").arg(funcName));
                        sqlite3_result_error(context, sError.toStdString().c_str(), errno);
                        return false;
                    }
                }
            default:
                {
                    if (validTypes.count(sqlite3_value_type(argv[i]))) //if this argument type is valid, then move on to check the next one
                    {
                        break;
                    }
                    else
                    {
                        QString sError(QString("Invalid input type supplied for %1").arg(funcName));
                        sqlite3_result_error(context, sError.toStdString().c_str(), errno);
                        return false;
                    }
                }
        }
    }
    return true;
}

//Maths functions
void pow(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 2;
    static const QString funcName("pow");
    static const std::unordered_set<int> validTypes { SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double val = 0.0;
        errno = 0;

        val = std::pow(sqlite3_value_double(argv[0]), sqlite3_value_double(argv[1]));
        if (errno == 0)
        {
            sqlite3_result_double(context, val);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void sqrt(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("sqrt");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::sqrt(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void max(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int minRequiredArgs = 1;
    static const QString funcName("max");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, minRequiredArgs, argc, context, true) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        switch (sqlite3_value_type(argv[0]))
        {
            case SQLITE_INTEGER:
                {
                    auto maxVal = sqlite3_value_int64(argv[0]);
                    for (int i = 0; i < argc; i++)
                    {
                        if (sqlite3_value_int64(argv[i]) > maxVal)
                        {
                            maxVal = sqlite3_value_int64(argv[i]);
                        }
                    }
                    sqlite3_result_int64(context, maxVal);
                    break;
                }
            case SQLITE_FLOAT:
                {
                    double maxVal = sqlite3_value_double(argv[0]);
                    for (int i = 0; i < argc; i++)
                    {
                        if (sqlite3_value_double(argv[i]) > maxVal)
                        {
                            maxVal = sqlite3_value_double(argv[i]);
                        }
                    }
                    sqlite3_result_double(context, maxVal);
                    break;
                }
        }
    }
}

void min(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int minRequiredArgs = 1;
    static const QString funcName("min");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, minRequiredArgs, argc, context, true) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        switch (sqlite3_value_type(argv[0]))
        {
            case SQLITE_INTEGER:
                {
                    qint64 minVal = sqlite3_value_int64(argv[0]);
                    for (int i = 0; i < argc; i++)
                    {
                        if (sqlite3_value_int64(argv[i]) < minVal)
                        {
                            minVal = sqlite3_value_int64(argv[i]);
                        }
                    }
                    sqlite3_result_int64(context, minVal);
                    break;
                }
            case SQLITE_FLOAT:
                {
                    double minVal = sqlite3_value_double(argv[0]);
                    for (int i = 0; i < argc; i++)
                    {
                        if (sqlite3_value_double(argv[i]) < minVal)
                        {
                            minVal = sqlite3_value_double(argv[i]);
                        }
                    }
                    sqlite3_result_double(context, minVal);
                    break;
                }
        }
    }
}

void ceil(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("ceil");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::ceil(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void floor(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("floor");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::floor(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void cos(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("cos");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::cos(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void sin(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("sin");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
            double result = 0;
            errno = 0;
            result = std::sin(sqlite3_value_double(argv[0]));
            if (0 == errno)
            {
                sqlite3_result_double(context, result);
            }
            else
            {
                sqlite3_result_error(context, strerror(errno), errno);
            }
    }
}

void tan(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("tan");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::tan(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void acos(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("acos");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::acos(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void asin(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("asin");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::asin(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void atan(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("atan");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::atan(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void atan2(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 2;
    static const QString funcName("atan2");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::atan2(sqlite3_value_double(argv[0]), sqlite3_value_double(argv[1]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void cosh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("cosh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::cosh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void sinh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("sinh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (!checkNumOfArgs(funcName, numRequiredArgs, argc, context))
    {
        double result = 0;

        errno = 0;
        result = std::sinh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void tanh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("tanh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::tanh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void acosh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("acosh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::acosh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void asinh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("asinh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::asinh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void atanh(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("atanh");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::atanh(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void exp(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("exp");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::exp(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void log(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("log");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::log(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void log10(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("log10");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::log10(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void log2(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("log2");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::log2(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void logb(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("logb");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::logb(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void cbrt(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("cbrt");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::cbrt(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void erf(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("erf");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::erf(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void erfc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("erfc");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::erfc(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void tgamma(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("tgamma");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::tgamma(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void lgamma(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("lgamma");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::lgamma(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void hypot(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 2;
    static const QString funcName("hypot");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::hypot(sqlite3_value_double(argv[0]), sqlite3_value_double(argv[1]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void abs(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("abs");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::abs(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void fabs(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 1;
    static const QString funcName("fabs");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double result = 0;
        errno = 0;
        result = std::fabs(sqlite3_value_double(argv[0]));
        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

void dist(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    static const int numRequiredArgs = 4;
    static const QString funcName("dist");
    static const std::unordered_set<int> validTypes{ SQLITE_FLOAT, SQLITE_INTEGER };

    if (checkNumOfArgs(funcName, numRequiredArgs, argc, context) && checkTypesOfArgs(funcName, argc, argv, validTypes, context))
    {
        double x1 = 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = 0;
        double result = 0;
        short i = 4;
        while (i--)
        {
            if (sqlite3_value_type(argv[i]) == SQLITE_NULL)
            {
                sqlite3_result_null(context);
                return;
            }
        }
        x1 = sqlite3_value_double(argv[0]);
        y1 = sqlite3_value_double(argv[1]);
        x2 = sqlite3_value_double(argv[2]);
        y2 = sqlite3_value_double(argv[3]);

        errno = 0;

        result = std::sqrt((x2 - x1)* (x2 - x1) + (y2 - y1) * (y2 - y1));

        if (0 == errno)
        {
            sqlite3_result_double(context, result);
        }
        else
        {
            sqlite3_result_error(context, strerror(errno), errno);
        }
    }
}

} //namespace SQLITE_CUSTOM_FUNCTIONS