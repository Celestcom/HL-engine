#pragma once
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
template<typename KeyType, typename MappedType>
class ReadableSharedMap
{

public:

	ReadableSharedMap(const std::string& name) :
		m_dataMemName(name),
		m_segment(boost::interprocess::open_only, (name + "-data").c_str()),
		m_allocInst(m_segment.get_segment_manager())
	{

		m_map = m_segment.find<Map>(name.c_str()).first;
		if (m_map == 0) {
			throw boost::interprocess::interprocess_exception("Failed to construct ReadableSharedMap memory");
		}
	
	}

	std::size_t Size() const {
		return m_map->size();
	}
	bool Contains(KeyType key) {
		return m_map->find(key) != m_map->end();
	}

	MappedType Get(KeyType key) {
		return m_map->at(key);
	}

private:
	using ptr_t = boost::interprocess::offset_ptr<void, boost::int32_t, boost::uint64_t>;
	using my_managed_shared_memory = boost::interprocess::basic_managed_shared_memory<
		char,
		boost::interprocess::rbtree_best_fit<boost::interprocess::mutex_family, ptr_t>,
		boost::interprocess::iset_index>;

	using ValueType = std::pair<const KeyType, MappedType>;

	std::string m_dataMemName;
	using ShmemAllocator = boost::interprocess::allocator<ValueType, my_managed_shared_memory::segment_manager>;

	using Map = boost::interprocess::map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator>;
	my_managed_shared_memory m_segment;
	ShmemAllocator m_allocInst;
	boost::interprocess::offset_ptr<Map> m_map;
};


