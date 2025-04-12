#include "MemManager.hpp"

void* operator new(size_t size, const char* src, size_t line) {
	return Clb184::g_MemManager.Allocate(size, src, line);
}

void* operator new[](size_t size, const char* src, size_t line) {
	return Clb184::g_MemManager.Allocate(size, src, line);
}

void* operator new(size_t size) {
	return Clb184::g_MemManager.Allocate(size, "Unknown");
}

void* operator new[](size_t size) {
	return Clb184::g_MemManager.Allocate(size, "Unknown");
}

void operator delete(void* address) {
	Clb184::g_MemManager.Free(address);
}

void operator delete[](void* address) {
	Clb184::g_MemManager.Free(address);
}

namespace Clb184 {
	MemManager g_MemManager;

	MemManager::MemManager() {
		m_BlockCnt = 0;
		memset(m_vMemInfo, 0x00000000, sizeof(MemInfo) * MEMINFO_MAX);
	}

	MemManager::~MemManager() {
		if (m_BlockCnt > 0) {
			size_t total_bytes = 0;
			printf("Not all memory has been freed:\n");
			for (int i = 0; i < MEMINFO_MAX; i++) {
				if (nullptr != m_vMemInfo[i].ptr_address) {
					//printf("%15s : %d -> %d bytes still allocated\n", m_vMemInfo[i].source_file, m_vMemInfo[i].line, m_vMemInfo[i].allocated);
					free(m_vMemInfo[i].ptr_address);
					m_vMemInfo[i].ptr_address = nullptr;
					total_bytes += m_vMemInfo[i].allocated;
				}
			}
			printf("Freed %d orphan bytes\n", total_bytes);
		}
		else {
			printf("All memory freed\n");
		}
	}

	void* MemManager::Allocate(size_t size, const char* source_file, size_t line) {
		void* ret = malloc(size);
		if (nullptr != ret) {
			for (int i = 0; i < MEMINFO_MAX; i++) {
				if (nullptr == m_vMemInfo[i].ptr_address) {
					const char* p = source_file + strlen(source_file);
					while (p > source_file) {
						p--;
						if (*p == '/' || *p == '\\') {
							p++;
							break;
						}
					}
					m_vMemInfo[i].ptr_address = ret;
					m_vMemInfo[i].allocated = size;
					m_vMemInfo[i].source_file = p;
					m_vMemInfo[i].line = line;
					m_BlockCnt++;
					//printf("Mem @ %p -> %d bytes allocated\n", ret, size);
					return ret;
				}
			}
			//printf("There's no room for Memory info, returning ptr %p\n", ret);
			return ret;
		}
		else {
			//printf("Failed allocating memory\n");
			return nullptr;
		}
	}

	void MemManager::Free(void* address) {
		if (nullptr == address) {
			//printf("nullptr given, ignoring\n");
		}
		else {
			for (int i = 0; i < MEMINFO_MAX; i++) {
				if (address == m_vMemInfo[i].ptr_address) {
					m_vMemInfo[i].ptr_address = nullptr;
					m_vMemInfo[i].allocated = 0;
					m_vMemInfo[i].source_file = "";
					m_vMemInfo[i].line = 0;
					m_BlockCnt--;
					free(address);
					//printf("Mem @ %p -> Freed memory\n", address);
					return;
				}
			}
			//printf("There's no Memory info for this address, freeing %p anyway\n", address);
			free(address);
		}
	}

	void* MemManager::Reallocate(void* address, size_t size) {
		if (nullptr == address) {
			//printf("nullptr given, ignoring\n");
			return nullptr;
		}
		else {
			void* pNew = realloc(address, size);
			if (nullptr != pNew) {
				for (int i = 0; i < MEMINFO_MAX; i++) {
					if (address == m_vMemInfo[i].ptr_address) {
						void* pOld = address;
						m_vMemInfo[i].ptr_address = pNew;
						m_vMemInfo[i].allocated = size;

						//printf("Mem @ %p -> Reallocated memory to Mem @ %p\n", pOld, pNew);
						return pNew;
					}
				}
				//printf("There's no Memory info for this address, reallocating %p to %p anyway\n", address, pNew);
				return pNew;
			}
			else {
				//printf("Failed reallocating memory\n");
				return nullptr;
			}
		}
	}

}