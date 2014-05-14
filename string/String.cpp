#include "String.hpp"
#include "../memory/PoolAllocator.hpp"
#include <stdint.h>

#ifdef _WIN32
#	pragma warning(disable: 4351)
#endif

namespace sprawl
{
	String::Holder::Holder()
		: m_staticData()
		, m_dynamicData(nullptr)
		, m_data(nullptr)
		, m_refCount(1)
		, m_length(0)
	{
		//
	}

	String::Holder::Holder(const char* data)
		: m_staticData()
		, m_dynamicData(nullptr)
		, m_data(nullptr)
		, m_refCount(1)
		, m_length(strlen(data))
	{
		if( m_length < staticDataSize )
		{
			memcpy(m_staticData, data, m_length);
			m_staticData[m_length] = '\0';
			m_data = m_staticData;
		}
		else
		{
			m_dynamicData = new char[m_length+1];
			memcpy(m_dynamicData, data, m_length);
			m_dynamicData[m_length] = '\0';
			m_data = m_dynamicData;
		}
	}

	String::Holder::Holder(const char* data, size_t length)
		: m_staticData()
		, m_dynamicData(nullptr)
		, m_data(nullptr)
		, m_refCount(1)
		, m_length(length)
	{
		if( m_length < staticDataSize )
		{
			memcpy(m_staticData, data, m_length);
			m_staticData[m_length] = '\0';
			m_data = m_staticData;
		}
		else
		{
			m_dynamicData = new char[m_length+1];
			memcpy(m_dynamicData, data, m_length);
			m_dynamicData[m_length] = '\0';
			m_data = m_dynamicData;
		}
	}

	String::Holder::Holder(const StringLiteral& literal)
		: m_staticData()
		, m_dynamicData(nullptr)
		, m_data(literal.m_ptr)
		, m_refCount(1)
		, m_length(literal.m_length)
	{
		//
	}

	void String::Holder::IncRef()
	{
		++m_refCount;
	}

	bool String::Holder::DecRef()
	{
		return (--m_refCount == 0);
	}

	String::Holder* String::Holder::CreateHolder()
	{
		typedef memory::DynamicPoolAllocator<sizeof(String::Holder)> holderAlloc;

		return (String::Holder*)holderAlloc::alloc();

	}

	void String::Holder::FreeHolder(Holder* holder)
	{
		typedef memory::DynamicPoolAllocator<sizeof(String::Holder)> holderAlloc;

		holder->~Holder();
		holderAlloc::free(holder);
	}

	String::Holder::~Holder()
	{
		if(m_dynamicData)
		{
			delete[] m_dynamicData;
		}
	}

	String::String()
		: m_holder(nullptr)
	{

	}

	String::String(const char* const data)
		: m_holder(Holder::CreateHolder())
	{
		new (m_holder) Holder(data);
	}

	String::String(const char* const data, size_t length)
		: m_holder(Holder::CreateHolder())
	{
		new (m_holder) Holder(data, length);
	}

	String::String(const String& other)
		: m_holder(other.m_holder)
	{
		if(m_holder)
		{
			m_holder->IncRef();
		}
	}

	String::String(String&& other)
		: m_holder(other.m_holder)
	{
		//Don't change ref count here, it's staying the same because it's being *moved*
		other.m_holder = nullptr;
	}

	String::String(const std::string& stlString)
		: m_holder(Holder::CreateHolder())
	{
		new (m_holder) Holder(stlString.c_str(), stlString.length());
	}	

	String::String(const StringLiteral& stringLiteral)
		: m_holder(Holder::CreateHolder())
	{
		new (m_holder) Holder(stringLiteral);
	}

	String::~String()
	{
		if(m_holder && m_holder->DecRef())
		{
			Holder::FreeHolder(m_holder);
		}
	}
	
	sprawl::String String::operator+(const sprawl::String& other) const
	{
		sprawl::String ret;
		ret.m_holder = Holder::CreateHolder();
		new (ret.m_holder) Holder();
		size_t fullLength = m_holder->m_length + other.m_holder->m_length;
		if(fullLength < Holder::staticDataSize)
		{
			memcpy(ret.m_holder->m_staticData, m_holder->m_data, m_holder->m_length);
			memcpy(ret.m_holder->m_staticData + m_holder->m_length, other.m_holder->m_data, other.m_holder->m_length);
			ret.m_holder->m_staticData[fullLength] = '\0';
			ret.m_holder->m_data = ret.m_holder->m_staticData;
		}
		else
		{
			ret.m_holder->m_dynamicData = new char[fullLength+1];
			memcpy(ret.m_holder->m_dynamicData, m_holder->m_data, m_holder->m_length);
			memcpy(ret.m_holder->m_dynamicData + m_holder->m_length, other.m_holder->m_data, other.m_holder->m_length);
			ret.m_holder->m_dynamicData[fullLength] = '\0';
			ret.m_holder->m_data = ret.m_holder->m_dynamicData;
		}
		return std::move(ret);
	}
	
	String& String::operator=(const String& other)
	{
		if(m_holder && m_holder->DecRef())
		{
			Holder::FreeHolder(m_holder);
		}

		m_holder = other.m_holder;

		if(m_holder)
		{
			m_holder->IncRef();
		}
		return *this;
	}
	
	bool String::operator<(const String& other) const
	{
		if(m_holder == other.m_holder)
		{
			return false;
		}
		if(!m_holder)
		{
			return false;
		}
		if(!other.m_holder)
		{
			return true;
		}
		size_t length = other.m_holder->m_length < m_holder->m_length ? other.m_holder->m_length : m_holder->m_length;
		const char* const left = m_holder->m_data;
		const char* const right = other.m_holder->m_data;
		for(size_t i = 0; i < length; ++i)
		{
			if(left[i] == right[i])
				continue;

			return left[i] < right[i];
		}
		return false;
	}

	std::string String::toStdString() const
	{
		if(!m_holder)
		{
			static std::string emptyStr;
			return emptyStr;
		}
		return std::string(m_holder->m_data, m_holder->m_length);
	}
}