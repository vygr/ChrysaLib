#include "font.h"

std::vector<uint8_t> gulp(const std::string &filename);

std::recursive_mutex Font::m_mutex;
std::map<std::string, std::shared_ptr<Font>> Font::m_cache;

uint32_t read_utf8(uint8_t **data)
{
	return *(*data)++;
}

Font::Font(const std::string &name, uint32_t pixels)
	: m_name(name)
	, m_pixels(pixels)
{
	//gulp font file into data vector
	m_data = gulp(name);
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

font_path *Font::glyph_data(uint32_t code)
{
	auto data = &m_data[0] + sizeof(font_data);
	for (;;)
	{
		auto end = ((font_page*)data)->m_end;
		if (!end) break;
		auto start = ((font_page*)data)->m_start;
		data += sizeof(font_page);
		if (code >= start && code <= end)
		{
			data = &m_data[0] + ((uint32_t*)data)[code - start];
			return (font_path*)data;
		}
		data += (end - start + 1) * sizeof(int32_t);
	}
	return nullptr;
}

std::vector<uint32_t> Font::glyph_ranges()
{
	auto ranges = std::vector<uint32_t>{};
	auto data = &m_data[0] + sizeof(font_data);
	for (;;)
	{
		auto end = ((font_page*)data)->m_end;
		if (!end) break;
		auto start = ((font_page*)data)->m_start;
		ranges.push_back(start);
		ranges.push_back(end);
		data += (end - start + 1) * sizeof(int32_t) + sizeof(font_page);
	}
	return ranges;
}

std::vector<font_path*> Font::glyph_info(const std::string &utf8)
{
	auto info = std::vector<font_path*>{};
	info.reserve(utf8.size());
	auto data = (uint8_t*)&utf8[0];
	for (;;)
	{
		auto code = read_utf8(&data);
		if (!code) break;
		info.push_back(glyph_data(code));
	}
	return info;
}

glyph_size Font::glyph_bounds(const std::vector<font_path*> &info)
{
	auto ranges = std::vector<uint32_t>{};
	auto data = &m_data[0];
	auto h = ((font_data*)data)->m_ascent;
	auto gap = ((font_data*)data)->m_descent;
	h += gap;
	gap = h;
	gap >>= 4;
	auto w = gap;
	for (auto font_path : info)
	{
		int32_t gw;
		if (font_path) gw = font_path->m_width;
		else gw = h >> 4;
		w += gw + gap;
	}
	w *= m_pixels;
	h *= m_pixels;
	w >>= 24;
	h >>= 24;
	w++;
	return glyph_size{w, h};
}
