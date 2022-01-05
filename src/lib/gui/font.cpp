#include "font.h"
#include <fstream>

std::recursive_mutex Font::m_mutex;
std::map<std::string, std::shared_ptr<Font>> Font::m_cache;

Font::Font(const std::string &name, uint32_t pixels)
	: m_name(name)
	, m_pixels(pixels)
{
	//slurp font file into data vector
	std::ifstream file(name, std::ios::in | std::ios::binary);
	m_data.insert(begin(m_data), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
}

std::shared_ptr<Font> Font::open(const std::string &name, uint32_t pixels)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto itr = m_cache.find(name);
	if (itr != end(m_cache)) return itr->second;
	auto font = std::make_shared<Font>(name, pixels);
	m_cache[name] = font;
	return font;
}
