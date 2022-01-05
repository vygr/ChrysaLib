#ifndef FONT_H
#define FONT_H

#include "../settings.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <mutex>

struct font_data
{
	uint32_t m_ascent;
	uint32_t m_descent;
};

struct font_page
{
	uint32_t m_end;
	uint32_t m_start;
};

struct font_path
{
	fixed32_t m_width;
	uint32_t m_len;
};

struct font_path_element
{
	uint32_t m_type;
};

struct font_line_element : font_path_element
{
	fixed32_t m_x;
	fixed32_t m_y;
};

struct font_curve_element : font_path_element
{
	fixed32_t m_x1;
	fixed32_t m_y1;
	fixed32_t m_x2;
	fixed32_t m_y2;
};

struct glyph_size
{
	uint32_t m_w;
	uint32_t m_h;
};

const uint32_t font_max_word_cache = 1024;

class Font
{
public:
	Font(const std::string &name, uint32_t pixels);
	static std::shared_ptr<Font> open(const std::string &name, uint32_t pixels);
	font_path *glyph_data(uint32_t code);
	std::vector<uint32_t> glyph_ranges();
	std::vector<font_path*> glyph_info(const std::string &utf8);
	glyph_size glyph_bounds(const std::vector<font_path*> &info);
	uint32_t m_pixels = 0;
	std::string m_name;
	std::vector<uint8_t> m_data;
	static std::map<std::string, std::shared_ptr<Font>> m_cache;
	static std::recursive_mutex m_mutex;
};

#endif
