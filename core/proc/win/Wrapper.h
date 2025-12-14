#include <windows.h>

namespace lab {
	namespace util {

		// Windows handle wrapper
		class WinHandle {
			HANDLE m_handle = NULL;

		public:
			explicit WinHandle(HANDLE h)
				: m_handle(h) {
				// Utils::LogDebug("WinHandle created: ");
			}


			WinHandle(const WinHandle&) = delete;

			WinHandle(WinHandle&& other) noexcept
				: m_handle(other.m_handle) {
				other.m_handle = NULL;
			}

			// object conversion to HANDLE
			operator HANDLE() const {
				return m_handle;
			}

			~WinHandle() {
				if (m_handle) {
					CloseHandle(m_handle);
				}
			}
		};

	}
}