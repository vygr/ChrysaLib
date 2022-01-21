#include "font.h"
#include "canvas.h"
#include "colors.h"

std::vector<uint8_t> gulp(const std::string &filename);
uint32_t from_utf8(uint8_t **data);

std::recursive_mutex Font::m_mutex;
std::map<std::pair<std::string, uint32_t>, std::shared_ptr<Font>> Font::m_cache_font;
std::map<std::string, std::vector<uint8_t>> Font::m_cache_data;

Font::Font(uint8_t *data, uint32_t pixels)
	: m_data(data)
	, m_pixels(pixels)
{}

font_metrics Font::get_metrics()
{
	auto data = (font_data*)m_data;
	fixed64_t ascent = data->m_ascent * m_pixels;
	fixed64_t descent = data->m_descent * m_pixels;
	fixed64_t height = ascent + descent;
	return font_metrics{uint32_t(ascent) >> 8, uint32_t(descent) >> 8, uint32_t(height) >> 8};
}

font_path *Font::glyph_data(uint32_t code)
{
	auto data = m_data + sizeof(font_data);
	for (;;)
	{
		auto end = ((font_page*)data)->m_end;
		if (!end) break;
		auto start = ((font_page*)data)->m_start;
		data += sizeof(font_page);
		if (code >= start && code <= end)
		{
			data = m_data + ((uint32_t*)data)[code - start];
			return (font_path*)data;
		}
		data += (end - start + 1) * sizeof(int32_t);
	}
	return nullptr;
}

std::vector<uint32_t> Font::glyph_ranges()
{
	auto ranges = std::vector<uint32_t>{};
	auto data = m_data + sizeof(font_data);
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
		auto code = from_utf8(&data);
		if (!code) break;
		info.push_back(glyph_data(code));
	}
	return info;
}

glyph_size Font::glyph_bounds(const std::vector<font_path*> &info)
{
	auto data = (font_data*)m_data;
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
	return glyph_size{(uint32_t(w) >> 8) + 1, uint32_t(h) >> 8};
}

std::vector<Path> Font::glyph_paths(const std::vector<font_path*> &info, glyph_size &size)
{
	auto paths = std::vector<Path>{};
	auto data = (font_data*)m_data;
	const fixed64_t height = data->m_ascent + data->m_descent;
	const fixed64_t gap = height >> 4;
	const fixed32_t eps = 0.25;
	const auto pixels = m_pixels;
	auto o = Vec2F(gap);
	Path *pth;
	for (auto font_path : info)
	{
		if (font_path)
		{
			fixed64_t width = font_path->m_width;
			auto font_data = ((uint8_t*)font_path) + sizeof(font_path);
			auto font_data_end = font_data + font_path->m_len;
			auto pos = o;
			while (font_data != font_data_end)
			{
				auto font_line = (font_line_element*)font_data;
				auto element_type = font_line->m_type;
				auto p = Vec2F(font_line->m_p);
				p = ((p + o) * pixels).asr(7);
				switch (element_type)
				{
				case 2:
				{
					//curve to
					pth->pop_back();
					auto font_curve = (font_curve_element*)font_data;
					auto p1 = Vec2F(font_curve->m_p1);
					auto p2 = Vec2F(font_curve->m_p2);
					p1 = ((p1 + o) * pixels).asr(7);
					p2 = ((p2 + o) * pixels).asr(7);
					pth->gen_cubic(pos, p, p1, p2, eps);
					pos = p2;
					font_data += sizeof(font_curve_element);
					break;
				}
				case 1:
				{
					//line to
					pth->push_back(p);
					pos = p;
					font_data += sizeof(font_line_element);
					break;
				}
				case 0:
				{
					//move to
					paths.emplace_back(Path());
					pth = &paths.back();
					pth->push_back(p);
					pos = p;
					font_data += sizeof(font_line_element);
					break;
				}
				}
			}
			o.m_x += width + gap;
		}
		else
		{
			o.m_x += (height >> 4) + gap;
		}
	}
	size.m_w = (uint32_t(o.m_x * pixels) >> 8) + 1;
	size.m_h = uint32_t(height * pixels) >> 8;
	return paths;
}

std::shared_ptr<Font> Font::open(const std::string &name, uint32_t pixels)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto key = std::make_pair<>(name, pixels);
	auto itr_font = m_cache_font.find(key);
	if (itr_font != end(m_cache_font)) return itr_font->second;
	auto itr_data = m_cache_data.find(name);
	if (itr_data == end(m_cache_data)) m_cache_data[name] = gulp(name);
	auto font = std::make_shared<Font>(&m_cache_data[name][0], pixels);
	m_cache_font[key] = font;
	return font;
}

std::shared_ptr<Texture> Font::sym_texture(const std::string &utf8)
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
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
	sym_canvas->fpoly(paths, Vec2f(0, metrics.m_ascent * 2), winding_odd_even);

	//take texture from canvas
	sym_canvas->swap();
	m_sym_map[utf8] = sym_canvas->m_texture;
	return sym_canvas->m_texture;
}
