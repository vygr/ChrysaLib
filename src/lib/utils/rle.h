#ifndef RLE_H
#define RLE_H

#include <string>
#include <memory>

///////////////
// rle template
///////////////

template<class T>
void rle_encode(std::shared_ptr<std::string> &data, T *src, unsigned int src_len)
{
	auto src_end = &src[src_len / sizeof(T)];
	T token;
	T next_token;
	T *src_start;
	unsigned int state = 0;
	while (src != src_end)
	{
		switch (state)
		{
		case 1:
			//repeat state
			next_token = *src++;
			if (src == src_end || (src - src_start) == 0x7f || token != next_token)
			{
				if (token != next_token) { src--; }
				data->push_back((char)(src - src_start) | 0x80);
				data->append((const char*)&token, sizeof(T));
				state = 0;
			}
			continue;
		case 2:
			//block state
			next_token = *src++;
			if (src == src_end || (src - src_start) == 0x7f || token == next_token)
			{
				auto l = (uint8_t)(src - src_start);
				data->push_back(l);
				data->append((const char*)src_start, l * sizeof(T));
				state = 0;
			}
			else
			{
				token = next_token;
			}
			continue;
		default:
			//no state
			src_start = src;
			token = *src++;
			if (src == src_end) goto last_token;
			next_token = *src++;
			state = 2;
			if (token == next_token) { state = 1; }
			if (src != src_end) continue;
			if (state == 1)
			{
			last_token:
				data->push_back((char)(src - src_start) | 0x80);
				data->append((const char*)&token, sizeof(T));
			}
			else
			{
				auto l = (uint8_t)(src - src_start);
				data->push_back(l);
				data->append((const char*)src_start, l * sizeof(T));
			}
			return;
		}
	}
}

template<class T>
uint8_t *rle_decode(T *dst, uint8_t *src, unsigned int dst_len)
{
	auto dst_end = &dst[dst_len / sizeof(T)];
	T token;
	while (dst != dst_end)
	{
		auto l = *src++;
		if (l & 0x80)
		{
			for (auto i = 0; i < (int)sizeof(T); ++i)
			{
				*(((uint8_t*)&token) + i) = *src++;
			}
			auto run_end = &dst[l & 0x7f];
			do
			{
				*dst++ = token;
			} while (dst != run_end);
		}
		else
		{
			memcpy(dst, src, l * sizeof(T));
			src = &src[l * sizeof(T)];
			dst = &dst[l];
		}
	}
	return src;
}

#endif
