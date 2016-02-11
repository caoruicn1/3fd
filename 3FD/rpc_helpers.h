#ifndef RPC_H // header guard
#define RPC_H

#include "exceptions.h"
#include "logger.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <AuthZ.h>
#include <rpc.h>

namespace _3fd
{
    using std::string;

    namespace rpc
    {
        /// <summary>
        /// Enumerates the possible options for RPC transport.
        /// </summary>
        enum class ProtocolSequence { Local, TCP };

        /// <summary>
        /// Enumerates the possible options for authentication level.
        /// </summary>
        enum class AuthenticationLevel : unsigned long
        {
            None = RPC_C_AUTHN_LEVEL_NONE, // no auth/security at all
            Integrity = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
            Privacy = RPC_C_AUTHN_LEVEL_PKT_PRIVACY
        };

        /// <summary>
        /// Enumerates the possible options for impersonation level.
        /// </summary>
        enum class ImpersonationLevel : unsigned long
        {
            Default = RPC_C_IMP_LEVEL_DEFAULT, // automatic
            Identify = RPC_C_IMP_LEVEL_IDENTIFY,
            Impersonate = RPC_C_IMP_LEVEL_IMPERSONATE,
            Delegate = RPC_C_IMP_LEVEL_DELEGATE
        };

        /// <summary>
        /// Holds a definition for a particular RPC interface implementation.
        /// </summary>
        struct RpcSrvObject
        {
            /// <summary>
            /// The UUID of the object, which is an external identifier
            /// exposed to clients. (This is not the interface UUID.)
            /// </summary>
            string uuid;

            /// <summary>
            /// The interface handle defined in the stub generated by MIDL compiler from IDL file.
            /// This handle internally defines the default EPV (when MIDL has been run with '/use_epv').
            /// </summary>
            RPC_IF_HANDLE interfaceHandle;

            /// <summary>
            /// The entry point vector. If this is null, then the default EPV supplied
            /// by the inteface handle is used.
            /// </summary>
            RPC_MGR_EPV *epv;

            RpcSrvObject(
                const string &p_uuid,
                RPC_IF_HANDLE p_interfaceHandle,
                RPC_MGR_EPV *p_epv = nullptr)
            :
                uuid(p_uuid),
                interfaceHandle(p_interfaceHandle),
                epv(p_epv)
            {}
        };

        class RpcServerImpl;

        /// <summary>
        /// Represents the RPC server that runs inside the application process.
        /// </summary>
        class RpcServer
        {
        private:

            static std::unique_ptr<RpcServerImpl> uniqueObject;

            static std::mutex singletonAccessMutex;

            RpcServer() {}

        public:

            ~RpcServer() {}

            static void Initialize(
                ProtocolSequence protSeq,
                const string &serviceClass,
                AuthenticationLevel authLevel,
                bool useActDirSec
            );

            static AuthenticationLevel GetRequiredAuthLevel();

            static bool Start(const std::vector<RpcSrvObject> &objects);

            static bool Stop();

            static bool Resume();

            static bool Wait();

            static bool Finalize() noexcept;
        };

        /// <summary>
        /// Implements an RPC client that provides an explicit binding handle
        /// to use as parameter for client stub code generated by the MIDL compiler.
        /// Client code is expected to derive from this class and call <see cref="RpcClient::GetBindingHandle"/>
        /// in order to obtain an explicit binding handle.
        /// </summary>
        class RpcClient
        {
        private:

            RPC_BINDING_HANDLE m_bindingHandle;

        protected:

            /// <summary>
            /// Gets the binding handle.
            /// </summary>
            /// <returns>The explicit binding handle expected as parameter for RPC.</returns>
            RPC_BINDING_HANDLE GetBindingHandle() const { return m_bindingHandle; }

        public:

            RpcClient(
                ProtocolSequence protSeq,
                const string &objUUID,
                const string &destination,
                AuthenticationLevel authLevel,
                ImpersonationLevel impLevel = ImpersonationLevel::Default,
                const string &serviceClass = "",
                const string &endpoint = ""
            );

            ~RpcClient();

            void ResetBindings();
        };

        /// <summary>
        /// Uses RAII to define a scope where impersonation takes place.
        /// </summary>
        class ScopedImpersonation
        {
        private:

            RPC_BINDING_HANDLE m_clientBindingHandle;

        public:

            ScopedImpersonation(RPC_BINDING_HANDLE clientBindingHandle);
            ~ScopedImpersonation();
        };

        /////////////////////////
        // Error Helpers
        /////////////////////////

        void ThrowIfError(RPC_STATUS status, const char *message);

        void ThrowIfError(
            RPC_STATUS status,
            const char *message,
            const string &details
        );

        void LogIfError(
            RPC_STATUS status,
            const char *message,
            core::Logger::Priority prio
        ) noexcept;

        void LogIfError(
            RPC_STATUS status,
            const char *message,
            const string &details,
            core::Logger::Priority prio
        ) noexcept;

    }// end of namespace rpc
}// end of namespace _3fd

#endif // end of header guard
