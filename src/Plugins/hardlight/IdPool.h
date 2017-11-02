#pragma once

#include <array>
#include <vector>
class IdPool {
public:
	IdPool();
	std::size_t Request();
	void Release(std::size_t);
private:
	std::size_t m_upperBound;
	std::vector<std::size_t> m_pool;
};