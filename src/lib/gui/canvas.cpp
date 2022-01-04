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
				auto dag = (uint32_t)(((uint64_t)(dcol & 0xff00ff00) * alpha) >> 8) & 0xff00ff00;
				auto drb = (((dcol & 0x00ff00ff) * alpha) >> 8) & 0x00ff00ff;
				col = col + dag + drb;
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

Canvas *Canvas::fbox(int32_t x, int32_t y, int32_t w, int32_t h)
{
	if (w > 0 && h > 0)
	{
		w += x;
		h += y;
		if (w > m_cx && h > m_cy && x < m_cx1 && y < m_cy1)
		{
			x = std::max(x, m_cx);
			y = std::max(y, m_cy);
			w = std::min(w, m_cx1);
			h = std::min(h, m_cy1);
			do { span_noclip(0x80, x, y, w); } while (++y < h);
		}
	}
	return this;
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
					auto dag = (uint32_t)(((uint64_t)(dcol & 0xff00ff00) * alpha) >> 8) & 0xff00ff00;
					auto drb = (((dcol & 0x00ff00ff) * alpha) >> 8) & 0x00ff00ff;
					dcol = col + dag + drb;
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

edge_bounds Canvas::set_edges(const std::vector<std::vector<int32_t>> &polygons, int32_t x, int32_t y, int32_t scale)
{
	edge_bounds bounds;
	m_edges.clear();
	x += 1 << (FP_SHIFT - 1);
	y += 1 << (FP_SHIFT - 1);
	auto cy = m_cy * scale;
	auto cy1 = m_cy1 * scale;
	for (const auto &path : polygons)
	{
		auto len = path.size();
		auto x2 = path[len - 2] + x;
		auto y2 = (path[len - 1] + y) * scale >> FP_SHIFT;
		for (auto i = 0; i < len;)
		{
			auto x1 = x2;
			auto y1 = y2;
			x2 = path[i++] + x;
			y2 = (path[i++] + y) * scale >> FP_SHIFT;
			bounds.m_min_x = std::min(bounds.m_min_x, x2 >> FP_SHIFT);
			bounds.m_max_x = std::max(bounds.m_max_x, x2 >> FP_SHIFT);
			if (y1 == y2) continue;
			if (y1 <= y2)
			{
				if (y2 <= cy) continue;
				auto dda = (x2 - x1) / (y2 - y1);
				if (y1 < cy)
				{
					x1 += ((cy - y1) * dda);
					y1 = cy;
				}
				m_edges.emplace_back(Edge(x1, y1, y2, 1, dda));
				bounds.m_min_y = std::min(bounds.m_min_y, y1);
				bounds.m_max_y = std::max(bounds.m_max_y, y2);
			}
			else
			{
				if (y1 >= cy1) continue;
				auto dda = (x1 - x2) / (y1 - y2);
				if (y2 < cy)
				{
					x2 += ((cy - y2) * dda);
					y2 = cy;
				}
				m_edges.emplace_back(Edge(x2, y2, y1, -1, dda));
				bounds.m_min_y = std::min(bounds.m_min_y, y2);
				bounds.m_max_y = std::max(bounds.m_max_y, y1);
			}
		}
	}
	return bounds;
}

Canvas *Canvas::fpoly(const std::vector<std::vector<int32_t>> &polygons, int32_t x, int32_t y, int winding)
{
	static auto sample_offsets = std::array<int32_t, 8>{
		-16384, 24576, 0, -24576, 16384, -8192, -32768, 8192};
	static auto mask_to_coverage = std::array<uint8_t, 256>{
		0, 16, 16, 32, 16, 32, 32, 48, 16, 32, 32, 48, 32, 48, 48, 64, 16, 32, 32, 48,
		32, 48, 48, 64, 32, 48, 48, 64, 48, 64, 64, 80, 16, 32, 32, 48, 32, 48, 48, 64,
		32, 48, 48, 64, 48, 64, 64, 80, 32, 48, 48, 64, 48, 64, 64, 80, 48, 64, 64, 80,
		64, 80, 80, 96, 16, 32, 32, 48, 32, 48, 48, 64, 32, 48, 48, 64, 48, 64, 64, 80,
		32, 48, 48, 64, 48, 64, 64, 80, 48, 64, 64, 80, 64, 80, 80, 96, 32, 48, 48, 64,
		48, 64, 64, 80, 48, 64, 64, 80, 64, 80, 80, 96, 48, 64, 64, 80, 64, 80, 80, 96,
		64, 80, 80, 96, 80, 96, 96, 112, 16, 32, 32, 48, 32, 48, 48, 64, 32, 48, 48,
		64, 48, 64, 64, 80, 32, 48, 48, 64, 48, 64, 64, 80, 48, 64, 64, 80, 64, 80, 80,
		96, 32, 48, 48, 64, 48, 64, 64, 80, 48, 64, 64, 80, 64, 80, 80, 96, 48, 64, 64,
		80, 64, 80, 80, 96, 64, 80, 80, 96, 80, 96, 96, 112, 32, 48, 48, 64, 48, 64,
		64, 80, 48, 64, 64, 80, 64, 80, 80, 96, 48, 64, 64, 80, 64, 80, 80, 96, 64, 80,
		80, 96, 80, 96, 96, 112, 48, 64, 64, 80, 64, 80, 80, 96, 64, 80, 80, 96, 80,
		96, 96, 112, 64, 80, 80, 96, 80, 96, 96, 112, 80, 96, 96, 112, 96, 112, 112,
		128};
	auto scale = (m_flags & canvas_flag_antialias) ? 8 : 1;
	auto bounds = set_edges(polygons, x, y, scale);
	auto xs = bounds.m_min_x;
	auto ys = bounds.m_min_y;
	auto xe = bounds.m_max_x;
	auto ye = bounds.m_max_y;
	auto cy = m_cy * scale;
	auto cy1 = m_cy1 * scale;
	if (xs < m_cx1 && ys < cy1 && xe > m_cx && ye > cy)
	{
		//setup active edge list, edge starts and coverage
		if (m_edges_start.empty())
			m_edges_start.resize(m_pixmap->m_h * 8);
		if (m_flags & canvas_flag_antialias && m_coverage.empty())
			m_coverage.resize(m_pixmap->m_w);

		//edges into edge start lists
		Edge *tracker_list = nullptr;
		for (auto &edge : m_edges)
		{
			edge.m_next = m_edges_start[edge.m_ys];
			m_edges_start[edge.m_ys] = &edge;
		}

		//for each scan line
		auto cx = m_cx << FP_SHIFT;
		auto cx1 = (m_cx1 - 1) << FP_SHIFT;
		auto min_x = INT32_MAX;
		auto max_x = INT32_MIN;
		while (ys < ye)
		{
			//include new edges that begin on this scanline
			auto last = m_edges_start[ys];
			if (last)
			{
				auto first = last;
				while (last->m_next) last = last->m_next;
				last->m_next = tracker_list;
				tracker_list = first;
				m_edges_start[ys] = nullptr;
			}

			//sort active edges on x ?
			if (!(m_flags & canvas_flag_antialias) || winding == winding_none_zero)
			{
				Edge *sorted_list = nullptr;
				auto node = tracker_list;
				while (node)
				{
					auto next = node->m_next;
					auto last = (Edge*)&sorted_list;
					while (auto insert_node = last->m_next)
					{
						if (node->m_x <= insert_node->m_x) break;
						last = insert_node;
					}
					node->m_next = last->m_next;
					last->m_next = node;
					node = next;
				}
				tracker_list = sorted_list;
			}

			//antialiased ?
			if (m_flags & canvas_flag_antialias)
			{
				//draw edges into coverage mask
				auto node = (Edge*)&tracker_list;
				auto xo = sample_offsets[ys & 7];
				auto xm = 1 << (ys & 7);
				if (winding == winding_odd_even)
				{
					//odd even
					while (node->m_next)
					{
						node = node->m_next;
						x = node->m_x + xo;
						x = std::max(x, cx);
						x = std::min(x, cx1);
						x >>= FP_SHIFT;
						min_x = std::min(x, min_x);
						max_x = std::max(x, max_x);
						m_coverage[x] ^= xm;
					}
				}
				else
				{
					//non zero
					while (node->m_next)
					{
						node = node->m_next;
						x = node->m_x + xo;
						auto w = node->m_w;
						x = std::max(x, cx);
						x = std::min(x, cx1);
						x >>= FP_SHIFT;
						min_x = std::min(x, min_x);
						m_coverage[x] ^= xm;
						do
						{
							node = node->m_next;
							w += node->m_w;
						} while (w != 0);
						x = node->m_x + xo;
						x = std::max(x, cx);
						x = std::min(x, cx1);
						x >>= FP_SHIFT;
						max_x = std::max(x, max_x);
						m_coverage[x] ^= xm;
					}
				}

				//flush coverage mask to scan line
				if (((ys & 7) == 7) || ((ys + 1) == ye)) goto flush_mask;
				if ((ys + 1) < cy1) goto next_subline;
			flush_mask:
				if (min_x > max_x) goto next_subline;
				max_x++;
				auto x = min_x;
				auto y = ys >> 3;
				auto m = 0;
				do
				{
					auto x1 = x;
					auto m1 = m;
					do
					{
						m1 ^= m_coverage[x1++];
						if (x1 >= max_x) break;
					} while (m1 == m);
					m_coverage[x1 - 1] = 0;
					span_noclip(mask_to_coverage[m], x, y, x1);
					x = x1;
					m = m1;
				} while (x < max_x);
				min_x = INT32_MAX;
				max_x = INT32_MIN;
			}
			else
			{
				//draw spans for mode
				auto node = (Edge*)&tracker_list;
				if (winding == winding_odd_even)
				{
					//odd even
					while (node->m_next)
					{
						node = node->m_next;
						auto x1 = node->m_x;
						node = node->m_next;
						auto x2 = node->m_x;
						x1 >>= FP_SHIFT;
						x2 >>= FP_SHIFT;
						span(0x80, x1, ys, x2);
					}
				}
				else
				{
					//none zero
					while (node->m_next)
					{
						node = node->m_next;
						auto x1 = node->m_x;
						auto w = node->m_w;
						do
						{
							node = node->m_next;
							w += node->m_w;
						} while (w != 0);
						auto x2 = node->m_x;
						x1 >>= FP_SHIFT;
						x2 >>= FP_SHIFT;
						span(0x80, x1, ys, x2);
					}
				}
			}
		next_subline:
			//next sub scanline
			if (++ys >= cy1) break;

			//step the edges and remove any dead ones
			last = (Edge*)&tracker_list;
			while (auto node = last->m_next)
			{
				if (node->m_ye != ys)
				{
					node->m_x += node->m_dda;
					last = node;
					continue;
				}
				last->m_next = node->m_next;
				last = node;
			}
		}
	}
	return this;
}
