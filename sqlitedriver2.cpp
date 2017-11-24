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

#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <QtSql/5.6.1/QtSql/private/qsqlcachedresult_p.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qdebug.h>

#if defined Q_OS_WIN
# include <qt_windows.h>
#else
# include <unistd.h>
#endif

#include "sqlitedriver2_functions.h"
#include "sqlitedriver2.h"

Q_DECLARE_OPAQUE_POINTER(sqlite3*)
Q_DECLARE_METATYPE(sqlite3*)

Q_DECLARE_OPAQUE_POINTER(sqlite3_stmt*)
Q_DECLARE_METATYPE(sqlite3_stmt*)

QT_BEGIN_NAMESPACE

static QString _q_escapeIdentifier(const QString &identifier)
{
    QString res = identifier;
    if (!identifier.isEmpty() && identifier.left(1) != QString(QLatin1Char('"')) && identifier.right(1) != QString(QLatin1Char('"'))) {
        res.replace(QLatin1Char('"'), QLatin1String("\"\""));
        res.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
        res.replace(QLatin1Char('.'), QLatin1String("\".\""));
    }
    return res;
}

static QVariant::Type qGetColumnType(const QString &tpName)
{
    const QString typeName = tpName.toLower();

    if (typeName == QLatin1String("integer")
        || typeName == QLatin1String("int"))
        return QVariant::Int;
    if (typeName == QLatin1String("double")
        || typeName == QLatin1String("float")
        || typeName == QLatin1String("real")
        || typeName.startsWith(QLatin1String("numeric")))
        return QVariant::Double;
    if (typeName == QLatin1String("blob"))
        return QVariant::ByteArray;
    if (typeName == QLatin1String("boolean")
        || typeName == QLatin1String("bool"))
        return QVariant::Bool;
    return QVariant::String;
}

static QSqlError qMakeError(sqlite3 *access, const QString &descr, QSqlError::ErrorType type,
    int errorCode = -1)
{
    return QSqlError(descr,
        QString(reinterpret_cast<const QChar *>(sqlite3_errmsg16(access))),
        type, errorCode);
}

class SqliteDriverResultPrivate2;

class SqliteResult2 : public QSqlCachedResult
{
    friend class SqliteDriver2;
    friend class SqliteDriverResultPrivate2;
public:
    explicit SqliteResult2(const SqliteDriver2* db);
    ~SqliteResult2();
    QVariant handle() const;

protected:
    bool gotoNext(QSqlCachedResult::ValueCache& row, int idx);
    bool reset(const QString &query);
    bool prepare(const QString &query);
    bool exec();
    int size();
    int numRowsAffected();
    QVariant lastInsertId() const;
    QSqlRecord record() const;
    void detachFromResultSet();
    void virtual_hook(int id, void *data);

private:
    SqliteDriverResultPrivate2* d;
};

class SqliteDriverPrivate2
{
public:
    inline SqliteDriverPrivate2() : access(0) {}
    sqlite3 *access;
    QList <SqliteResult2 *> results;
};


class SqliteDriverResultPrivate2
{
public:
    SqliteDriverResultPrivate2(SqliteResult2 *res);
    void cleanup();
    bool fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch);
    // initializes the recordInfo and the cache
    void initColumns(bool emptyResultset);
    void finalize();

    SqliteResult2* q;
    sqlite3 *access;

    sqlite3_stmt *stmt;

    bool skippedStatus; // the status of the fetchNext() that's skipped
    bool skipRow; // skip the next fetchNext()?
    QSqlRecord rInf;
    QVector<QVariant> firstRow;
};

SqliteDriverResultPrivate2::SqliteDriverResultPrivate2(SqliteResult2* res) : q(res), access(0),
stmt(0), skippedStatus(false), skipRow(false)
{
}

void SqliteDriverResultPrivate2::cleanup()
{
    finalize();
    rInf.clear();
    skippedStatus = false;
    skipRow = false;
    q->setAt(QSql::BeforeFirstRow);
    q->setActive(false);
    q->cleanup();
}

void SqliteDriverResultPrivate2::finalize()
{
    if (!stmt)
        return;

    sqlite3_finalize(stmt);
    stmt = 0;
}

void SqliteDriverResultPrivate2::initColumns(bool emptyResultset)
{
    int nCols = sqlite3_column_count(stmt);
    if (nCols <= 0)
        return;

    q->init(nCols);

    for (int i = 0; i < nCols; ++i) {
        QString colName = QString(reinterpret_cast<const QChar *>(
            sqlite3_column_name16(stmt, i))
            ).remove(QLatin1Char('"'));

        // must use typeName for resolving the type to match WSQLiteDriver::record
        QString typeName = QString(reinterpret_cast<const QChar *>(
            sqlite3_column_decltype16(stmt, i)));
        // sqlite3_column_type is documented to have undefined behavior if the result set is empty
        int stp = emptyResultset ? -1 : sqlite3_column_type(stmt, i);

        QVariant::Type fieldType;

        if (!typeName.isEmpty()) {
            fieldType = qGetColumnType(typeName);
        }
        else {
            // Get the proper type for the field based on stp value
            switch (stp) {
                case SQLITE_INTEGER:
                    fieldType = QVariant::Int;
                    break;
                case SQLITE_FLOAT:
                    fieldType = QVariant::Double;
                    break;
                case SQLITE_BLOB:
                    fieldType = QVariant::ByteArray;
                    break;
                case SQLITE_TEXT:
                    fieldType = QVariant::String;
                    break;
                case SQLITE_NULL:
                default:
                    fieldType = QVariant::Invalid;
                    break;
            }
        }

        int dotIdx = colName.lastIndexOf(QLatin1Char('.'));
        QSqlField fld(colName.mid(dotIdx == -1 ? 0 : dotIdx + 1), fieldType);
        fld.setSqlType(stp);
        rInf.append(fld);
    }
}

bool SqliteDriverResultPrivate2::fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch)
{
    int res;
    int i;

    if (skipRow) {
        // already fetched
        Q_ASSERT(!initialFetch);
        skipRow = false;
        for (int i = 0; i < firstRow.count(); i++)
            values[i] = firstRow[i];
        return skippedStatus;
    }
    skipRow = initialFetch;

    if (initialFetch) {
        firstRow.clear();
        firstRow.resize(sqlite3_column_count(stmt));
    }

    if (!stmt) {
        q->setLastError(QSqlError(QCoreApplication::translate("WSQLiteResult", "Unable to fetch row"),
            QCoreApplication::translate("WSQLiteResult", "No query"), QSqlError::ConnectionError));
        q->setAt(QSql::AfterLastRow);
        return false;
    }
    res = sqlite3_step(stmt);

    switch (res) {
        case SQLITE_ROW:
            // check to see if should fill out columns
            if (rInf.isEmpty())
                // must be first call.
                initColumns(false);
            if (idx < 0 && !initialFetch)
                return true;
            for (i = 0; i < rInf.count(); ++i) {
                switch (sqlite3_column_type(stmt, i)) {
                    case SQLITE_BLOB:
                        values[i + idx] = QByteArray(static_cast<const char *>(
                            sqlite3_column_blob(stmt, i)),
                            sqlite3_column_bytes(stmt, i));
                        break;
                    case SQLITE_INTEGER:
                        values[i + idx] = sqlite3_column_int64(stmt, i);
                        break;
                    case SQLITE_FLOAT:
                        switch (q->numericalPrecisionPolicy()) {
                            case QSql::LowPrecisionInt32:
                                values[i + idx] = sqlite3_column_int(stmt, i);
                                break;
                            case QSql::LowPrecisionInt64:
                                values[i + idx] = sqlite3_column_int64(stmt, i);
                                break;
                            case QSql::LowPrecisionDouble:
                            case QSql::HighPrecision:
                            default:
                                values[i + idx] = sqlite3_column_double(stmt, i);
                                break;
                        };
                        break;
                    case SQLITE_NULL:
                        values[i + idx] = QVariant(QVariant::String);
                        break;
                    default:
                        values[i + idx] = QString(reinterpret_cast<const QChar *>(
                            sqlite3_column_text16(stmt, i)),
                            sqlite3_column_bytes16(stmt, i) / sizeof(QChar));
                        break;
                }
            }
            return true;
        case SQLITE_DONE:
            if (rInf.isEmpty())
                // must be first call.
                initColumns(true);
            q->setAt(QSql::AfterLastRow);
            sqlite3_reset(stmt);
            return false;
        case SQLITE_CONSTRAINT:
        case SQLITE_ERROR:
            // SQLITE_ERROR is a generic error code and we must call sqlite3_reset()
            // to get the specific error message.
            res = sqlite3_reset(stmt);
            q->setLastError(qMakeError(access, QCoreApplication::translate("WSQLiteResult",
                "Unable to fetch row"), QSqlError::ConnectionError, res));
            q->setAt(QSql::AfterLastRow);
            return false;
        case SQLITE_MISUSE:
        case SQLITE_BUSY:
        default:
            // something wrong, don't get col info, but still return false
            q->setLastError(qMakeError(access, QCoreApplication::translate("WSQLiteResult",
                "Unable to fetch row"), QSqlError::ConnectionError, res));
            sqlite3_reset(stmt);
            q->setAt(QSql::AfterLastRow);
            return false;
    }
    return false;
}

SqliteResult2::SqliteResult2(const SqliteDriver2* db)
    : QSqlCachedResult(db)
{
    d = new SqliteDriverResultPrivate2(this);
    d->access = db->d->access;
    db->d->results.append(this);
}

SqliteResult2::~SqliteResult2()
{
    const QSqlDriver *sqlDriver = driver();
    if (sqlDriver)
        qobject_cast<const SqliteDriver2 *>(sqlDriver)->d->results.removeOne(this);
    d->cleanup();
    delete d;
}

void SqliteResult2::virtual_hook(int id, void *data)
{
    QSqlCachedResult::virtual_hook(id, data);
}

bool SqliteResult2::reset(const QString &query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool SqliteResult2::prepare(const QString &query)
{
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;

    d->cleanup();

    setSelect(false);

    const void *pzTail = NULL;

#if (SQLITE_VERSION_NUMBER >= 3003011)
    int res = sqlite3_prepare16_v2(d->access, query.constData(), (query.size() + 1) * sizeof(QChar),
        &d->stmt, &pzTail);
#else
    int res = sqlite3_prepare16(d->access, query.constData(), (query.size() + 1) * sizeof(QChar),
        &d->stmt, &pzTail);
#endif

    if (res != SQLITE_OK) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("WSQLiteResult",
            "Unable to execute statement"), QSqlError::StatementError, res));
        d->finalize();
        return false;
    }
    else if (pzTail && !QString(reinterpret_cast<const QChar *>(pzTail)).trimmed().isEmpty()) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("WSQLiteResult",
            "Unable to execute multiple statements at a time"), QSqlError::StatementError, SQLITE_MISUSE));
        d->finalize();
        return false;
    }
    return true;
}

bool SqliteResult2::exec()
{
    const QVector<QVariant> values = boundValues();

    d->skippedStatus = false;
    d->skipRow = false;
    d->rInf.clear();
    clearValues();
    setLastError(QSqlError());

    int res = sqlite3_reset(d->stmt);
    if (res != SQLITE_OK) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("WSQLiteResult",
            "Unable to reset statement"), QSqlError::StatementError, res));
        d->finalize();
        return false;
    }
    int paramCount = sqlite3_bind_parameter_count(d->stmt);
    if (paramCount == values.count()) {
        for (int i = 0; i < paramCount; ++i) {
            res = SQLITE_OK;
            const QVariant value = values.at(i);

            if (value.isNull()) {
                res = sqlite3_bind_null(d->stmt, i + 1);
            }
            else {
                switch (value.type()) {
                    case QVariant::ByteArray: {
                            const QByteArray *ba = static_cast<const QByteArray*>(value.constData());
                            res = sqlite3_bind_blob(d->stmt, i + 1, ba->constData(),
                                ba->size(), SQLITE_STATIC);
                            break; }
                    case QVariant::Int:
                    case QVariant::Bool:
                        res = sqlite3_bind_int(d->stmt, i + 1, value.toInt());
                        break;
                    case QVariant::Double:
                        res = sqlite3_bind_double(d->stmt, i + 1, value.toDouble());
                        break;
                    case QVariant::UInt:
                    case QVariant::LongLong:
                        res = sqlite3_bind_int64(d->stmt, i + 1, value.toLongLong());
                        break;
                    case QVariant::String: {
                            // lifetime of string == lifetime of its qvariant
                            const QString *str = static_cast<const QString*>(value.constData());
                            res = sqlite3_bind_text16(d->stmt, i + 1, str->utf16(),
                                (str->size()) * sizeof(QChar), SQLITE_STATIC);
                            break; }
                    default: {
                            QString str = value.toString();
                            // SQLITE_TRANSIENT makes sure that sqlite buffers the data
                            res = sqlite3_bind_text16(d->stmt, i + 1, str.utf16(),
                                (str.size()) * sizeof(QChar), SQLITE_TRANSIENT);
                            break; }
                }
            }
            if (res != SQLITE_OK) {
                setLastError(qMakeError(d->access, QCoreApplication::translate("WSQLiteResult",
                    "Unable to bind parameters"), QSqlError::StatementError, res));
                d->finalize();
                return false;
            }
        }
    }
    else {
        setLastError(QSqlError(QCoreApplication::translate("WSQLiteResult",
            "Parameter count mismatch"), QString(), QSqlError::StatementError));
        return false;
    }
    d->skippedStatus = d->fetchNext(d->firstRow, 0, true);
    if (lastError().isValid()) {
        setSelect(false);
        setActive(false);
        return false;
    }
    setSelect(!d->rInf.isEmpty());
    setActive(true);
    return true;
}

bool SqliteResult2::gotoNext(QSqlCachedResult::ValueCache& row, int idx)
{
    return d->fetchNext(row, idx, false);
}

int SqliteResult2::size()
{
    return -1;
}

int SqliteResult2::numRowsAffected()
{
    return sqlite3_changes(d->access);
}

QVariant SqliteResult2::lastInsertId() const
{
    if (isActive()) {
        qint64 id = sqlite3_last_insert_rowid(d->access);
        if (id)
            return id;
    }
    return QVariant();
}

QSqlRecord SqliteResult2::record() const
{
    if (!isActive() || !isSelect())
        return QSqlRecord();
    return d->rInf;
}

void SqliteResult2::detachFromResultSet()
{
    if (d->stmt)
        sqlite3_reset(d->stmt);
}

QVariant SqliteResult2::handle() const
{
    return QVariant::fromValue(d->stmt);
}

/////////////////////////////////////////////////////////

SqliteDriver2::SqliteDriver2(QObject * parent)
    : QSqlDriver(parent)
{
    d = new SqliteDriverPrivate2();
}

SqliteDriver2::SqliteDriver2(sqlite3 *connection, QObject *parent)
    : QSqlDriver(parent)
{
    d = new SqliteDriverPrivate2();
    d->access = connection;
    setOpen(true);
    setOpenError(false);
}


SqliteDriver2::~SqliteDriver2()
{
    delete d;
}

bool SqliteDriver2::hasFeature(DriverFeature f) const
{
    switch (f) {
        case BLOB:
        case Transactions:
        case Unicode:
        case LastInsertId:
        case PreparedQueries:
        case PositionalPlaceholders:
        case SimpleLocking:
        case FinishQuery:
        case LowPrecisionNumbers:
            return true;
        case QuerySize:
        case NamedPlaceholders:
        case BatchOperations:
        case EventNotifications:
        case MultipleResultSets:
        case CancelQuery:
            return false;
    }
    return false;
}

/*
SQLite dbs have no user name, passwords, hosts or ports.
just file names.
*/
bool SqliteDriver2::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &conOpts)
{
    if (isOpen())
        close();

    if (db.isEmpty())
        return false;
    bool sharedCache = false;
    int openMode = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, timeOut = 5000;
    QStringList opts = QString(conOpts).remove(QLatin1Char(' ')).split(QLatin1Char(';'));
    foreach(const QString &option, opts) {
        if (option.startsWith(QLatin1String("QSQLITE_BUSY_TIMEOUT="))) {
            bool ok;
            int nt = option.mid(21).toInt(&ok);
            if (ok)
                timeOut = nt;
        }
        if (option == QLatin1String("QSQLITE_OPEN_READONLY"))
            openMode = SQLITE_OPEN_READONLY;
        if (option == QLatin1String("QSQLITE_ENABLE_SHARED_CACHE"))
            sharedCache = true;
    }

    sqlite3_enable_shared_cache(sharedCache);

    if (sqlite3_open_v2(db.toUtf8().constData(), &d->access, openMode, NULL) == SQLITE_OK) {
        sqlite3_busy_timeout(d->access, timeOut);
        setOpen(true);
        setOpenError(false);

        if (!RegisterStandardMathFunctions())
        {
            //log an error here
            return false;
        }

        return true;
    }
    else {
        if (d->access) {
            sqlite3_close(d->access);
            d->access = 0;
        }

        setLastError(qMakeError(d->access, tr("Error opening database"),
            QSqlError::ConnectionError));
        setOpenError(true);
        return false;
    }
}

bool SqliteDriver2::CreateSqlite3Function(const char* functionName, void(*funcAddress)(sqlite3_context*, int, sqlite3_value **), int nParams/* = 1*/)
{
    return (SQLITE_OK == sqlite3_create_function(d->access, functionName, nParams, SQLITE_UTF8, NULL, funcAddress, NULL, NULL));
}

bool SqliteDriver2::RegisterStandardMathFunctions()
{
    if (SQLITE_OK == sqlite3_initialize())
    {
        if (!CreateSqlite3Function("sqrt", &SQLITE_CUSTOM_FUNCTIONS::sqrt))
        {
            return false;
        }
        else if (!CreateSqlite3Function("pow", &SQLITE_CUSTOM_FUNCTIONS::pow, 2))
        {
            return false;
        }
        else if (!CreateSqlite3Function("ceil", &SQLITE_CUSTOM_FUNCTIONS::ceil))
        {
            return false;
        }
        else if (!CreateSqlite3Function("floor", &SQLITE_CUSTOM_FUNCTIONS::floor))
        {
            return false;
        }
        else if (!CreateSqlite3Function("cos", &SQLITE_CUSTOM_FUNCTIONS::cos))
        {
            return false;
        }
        else if (!CreateSqlite3Function("sin", &SQLITE_CUSTOM_FUNCTIONS::sin))
        {
            return false;
        }
        else if (!CreateSqlite3Function("tan", &SQLITE_CUSTOM_FUNCTIONS::tan))
        {
            return false;
        }
        else if (!CreateSqlite3Function("acos", &SQLITE_CUSTOM_FUNCTIONS::acos))
        {
            return false;
        }
        else if (!CreateSqlite3Function("asin", &SQLITE_CUSTOM_FUNCTIONS::asin))
        {
            return false;
        }
        else if (!CreateSqlite3Function("atan", &SQLITE_CUSTOM_FUNCTIONS::atan))
        {
            return false;
        }
        else if (!CreateSqlite3Function("atan2", &SQLITE_CUSTOM_FUNCTIONS::atan2, 2))
        {
            return false;
        }
        else if (!CreateSqlite3Function("cosh", &SQLITE_CUSTOM_FUNCTIONS::cosh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("sinh", &SQLITE_CUSTOM_FUNCTIONS::sinh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("tanh", &SQLITE_CUSTOM_FUNCTIONS::tanh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("acosh", &SQLITE_CUSTOM_FUNCTIONS::acosh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("asinh", &SQLITE_CUSTOM_FUNCTIONS::asinh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("atanh", &SQLITE_CUSTOM_FUNCTIONS::atanh))
        {
            return false;
        }
        else if (!CreateSqlite3Function("exp", &SQLITE_CUSTOM_FUNCTIONS::exp))
        {
            return false;
        }
        else if (!CreateSqlite3Function("log", &SQLITE_CUSTOM_FUNCTIONS::log))
        {
            return false;
        }
        else if (!CreateSqlite3Function("log10", &SQLITE_CUSTOM_FUNCTIONS::log10))
        {
            return false;
        }
        else if (!CreateSqlite3Function("log2", &SQLITE_CUSTOM_FUNCTIONS::log2))
        {
            return false;
        }
        else if (!CreateSqlite3Function("logb", &SQLITE_CUSTOM_FUNCTIONS::logb))
        {
            return false;
        }
        else if (!CreateSqlite3Function("cbrt", &SQLITE_CUSTOM_FUNCTIONS::cbrt))
        {
            return false;
        }
        else if (!CreateSqlite3Function("erf", &SQLITE_CUSTOM_FUNCTIONS::erf))
        {
            return false;
        }
        else if (!CreateSqlite3Function("erfc", &SQLITE_CUSTOM_FUNCTIONS::erfc))
        {
            return false;
        }
        else if (!CreateSqlite3Function("tgamma", &SQLITE_CUSTOM_FUNCTIONS::tgamma))
        {
            return false;
        }
        else if (!CreateSqlite3Function("lgamma", &SQLITE_CUSTOM_FUNCTIONS::lgamma))
        {
            return false;
        }
        else if (!CreateSqlite3Function("hypot", &SQLITE_CUSTOM_FUNCTIONS::hypot, 2))
        {
            return false;
        }
        else if (!CreateSqlite3Function("abs", &SQLITE_CUSTOM_FUNCTIONS::abs))
        {
            return false;
        }
        else if (!CreateSqlite3Function("fabs", &SQLITE_CUSTOM_FUNCTIONS::fabs))
        {
            return false;
        }
        else if (!CreateSqlite3Function("dist", &SQLITE_CUSTOM_FUNCTIONS::dist, 4))
        {
            return false;
        }
    }
    return true;
}

void SqliteDriver2::close()
{
    if (isOpen()) {
        foreach(SqliteResult2 *result, d->results) {
            result->d->finalize();
        }

        if (sqlite3_close(d->access) != SQLITE_OK)
            setLastError(qMakeError(d->access, tr("Error closing database"),
            QSqlError::ConnectionError));
        d->access = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *SqliteDriver2::createResult() const
{
    return new SqliteResult2(this);
}

bool SqliteDriver2::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("BEGIN"))) {
        setLastError(QSqlError(tr("Unable to begin transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

bool SqliteDriver2::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("COMMIT"))) {
        setLastError(QSqlError(tr("Unable to commit transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

bool SqliteDriver2::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("ROLLBACK"))) {
        setLastError(QSqlError(tr("Unable to rollback transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

QStringList SqliteDriver2::tables(QSql::TableType type) const
{
    QStringList res;
    if (!isOpen())
        return res;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);

    QString sql = QLatin1String("SELECT name FROM sqlite_master WHERE %1 "
        "UNION ALL SELECT name FROM sqlite_temp_master WHERE %1");
    if ((type & QSql::Tables) && (type & QSql::Views))
        sql = sql.arg(QLatin1String("type='table' OR type='view'"));
    else if (type & QSql::Tables)
        sql = sql.arg(QLatin1String("type='table'"));
    else if (type & QSql::Views)
        sql = sql.arg(QLatin1String("type='view'"));
    else
        sql.clear();

    if (!sql.isEmpty() && q.exec(sql)) {
        while (q.next())
            res.append(q.value(0).toString());
    }

    if (type & QSql::SystemTables) {
        // there are no internal tables beside this one:
        res.append(QLatin1String("sqlite_master"));
    }

    return res;
}

static QSqlIndex qGetTableInfo(QSqlQuery &q, const QString &tableName, bool onlyPIndex = false)
{
    QString schema;
    QString table(tableName);
    int indexOfSeparator = tableName.indexOf(QLatin1Char('.'));
    if (indexOfSeparator > -1) {
        schema = tableName.left(indexOfSeparator).append(QLatin1Char('.'));
        table = tableName.mid(indexOfSeparator + 1);
    }
    q.exec(QLatin1String("PRAGMA ") + schema + QLatin1String("table_info (") + _q_escapeIdentifier(table) + QLatin1String(")"));

    QSqlIndex ind;
    while (q.next()) {
        bool isPk = q.value(5).toInt();
        if (onlyPIndex && !isPk)
            continue;
        QString typeName = q.value(2).toString().toLower();
        QSqlField fld(q.value(1).toString(), qGetColumnType(typeName));
        if (isPk && (typeName == QLatin1String("integer")))
            // INTEGER PRIMARY KEY fields are auto-generated in sqlite
            // INT PRIMARY KEY is not the same as INTEGER PRIMARY KEY!
            fld.setAutoValue(true);
        fld.setRequired(q.value(3).toInt() != 0);
        fld.setDefaultValue(q.value(4));
        ind.append(fld);
    }
    return ind;
}

QSqlIndex SqliteDriver2::primaryIndex(const QString &tblname) const
{
    if (!isOpen())
        return QSqlIndex();

    QString table = tblname;
    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, table, true);
}

QSqlRecord SqliteDriver2::record(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecord();

    QString table = tbl;
    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, table);
}

QVariant SqliteDriver2::handle() const
{
    return QVariant::fromValue(d->access);
}

QString SqliteDriver2::escapeIdentifier(const QString &identifier, IdentifierType type) const
{
    Q_UNUSED(type);
    return _q_escapeIdentifier(identifier);
}

QT_END_NAMESPACE

extern "C" void SQLITEDRIVER2_API initSqliteDriver2()
{
    QSqlDatabase::registerSqlDriver("sqlitedriver2", new QSqlDriverCreator<SqliteDriver2>);
}
