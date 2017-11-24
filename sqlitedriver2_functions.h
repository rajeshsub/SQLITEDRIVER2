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

#ifndef SQLITE_CUSTOM_FUNCTIONS_H
#define SQLITE_CUSTOM_FUNCTIONS_H

#include <unordered_set>
#include <sqlite3.h>

namespace SQLITE_CUSTOM_FUNCTIONS
{
//helper functions
bool checkNumOfArgs(const QString& funcName, const int numReqArgs, const int numSuppliedArgs, sqlite3_context *context, const bool bCheckMinimumArgsOnly = false);
bool checkTypesOfArgs(const QString& funcName, const int argc, const sqlite3_value **argv, const std::unordered_set<int>& setOfValidTypes, sqlite3_context *context);

//maths functions
void pow(sqlite3_context *context, int argc, sqlite3_value **argv);
void sqrt(sqlite3_context *context, int argc, sqlite3_value **argv);
void max(sqlite3_context *context, int argc, sqlite3_value **argv);
void min(sqlite3_context *context, int argc, sqlite3_value **argv);
void ceil(sqlite3_context *context, int argc, sqlite3_value **argv);
void floor(sqlite3_context *context, int argc, sqlite3_value **argv);
void cos(sqlite3_context *context, int argc, sqlite3_value **argv);
void sin(sqlite3_context *context, int argc, sqlite3_value **argv);
void tan(sqlite3_context *context, int argc, sqlite3_value **argv);
void acos(sqlite3_context *context, int argc, sqlite3_value **argv);
void asin(sqlite3_context *context, int argc, sqlite3_value **argv);
void atan(sqlite3_context *context, int argc, sqlite3_value **argv);
void atan2(sqlite3_context *context, int argc, sqlite3_value **argv);
void cosh(sqlite3_context *context, int argc, sqlite3_value **argv);
void sinh(sqlite3_context *context, int argc, sqlite3_value **argv);
void tanh(sqlite3_context *context, int argc, sqlite3_value **argv);
void acosh(sqlite3_context *context, int argc, sqlite3_value **argv);
void asinh(sqlite3_context *context, int argc, sqlite3_value **argv);
void atanh(sqlite3_context *context, int argc, sqlite3_value **argv);
void exp(sqlite3_context *context, int argc, sqlite3_value **argv);
void log(sqlite3_context *context, int argc, sqlite3_value **argv);
void log10(sqlite3_context *context, int argc, sqlite3_value **argv);
void log2(sqlite3_context *context, int argc, sqlite3_value **argv);
void logb(sqlite3_context *context, int argc, sqlite3_value **argv);
void cbrt(sqlite3_context *context, int argc, sqlite3_value **argv);
void erf(sqlite3_context *context, int argc, sqlite3_value **argv);
void erfc(sqlite3_context *context, int argc, sqlite3_value **argv);
void tgamma(sqlite3_context *context, int argc, sqlite3_value **argv);
void lgamma(sqlite3_context *context, int argc, sqlite3_value **argv);
void hypot(sqlite3_context *context, int argc, sqlite3_value **argv);
void abs(sqlite3_context *context, int argc, sqlite3_value **argv);
void fabs(sqlite3_context *context, int argc, sqlite3_value **argv);
void dist(sqlite3_context *context, int argc, sqlite3_value **argv);
}

#endif //SQLITE_CUSTOM_FUNCTIONS_H
