#include "plh_ThreadState.h"
#include "plh_Os.h"
#include <limits.h>
#include <atomic>
#include <mutex>

// skipping finalization is DANGEROUS -- your hooks will be called AFTER
// C runtime is finalized (lots of standard C function will crash-and-burn)

#define _PLH_FINALIZE_HOOKS_AT_EXIT 1

namespace plh {

//..............................................................................

static volatile std::atomic<int> g_enableCount;
static size_t g_threadDisableCountSlot = -1;
static size_t g_threadStateSlot = -1;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
deleteCurrentThreadState()
{
	setTlsValue(g_threadDisableCountSlot, INT_MAX); // compensate for possibly unbalanced enable calls

	ThreadState* state = (ThreadState*)getTlsValue(g_threadStateSlot);
	if (state)
	{
		delete state;
		setTlsValue(g_threadStateSlot, 0);
	}
}

#if (_PLH_OS_WIN)

void
NTAPI
tlsCallback(
	HANDLE hModule,
	DWORD reason,
	void* reserved
	)
{
	if (reason == DLL_THREAD_DETACH && g_threadStateSlot != -1)
		deleteCurrentThreadState();
}

#else

void
deleteThreadState(void* p)
{
	ThreadState* state = (ThreadState*)p;
	assert(state && "null in TLS-destructor");

	setTlsValue(g_threadDisableCountSlot, INT_MAX);  // compensate for possibly unbalanced enable calls
	delete state;
}

#endif

void
finalizeHooks()
{
	g_enableCount = INT_MIN / 2; // compensate for possibly unbalanced enable calls
	deleteCurrentThreadState();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initializeHooks()
{
	g_threadDisableCountSlot = createTlsSlot();

#if (_PLH_OS_WIN)
	g_threadStateSlot = createTlsSlot();
#else
	g_threadStateSlot = createDestructibleTlsSlot(deleteThreadState);
#endif

#if (_PLH_FINALIZE_HOOKS_AT_EXIT)
	::atexit(finalizeHooks);
#endif
}

size_t
getCurrentThreadDisableCount()
{
	return getTlsValue(g_threadDisableCountSlot);
}

bool
areHooksEnabled()
{
	return
		g_enableCount > 0 &&
		getCurrentThreadDisableCount() == 0;
}

void
enableHooks()
{
	std::once_flag once;
	std::call_once(once, initializeHooks);
	g_enableCount++;
}

void
disableHooks()
{
	g_enableCount--;
}

inline
void
incrementCurrentThreadDisableCount(intptr_t delta)
{
	size_t count = getTlsValue(g_threadDisableCountSlot);
	setTlsValue(g_threadDisableCountSlot, count + delta);
}

void
disableCurrentThreadHooks()
{
	incrementCurrentThreadDisableCount(1);
}

void
enableCurrentThreadHooks()
{
	incrementCurrentThreadDisableCount(-1);
}

ThreadState*
getCurrentThreadState(bool createIfNotExists)
{
	assert(getCurrentThreadDisableCount() && "thread hooks are not disabled (should be)");

	ThreadState* state = (ThreadState*)getTlsValue(g_threadStateSlot);
	if (state || !createIfNotExists)
		return state;

	state = new ThreadState;
	setTlsValue(g_threadStateSlot, (intptr_t)state);
	return state;
}

//..............................................................................

void
ThreadState::addFrame(
	HookCommonContext* context,
	size_t frameBase,
	size_t originalRet
	)
{
	std::pair<std::map<size_t, Frame>::iterator, bool> result = m_frameMap.emplace(frameBase, Frame());
	Frame* frame = &result.first->second;
	if (frame->m_ret.m_originalRet)
		frame->m_chainedRetStack.push_back(frame->m_ret);

	frame->m_ret.m_context = context;
	frame->m_ret.m_originalRet = originalRet;
	cleanup(result.first);
}

std::map<size_t, ThreadState::Frame>::iterator
ThreadState::findFrame(size_t frameBase)
{
#if (!_PLH_CPU_X86)
	return m_frameMap.find(frameBase);
#else // allowance for stdcall ret <n>
	std::map<size_t, Frame>::iterator it = m_frameMap.lower_bound(frameBase);
	if (!m_frameMap.empty() && (it == m_frameMap.end() || it->first != frameBase))
	{
		it--;
		assert(it == m_frameMap.end() || it->first < frameBase);
	}

	return it;
#endif
}

size_t
ThreadState::removeFrame(size_t frameBase)
{
	std::map<size_t, Frame>::iterator it = findFrame(frameBase);

	if (it == m_frameMap.end())
	{
		assert(false && "protolesshooks: FATAL ERROR: return address not found");
		return 0;
	}

	size_t originalRet = it->second.m_ret.m_originalRet;
	cleanup(it);

	if (it->second.m_chainedRetStack.empty())
		m_frameMap.erase(it);
	else
	{
		it->second.m_ret = it->second.m_chainedRetStack.back();
		it->second.m_chainedRetStack.pop_back();
	}

	return originalRet;
}

size_t
ThreadState::getOriginalRet(size_t frameBase)
{
	std::map<size_t, Frame>::iterator it = findFrame(frameBase);
	if (it == m_frameMap.end())
	{
		assert(false && "protolesshooks: FATAL ERROR: return address not found");
		return 0;
	}

	return it->second.m_ret.m_originalRet;
}

inline
void
callHookLeaveFunc(HookCommonContext* context)
{
	if (context->m_leaveFunc)
		context->m_leaveFunc(context->m_targetFunc, context->m_callbackParam, 0);
}

void
ThreadState::cleanup(const std::map<size_t, Frame>::iterator& it)
{
	// we may end up with abandoned frames (e.g., due to SEH or longjmp-s)
	// this loop cleans up all frames *above* `it` (or all if `it` is NULL)

	while (m_frameMap.begin() != it)
	{
		Frame* frame = &m_frameMap.begin()->second;

		size_t chainedRetCount = frame->m_chainedRetStack.size();
		for (intptr_t i = chainedRetCount - 1; i >= 0; i--)
			callHookLeaveFunc(frame->m_chainedRetStack[i].m_context);

		callHookLeaveFunc(frame->m_ret.m_context);
		m_frameMap.erase(m_frameMap.begin());
	}
}

//..............................................................................

} // namespace plh

#if (_PLH_OS_WIN)
#	define CRT_TLS_DESTRUCT_SECTION ".CRT$XLD"
#	pragma section(CRT_TLS_DESTRUCT_SECTION, long, read)

extern "C"
__declspec(allocate(CRT_TLS_DESTRUCT_SECTION))
PIMAGE_TLS_CALLBACK plh_tlsCallback = plh::tlsCallback;

#	ifdef _WIN64
#		pragma comment(linker, "/INCLUDE:_tls_used")
#		pragma comment(linker, "/INCLUDE:plh_tlsCallback")
#	else
#		pragma comment(linker, "/INCLUDE:__tls_used")
#		pragma comment(linker, "/INCLUDE:_plh_tlsCallback")
#	endif
#endif
