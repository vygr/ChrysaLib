#ifndef FONT_H
#define FONT_H

#include "../settings.h"
#include "path.h"
#include "texture.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <mutex>

struct font_data
{
	fixed32_t m_ascent;
	fixed32_t m_descent;
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

struct font_line_element : public font_path_element
{
	Vec2f m_p;
};

struct font_curve_element : public font_line_element
{
	Vec2f m_p1;
	Vec2f m_p2;
};

struct glyph_size
{
	uint32_t m_w = 0;
	uint32_t m_h = 0;
};

struct font_metrics
{
	uint32_t m_ascent = 0;
	uint32_t m_descent = 0;
	uint32_t m_height = 0;
};

const uint32_t font_max_word_cache = 1024;

class Font
{
public:
	Font(uint8_t *data, uint32_t pixels);
	static std::shared_ptr<Font> open(const std::string &name, uint32_t pixels);
	font_metrics get_metrics();
	font_path *glyph_data(uint32_t code);
	std::vector<uint32_t> glyph_ranges();
	std::vector<font_path*> glyph_info(const std::string &utf8);
	glyph_size glyph_bounds(const std::vector<font_path*> &info);
	std::vector<Path> glyph_paths(const std::vector<font_path*> &info, glyph_size &size);
	std::shared_ptr<Texture> sym_texture(const std::string &utf8);
	uint32_t m_pixels = 0;
	uint8_t *m_data = nullptr;
	std::map<std::string, std::shared_ptr<Texture>> m_sym_map;
	static std::map<std::pair<std::string, uint32_t>, std::shared_ptr<Font>> m_cache_font;
	static std::map<std::string, std::vector<uint8_t>> m_cache_data;
	static std::recursive_mutex m_mutex;
};

#endif
