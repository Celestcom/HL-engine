#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/log/trivial.hpp>
#include "SharedTypes.h"
template<typename T> 
class ReadableSharedVector
{

public:
	using ptr_t = boost::interprocess::offset_ptr<void, boost::int32_t, boost::uint64_t>;
	using my_managed_shared_memory = boost::interprocess::basic_managed_shared_memory<
		char,
		boost::interprocess::rbtree_best_fit<boost::interprocess::mutex_family, ptr_t>,
		boost::interprocess::iset_index>;


	using TAlloc = boost::interprocess::allocator<T, my_managed_shared_memory::segment_manager>;
	using TVector = boost::interprocess::vector<T, TAlloc>;
	using MutexGuard = boost::interprocess::scoped_lock<boost::interprocess::named_mutex>;

	ReadableSharedVector(const std::string& memName, const std::string& vecName) :

		m_memName(memName),
		m_vecName(vecName),
		m_segment(),
		m_vector{ nullptr },
		m_mutexName(memName + "-mutex"),
		m_mutex(boost::interprocess::open_only, m_mutexName.c_str())
		
	{
		m_segment = my_managed_shared_memory(boost::interprocess::open_only, m_memName.c_str());
		//if (!m_segment.check_sanity()) {
		//	throw boost::interprocess::interprocess_exception("Failed to open the shared memory for tracking");
		//}
		m_vector = m_segment.find<TVector>(m_vecName.c_str()).first;
		if (m_vector == 0) {
			throw boost::interprocess::interprocess_exception("Failed to construct ReadableSharedVector memory");
		}


		assert(0 == strcmp(my_managed_shared_memory::get_instance_name(m_vector), m_vecName.c_str()));
		assert(1 == my_managed_shared_memory::get_instance_length(m_vector));
	
	}

	std::size_t Size() const {
		MutexGuard guard(m_mutex);
		assert(m_vector != nullptr);
		return m_vector->size();
	}

	T Get(std::size_t index) const {
		MutexGuard guard(m_mutex);


		assert(m_vector != nullptr);

		return m_vector->at(static_cast<TVector::size_type>(index));
	}
	boost::optional<std::size_t> Find(std::function<bool(const T& item)> predicate)  const {
		MutexGuard guard(m_mutex);

		const auto it = std::find_if(m_vector->cbegin(), m_vector->cend(), predicate);
		if (it != m_vector->cend()) {
			return it - m_vector->cbegin();
		}
		else {
			return boost::none;
		}
	}

	boost::optional<std::size_t> Find(const T& item) const {
		MutexGuard guard(m_mutex);

		assert(m_vector != nullptr);

		std::size_t pos = std::find(m_vector->cbegin(), m_vector->cend(), item) - m_vector->cbegin();
		if (pos < m_vector->size()) {
			return pos;
		}
		else {
			return boost::none;
		}
	}




private:
	std::string m_memName;
	std::string m_vecName;
	std::string m_mutexName;
	my_managed_shared_memory m_segment;
	TVector* m_vector;
	mutable boost::interprocess::named_mutex m_mutex;

	

};

