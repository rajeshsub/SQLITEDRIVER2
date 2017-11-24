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

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


/**
* \file
*/

#ifndef SQLITE_DRIVER2_H
#define SQLITE_DRIVER2_H

#include <QSqlDriver>
#include <QSqlResult>
#include <sqlite3.h>
#include "sqlitedriver2_api.h"

#ifdef _WIN32
    #ifdef SQLITEDRIVER2_EXPORT 
        #define SQLITEDRIVER2_API __declspec(dllexport)
    #else
        #define SQLITEDRIVER2_API __declspec(dllimport)
    #endif
#endif

struct sqlite3;
QT_BEGIN_NAMESPACE

#if 0
#pragma qt_no_master_include
#pragma qt_sync_stop_processing
#endif

class SqliteDriverPrivate2;
class SqliteDriver2;

class SQLITEDRIVER2_API SqliteDriver2 : public QSqlDriver
{
    Q_OBJECT
    friend class  SqliteResult2;
public:
    explicit SqliteDriver2(QObject *parent = 0);
    explicit SqliteDriver2(sqlite3 *connection, QObject *parent = 0);
    ~SqliteDriver2();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
        const QString & user,
        const QString & password,
        const QString & host,
        int port,
        const QString & connOpts);
    void close();
    QSqlResult *createResult() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(QSql::TableType) const;

    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;
    QVariant handle() const;
    QString escapeIdentifier(const QString &identifier, IdentifierType) const;

public:
    SqliteDriverPrivate2* d;
    bool RegisterStandardMathFunctions();
    bool CreateSqlite3Function(const char* functionName, void(*)(sqlite3_context*, int, sqlite3_value **), int nParams = 1);
};

QT_END_NAMESPACE

#endif // SQLITE_DRIVER2_H