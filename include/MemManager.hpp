#ifndef MEMMANAGER_INCLUDED
#define MEMMANAGER_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr int MEMINFO_MAX = 1024 * 8;

namespace Clb184 {
	class MemManager {
	private:
		struct MemInfo {
			void* ptr_address = nullptr;
			size_t allocated = 0;
			const char* source_file = "";
			size_t line = 0;
		};
	public:
		MemManager();
		~MemManager();

		void* Allocate(size_t size, const char* source_file = "", size_t line = 0);
		void Free(void* address);
		void* Reallocate(void* address, size_t size);
	private:
		int m_BlockCnt;
		MemInfo m_vMemInfo[MEMINFO_MAX] = {};
	};

	extern MemManager g_MemManager;

}

#define MemAlloc(n) Clb184::g_MemManager.Allocate(n, __FILE__, __LINE__)
#define MemFree(p) Clb184::g_MemManager.Free(p)
#define MemRealloc(p, n) Clb184::g_MemManager.Reallocate(p, n)

void* operator new(size_t size, const char* src, size_t line);
void* operator new[](size_t size, const char* src, size_t line);

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* mem);
void operator delete[](void* mem);

#endif // !MEMMANAGER_INCLUDED
