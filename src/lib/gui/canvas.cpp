#include "canvas.h"
#include "pixmap.h"
#include "texture.h"
#include "property.h"
#include "ctx.h"
#include "colors.h"

Canvas::Canvas(int32_t w, int32_t h, int32_t scale)
	: View()
	, m_cx1(w * scale)
	, m_cy1(h * scale)
	, m_scale(scale)
	, m_col(argb_black)
	, m_pixmap(std::make_shared<Pixmap>(w * scale, h * scale))
{
	m_w = w;
	m_h = h;
}

Canvas::Canvas(std::shared_ptr<Pixmap> pixmap)
	: View()
	, m_cx1(pixmap->m_w)
	, m_cy1(pixmap->m_h)
	, m_scale(1)
	, m_col(argb_black)
	, m_pixmap(pixmap)
	, m_texture(pixmap->m_texture)
{
	m_w = m_cx1;
	m_h = m_cy1;
}

view_size Canvas::pref_size()
{
	return view_size{m_pixmap->m_w / m_scale, m_pixmap->m_h / m_scale};
}

Canvas *Canvas::swap()
{
	//scale the pixmap if needed
	auto pixmap = m_pixmap;
	if (m_scale != 1)
	{
		pixmap = std::make_shared<Pixmap>(m_pixmap->m_w / m_scale, m_pixmap->m_h / m_scale);
		//pixmap.resize(m_pixmap);
	}

	//upload then grab the texture
	pixmap->upload();
	m_texture = pixmap->m_texture;
	dirty();
	return this;
}

Canvas *Canvas::draw(const Ctx &ctx)
{
	//allready locked by GUI thread
	if (m_texture)
	{
		auto offset_x = get_long_prop("offset_x");
		auto offset_y = get_long_prop("offset_y");
		if (!offset_x) offset_x = (m_w - m_texture->m_w) >> 1;
		if (!offset_y) offset_y = (m_h - m_texture->m_h) >> 1;
		ctx.blit(m_texture->m_handle, argb_white, offset_x, offset_y, m_texture->m_w, m_texture->m_h);
	}
	return this;
}

Canvas *Canvas::set_clip(int32_t x, int32_t y, int32_t x1, int32_t y1)
{
	auto pw = m_pixmap->m_w;
	auto ph = m_pixmap->m_h;
	if (x <= x1 && y <= y1 && x < pw && y < ph && x1 > 0 && y1 > 0)
	{
		m_cx = std::max(x, 0);
		m_cy = std::max(y, 0);
		m_cx1 = std::min(x1, pw);
		m_cy1 = std::min(y1, ph);
	}
	return this;
}

Canvas *Canvas::plot(int32_t x, int32_t y)
{
	if (x >= m_cx && y >= m_cy && x < m_cx1 && y < m_cy1)
	{
		auto col = Pixmap::to_premul(m_col);
		auto alpha = col >> 24;
		if (alpha != 0)
		{
			auto pixel = &m_pixmap->m_data[y * m_pixmap->m_w + x];
			if (alpha != 0xff)
			{
				alpha = 0xff - alpha;
				auto dcol = *pixel;
				auto dag = (((dcol & 0xff00ff00) * alpha) >> 8) & 0xff00ff00;
				auto drb = (((dcol & 0x00ff00ff) * alpha) >> 8) & 0x00ff00ff;
				col = col + dag + dag;
			}
			*pixel = col;
		}
	}
	return this;
}

uint32_t Canvas::pick(int32_t x, int32_t y)
{
	if (x >= m_cx && y >= m_cy && x < m_cx1 && y < m_cy1)
	{
		return Pixmap::to_argb(m_pixmap->m_data[y * m_pixmap->m_w + x]);
	}
	return 0;
}

Canvas *Canvas::span_noclip(int32_t coverage, int32_t x, int32_t y, int32_t x1)
{
	if (coverage != 0)
	{
		auto alpha = ((m_col >> 24) * coverage) >> 7;
		if (alpha != 0)
		{
			auto col = Pixmap::to_premul((m_col & 0xffffff) + (alpha << 24));
			auto pix_begin = &m_pixmap->m_data[y * m_pixmap->m_w];
			auto pix_end = pix_begin + x1;
			pix_begin += x;
			if (alpha == 0xff)
			{
				std::fill(pix_begin, pix_end, col);
			}
			else
			{
				alpha = 0xff - alpha;
				do
				{
					auto dcol = *pix_begin;
					auto dag = (((dcol & 0xff00ff00) * alpha) >> 8) & 0xff00ff00;
					auto drb = (((dcol & 0x00ff00ff) * alpha) >> 8) & 0x00ff00ff;
					dcol = col + dag + dag;
					*pix_begin++ = dcol;
				} while (pix_begin != pix_end);
			}
		}
	}
	return this;
}

Canvas *Canvas::span(int32_t coverage, int32_t x, int32_t y, int32_t x1)
{
	if (x1 > x && x1 > m_cx && y > m_cy && x < m_cx1 && y < m_cy1)
	{
		x = std::max(x, m_cx);
		x1 = std::min(x1, m_cx1);
		span_noclip(coverage, x, y, x1);
	}
	return this;
}
