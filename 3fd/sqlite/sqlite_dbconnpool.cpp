//
// Copyright (c) 2020 Part of 3FD project (https://github.com/faburaya/3fd)
// It is FREELY distributed by the author under the Microsoft Public License
// and the observance that it should only be used for the benefit of mankind.
//
#include "pch.h"
#include "sqlite.h"
#include <3fd/core/exceptions.h>
#include <3fd/core/logger.h>

#include <sqlite3/sqlite3.h>
#include <cassert>
#include <sstream>

namespace _3fd
{
    using std::ostringstream;

    namespace sqlite
    {
        ///////////////////////////////
        // DbConnPool Class
        ///////////////////////////////

        /// <summary>
        /// Initializes a new instance of the <see cref="DbConnPool"/> class.
        /// </summary>
        /// <param name="dbFilePath">The database file path.</param>
        DbConnPool::DbConnPool(const string &dbFilePath)
        try : m_availableConnections(),
              m_dbFilePath(dbFilePath),
              m_numConns(0)
        {
        }
        catch (std::exception &ex)
        {
            CALL_STACK_TRACE;
            std::ostringstream oss;
            oss << "Failed to instantiate pool of SQLite connections: " << ex.what();
            throw core::AppException<std::runtime_error>(oss.str());
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="ModbusConnPool"/> class.
        /// </summary>
        DbConnPool::~DbConnPool()
        {
            CloseAll();
        }

        /// <summary>
        /// Get a SQLite connection from the pool if available, otherwise, creates a new one.
        /// </summary>
        /// <returns>A wrapped SQLite connection.</returns>
        DbConnWrapper DbConnPool::AcquireSQLiteConn()
        {
            DatabaseConn *conn = m_availableConnections.Remove();

            if (conn != nullptr)
                return DbConnWrapper(*this, conn);
            else
            {
                conn = dbg_new DatabaseConn(m_dbFilePath);
                m_numConns.fetch_add(1, std::memory_order_acq_rel);
                return DbConnWrapper(*this, conn);
            }
        }

        /// <summary>
        /// Returns a SQLite database "connection" to the pool.
        /// </summary>
        /// <param name="conn">The database connection.</param>
        void DbConnPool::ReleaseSQLiteConn(DatabaseConn *conn)
        {
            _ASSERTE(conn != nullptr); // Must not return an invalid context
            m_availableConnections.Add(conn);
        }

        /// <summary>
        /// Closes and removes all the connections in the pool.
        /// </summary>
        void DbConnPool::CloseAll()
        {
            size_t numClosedConns(0);
            DatabaseConn *conn;
            while ((conn = m_availableConnections.Remove()) != nullptr)
            {
                delete conn;
                ++numClosedConns;
            }

            /* If this assertion fails, it means the client code did not 
        properly released all the connections it got from this pool! */
            _ASSERTE(m_numConns.load() == numClosedConns);

            m_numConns.exchange(0, std::memory_order_release);
        }

        ///////////////////////////////
        // DbConnWrapper Class
        ///////////////////////////////

        /// <summary>
        /// Initializes a new instance of the <see cref="DbConnWrapper"/> class.
        /// </summary>
        /// <param name="p_pool">The pool of database connection from which the given connection came.</param>
        /// <param name="p_conn">The database connection to wrap.</param>
        DbConnWrapper::DbConnWrapper(DbConnPool &p_pool, DatabaseConn *p_conn)
            : pool(p_pool), conn(p_conn) {}

        /// <summary>
        /// Initializes a new instance of the <see cref="DbConnWrapper"/> class using move semantics.
        /// </summary>
        /// <param name="ob">The object whose resource will be stolen.</param>
        DbConnWrapper::DbConnWrapper(DbConnWrapper &&ob) noexcept
            : pool(ob.pool), conn(ob.conn)
        {
            ob.conn = nullptr;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="DbConnWrapper"/> class.
        /// </summary>
        DbConnWrapper::~DbConnWrapper()
        {
            if (conn != nullptr)
                pool.ReleaseSQLiteConn(conn);
        }

    } // end of namespace sqlite
} // end of namespace _3fd
