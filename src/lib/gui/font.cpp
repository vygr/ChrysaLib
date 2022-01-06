#include "font.h"
#include "canvas.h"
#include "colors.h"

std::vector<uint8_t> gulp(const std::string &filename);
uint32_t from_utf8(uint8_t **data);

std::recursive_mutex Font::m_mutex;
std::map<std::string, std::shared_ptr<Font>> Font::m_cache;

Font::Font(const std::string &name, uint32_t pixels)
	: m_name(name)
	, m_pixels(pixels)
	, m_data(gulp(name))
{}

std::shared_ptr<Font> Font::open(const std::string &name, uint32_t pixels)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto itr = m_cache.find(name);
	if (itr != end(m_cache)) return itr->second;
	auto font = std::make_shared<Font>(name, pixels);
	m_cache[name] = font;
	return font;
}

font_metrics Font::get_metrics()
{
	auto data = (font_data*)&m_data[0];
	fixed64_t ascent = data->m_ascent * m_pixels;
	fixed64_t descent = data->m_descent * m_pixels;
	fixed64_t height = ascent + descent;
	return font_metrics{(uint32_t)(ascent >> 24), (uint32_t)(descent >> 24), (uint32_t)(height >> 24)};
}

font_path *Font::glyph_data(uint32_t code)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
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
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
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
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto info = std::vector<font_path*>{};
	info.reserve(utf8.size());
	auto data = (uint8_t*)&utf8[0];
	for (;;)
	{
		auto code = from_utf8(&data);
		if (!code) break;
		info.push_back(glyph_data(code));
	}
	return info;
}

glyph_size Font::glyph_bounds(const std::vector<font_path*> &info)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto data = (font_data*)&m_data[0];
	fixed64_t h = data->m_ascent;
	fixed64_t gap = data->m_descent;
	h += gap;
	gap = h;
	gap >>= 4;
	fixed64_t w = gap;
	for (auto font_path : info)
	{
		fixed64_t gw;
		if (font_path) gw = font_path->m_width;
		else gw = h >> 4;
		w += gw + gap;
	}
	w *= m_pixels;
	h *= m_pixels;
	w >>= 24;
	h >>= 24;
	w++;
	return glyph_size{(uint32_t)w, (uint32_t)h};
}

std::vector<Path> Font::glyph_paths(const std::vector<font_path*> &info, glyph_size &size)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto paths = std::vector<Path>{};
	auto data = (font_data*)&m_data[0];
	const fixed64_t height = data->m_ascent + data->m_descent;
	const fixed64_t gap = height >> 4;
	const fixed32_t eps = 1 << (FP_SHIFT - 2);
	const auto pixels = m_pixels;
	fixed64_t ox = gap;
	fixed64_t oy = 0;
	Path *p;
	for (auto font_path : info)
	{
		if (font_path)
		{
			fixed64_t width = font_path->m_width;
			auto font_data = ((uint8_t*)font_path) + sizeof(font_path);
			auto font_data_end = font_data + font_path->m_len;
			auto pos_x = ox;
			auto pos_y = oy;
			while (font_data != font_data_end)
			{
				auto font_line = (font_line_element*)font_data;
				auto element_type = font_line->m_type;
				fixed64_t x = font_line->m_x;
				fixed64_t y = font_line->m_y;
				x = ((x + ox) * pixels) >> 7;
				y = ((y + oy) * pixels) >> 7;
				switch (element_type)
				{
				case 2:
				{
					//curve to
					p->pop_back();
					auto font_curve = (font_curve_element*)font_data;
					fixed64_t x1 = font_curve->m_x1;
					fixed64_t y1 = font_curve->m_y1;
					fixed64_t x2 = font_curve->m_x2;
					fixed64_t y2 = font_curve->m_y2;
					x1 = ((x1 + ox) * pixels) >> 7;
					y1 = ((y1 + oy) * pixels) >> 7;
					x2 = ((x2 + ox) * pixels) >> 7;
					y2 = ((y2 + oy) * pixels) >> 7;
					p->gen_cubic(pos_x, pos_y, x, y, x1, y1, x2, y2, eps);
					pos_x = x2;
					pos_y = y2;
					font_data += sizeof(font_curve_element);
					break;
				}
				case 1:
				{
					//line to
					p->push_back(x, y);
					pos_x = x;
					pos_y = y;
					font_data += sizeof(font_line_element);
					break;
				}
				case 0:
				{
					//move to
					paths.emplace_back(Path());
					p = &paths.back();
					p->push_back(x, y);
					pos_x = x;
					pos_y = y;
					font_data += sizeof(font_line_element);
					break;
				}
				}
			}
			ox += width + gap;
		}
		else
		{
			ox += (height >> 4) + gap;
		}
	}
	size.m_w = (ox * pixels >> 24) + 1;
	size.m_h = height * pixels >> 24;
	return paths;
}

std::shared_ptr<Texture> Font::sym_texture(const std::string &utf8)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	//look up string in sym map
	auto itr = m_sym_map.find(utf8);
	if (itr != end(m_sym_map)) return itr->second;

	//create sym canvas
	auto info = glyph_info(utf8);
	glyph_size size;
	auto paths = glyph_paths(info, size);
	auto sym_canvas = std::make_shared<Canvas>(size.m_w, size.m_h, 2);
	sym_canvas->set_col(argb_white);
	sym_canvas->set_canvas_flags(canvas_flag_antialias);
	auto metrics = get_metrics();
	sym_canvas->fpoly(paths, 0, metrics.m_ascent << (FP_SHIFT + 1), winding_odd_even);

	//take texture from canvas
	sym_canvas->swap();
	m_sym_map[utf8] = sym_canvas->m_texture;
	return sym_canvas->m_texture;
}
