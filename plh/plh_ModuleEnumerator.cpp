#include "plh_ModuleEnumerator.h"

#if (_PLH_OS_WIN)
#	include <codecvt>
#	include <psapi.h>
#elif (_PLH_OS_LINUX)
#	include <linux/limits.h>
#elif (_PLH_OS_DARWIN)
#	include <mach-o/dyld.h>
#endif

namespace plh {

//..............................................................................

#if (_PLH_OS_WIN)

ModuleIterator::ModuleIterator(std::shared_ptr<std::vector<HMODULE> >&& moduleArray)
{
	m_moduleArray = moduleArray;
	m_index = 0;
}

ModuleIterator&
ModuleIterator::operator ++ ()
{
	if (!m_moduleArray || m_index >= m_moduleArray->size())
		return *this;

	m_index++;
	m_moduleFileName.clear();
	return *this;
}

const char*
ModuleIterator::prepareModuleFileName() const
{
	assert(!m_moduleFileName.length());

	if (!m_moduleArray || m_index >= m_moduleArray->size())
		return NULL;

	enum
	{
		BuferLength = 1024,
	};

	HMODULE module = m_moduleArray->at(m_index);
	wchar_t fileName[BuferLength];
	fileName[BuferLength - 1] = 0;
	::GetModuleFileNameW(module, fileName, BuferLength - 1);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	m_moduleFileName = convert.to_bytes(fileName);
	return m_moduleFileName.c_str();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
enumerateModules(ModuleIterator* iterator)
{
	std::shared_ptr<std::vector<HMODULE> > moduleArray = std::make_shared<std::vector<HMODULE> >();

	for (;;)
	{
		size_t currentCount = moduleArray->size();
		DWORD requiredSize;

		::EnumProcessModules(
			::GetCurrentProcess(),
			moduleArray->data(),
			(DWORD)currentCount * sizeof(HMODULE),
			&requiredSize
			);

		size_t requiredCount = requiredSize / sizeof(HMODULE);
		if (requiredCount <= currentCount)
			break;

		moduleArray->resize(requiredCount);
	}

	*iterator = ModuleIterator(std::move(moduleArray));
	return true;
}

#elif (_PLH_OS_LINUX)

ModuleIterator&
ModuleIterator::operator ++ ()
{
	if (m_linkMap)
	{
		m_linkMap = m_linkMap->l_next;
		m_moduleFileName = NULL;
	}

	return *this;
}

const char*
ModuleIterator::prepareModuleFileName() const
{
	if (!m_linkMap)
		return m_moduleFileName;

	m_moduleFileName = m_linkMap->l_name;
	if (m_moduleFileName && m_moduleFileName[0])
		return m_moduleFileName;

	static char exeFilePath[PATH_MAX] = { 0 };
	if (!exeFilePath[0])
		::readlink("/proc/self/exe", exeFilePath, sizeof(exeFilePath) - 1);

	m_moduleFileName = exeFilePath;
	return m_moduleFileName;
}

bool
enumerateModules(ModuleIterator* iterator)
{
	*iterator = ModuleIterator(_r_debug.r_map);
	return true;
}

#elif (_PLH_OS_DARWIN)

ModuleIterator::ModuleIterator(size_t count)
{
	m_moduleFileName = NULL;
	m_count = count;
	m_index = 0;
}

ModuleIterator&
ModuleIterator::operator ++ ()
{
	if (m_index >= m_count)
		return *this;

	m_index++;
	m_moduleFileName = NULL;
	return *this;
}

void*
ModuleIterator::prepareModule() const
{
	assert(!m_module.isOpen() && "module handle is already set");

	if (m_index < m_count)
		m_module.open(getModuleFileName(), RTLD_NOLOAD);

	return m_module;
}

const char*
ModuleIterator::prepareModuleFileName() const
{
	assert(!m_moduleFileName && "module file name is already set");

	if (m_index < m_count)
		m_moduleFileName = ::_dyld_get_image_name(m_index);

	return m_moduleFileName;
}

bool
enumerateModules(ModuleIterator* iterator)
{
	*iterator = ModuleIterator(_dyld_image_count());
	return true;
}

#endif

//..............................................................................

ModuleIterator
ModuleIterator::operator ++ (int)
{
	ModuleIterator it = *this;
	operator ++ ();
	return it;
}

//..............................................................................

} // namespace plh
