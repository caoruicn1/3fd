#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <3fd/core/callstacktracer.h>
#include <string>
#include <sstream>
#include <exception>
#include <memory>
#include <future>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#   include <windows.h>
    class _com_error;
#   ifdef _3FD_PLATFORM_WINRT
#       include <winrt\base.h>
#   endif
#endif

namespace _3fd
{
namespace core
{
    using std::string;

    /// <summary>
    /// Aggregates extension functions that work on objects of the C++ Standard Library.
    /// </summary>
    class StdLibExt
    {
    public:

        static string GetDetailsFromSystemError(const std::error_code &code);

        static string GetDetailsFromSystemError(const std::system_error &ex);

        static string GetDetailsFromFutureError(const std::future_error &ex);
    };

#ifdef _WIN32
    /// <summary>
    /// Aggregates extension functions that work on types/objects of the Windows API
    /// </summary>
    class WWAPI
    {
    public:

        static string GetHResultLabel(HRESULT errCode);

        static string GetDetailsFromHResult(HRESULT errCode);

        static void RaiseHResultException(HRESULT errCode, const char *message, const char *function);

        static void RaiseHResultException(const _com_error &ex, const char *message);

#   ifdef _3FD_PLATFORM_WIN32API
        static void AppendDWordErrorMessage(
            DWORD errCode,
            const char *funcName,
            std::ostringstream &oss,
            HMODULE dlibHandle = nullptr
        );
#   endif

#    ifdef _3FD_PLATFORM_WINRT
        static string GetDetailsFromWinRTEx(const winrt::hresult_error &ex);
#    endif
    };
#endif

    /// <summary>
    /// An interface that encompasses all type of exceptions dealt by this application.
    /// </summary>
    class IAppException
    {
    public:

        virtual ~IAppException() {}
        virtual IAppException *Move() = 0;
        virtual std::shared_ptr<IAppException> GetInnerException() const = 0;

        virtual string What() const = 0;
        virtual string Details() const = 0;
        virtual string ToString() const = 0;
    };

    /// <summary>
    /// A template used to wrap different types of exceptions under the same interface.
    /// </summary>
    template <typename StdExType>
    class AppException : public IAppException, public StdExType
    {
    private:

        string m_details;
        string m_cst;
        std::shared_ptr<IAppException> m_innerEx;

        /// <summary>
        /// Moves this instance exporting the resources to a new object dynamically allocated.
        /// </summary>
        /// <returns>A new exception object built from the resources of this object.</returns>
        virtual AppException *Move() override { return dbg_new AppException(std::move(*this)); }

        /// <summary>
        /// Gets the inner exception.
        /// </summary>
        virtual std::shared_ptr<IAppException> GetInnerException() const override
        {
            return m_innerEx;
        }

    public:

        template <typename StrType>
        AppException(StrType &&what) :
            StdExType(std::forward<StrType>(what))
        {
#        ifdef ENABLE_3FD_CST
            if (CallStackTracer::IsReady())
                m_cst = CallStackTracer::GetStackReport();
#        endif    
        }

        template <typename StrType>
        AppException(StrType &&what, IAppException &innerEx) :
            StdExType(std::forward<StrType>(what)),
            m_innerEx(innerEx.Move())
        {
#        ifdef ENABLE_3FD_CST
            if (CallStackTracer::IsReady())
                m_cst = CallStackTracer::GetStackReport();
#        endif
        }

        template <typename StrType1, typename StrType2>
        AppException(StrType1 &&what, StrType2 &&details) :
            StdExType(std::forward<StrType1>(what)),
            m_details(std::forward<StrType2>(details))
        {
#        ifdef ENABLE_3FD_CST
            if (CallStackTracer::IsReady())
                m_cst = CallStackTracer::GetStackReport();
#        endif
        }

        template <typename StrType1, typename StrType2>
        AppException(StrType1 &&what, StrType2 &&details, IAppException &innerEx) :
            StdExType(std::forward<StrType1>(what)),
            m_details(std::forward<StrType2>(details)),
            m_innerEx(innerEx.Move())
        {
#        ifdef ENABLE_3FD_CST
            if (CallStackTracer::IsReady())
                m_cst = CallStackTracer::GetStackReport();
#        endif
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AppException{StdExType}"/>
        /// class using copy semantics.
        /// </summary>
        /// <param name="ob">The object whose resources will be copied.</param>
        AppException(const AppException &ob)
            : StdExType(ob)
            , m_details(ob.m_details)
            , m_cst(ob.m_cst)
            , m_innerEx(ob.m_innerEx)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AppException{StdExType}"/>
        /// class using move semantics.
        /// </summary>
        /// <param name="ob">The object whose resources will be stolen.</param>
        AppException(AppException &&ob) noexcept
            : StdExType(std::move(ob))
            , m_details(std::move(ob.m_details))
            , m_cst(std::move(ob.m_cst))
        {
            m_innerEx.swap(ob.m_innerEx);
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="AppException{StdExType}"/> class.
        /// </summary>
        virtual ~AppException() {}

        /// <summary>
        /// Gets the main error message.
        /// </summary>
        /// <returns>The main error message, without details or stack trace.</returns>
        virtual string What() const override
        {
            return StdExType::what();
        }

        /// <summary>
        /// Gets the error details.
        /// </summary>
        /// <returns>The error details only, without stack trace.</returns>
        virtual string Details() const override
        {
            return m_details;
        }

        /// <summary>
        /// Gets the exception content, including the details and stack trace (when present)
        /// if in debug mode, serialized to a text format compatible with POCO logger.
        /// </summary>
        virtual string ToString() const override
        {
            std::ostringstream oss;
            oss << StdExType::what();

#        ifdef ENABLE_3FD_ERR_IMPL_DETAILS
            if(m_details.empty() == false)
                oss << " - " << m_details;

#            ifdef ENABLE_3FD_CST
            if (m_cst.empty() == false)
                oss << _newLine_ _newLine_ "### CALL STACK TRACE ###" _newLine_ << m_cst;
#            endif    
#        endif
            return oss.str();
        }

    };// end of class exception

#if defined _3FD_PLATFORM_WIN32API || defined _3FD_PLATFORM_WINRT
    /// <summary>
    /// A specialization of runtime application exception that is handful when
    /// dealing with Windows API errors, because it holds the HRESULT error code.
    /// </summary>
    /// <seealso cref="AppException{std::runtime_error}" />
    class HResultException : public AppException<std::runtime_error>
    {
    private:

        HRESULT m_hres;

    public:

        HResultException(HRESULT hr, string &&message)
            : AppException(std::move(message)), m_hres(hr) {}

        HRESULT GetErrorCode() { return m_hres; }
    };
#endif

}// end namespace core
}// end namespace _3fd

#endif // header guard
