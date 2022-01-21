#include "fixed.h"

//////////
//fixed 64
//////////

fixed64_t::fixed64_t(fixed32_t n)
	: m_val(n.m_val)
{}

fixed64_t abs(const fixed64_t &n)
{
	if (n.m_val >= 0) return n;
	return fixed64_t(0) - n;
}

fixed64_t sqrt(const fixed64_t &n)
{
	return fixed64_t(std::sqrt(((double)n.m_val) / (1 << FP_SHIFT)));
}

//////////
//fixed 32
//////////

fixed32_t::fixed32_t(fixed64_t n)
	: m_val(n.m_val)
{}

fixed32_t abs(const fixed32_t &n)
{
	if (n.m_val >= 0) return n;
	return fixed32_t(0) - n;
}

fixed32_t sqrt(const fixed32_t &n)
{
	return std::sqrt(double(n));
}

fixed32_t sin(const fixed32_t &n)
{
	return std::sin(double(n));
}

fixed32_t cos(const fixed32_t &n)
{
	return std::cos(double(n));
}

fixed32_t acos(const fixed32_t &n)
{
	return std::acos(double(n));
}

fixed32_t operator/(const double &n, const fixed32_t &f)
{
	return n / double(f);
}

fixed32_t operator*(const double &n, const fixed32_t &f)
{
	return double(f) * n;
}
