#include "fixed.h"

//////////
//fixed 64
//////////

fixed64_t::fixed64_t(fixed32_t v)
	: m_val(v.m_val)
{}

fixed64_t &fixed64_t::operator>>=(const int &s)
{
	m_val >>= s;
	return *this;
}

fixed64_t fixed64_t::operator>>(const int &s) const
{
	fixed64_t result(*this);
	return result >>= s;
}

fixed64_t &fixed64_t::operator+=(const fixed64_t &n)
{
	m_val += n.m_val;
	return *this;
}

fixed64_t fixed64_t::operator+(const fixed64_t &n) const
{
	fixed64_t result(*this);
	return result += n;
}

fixed64_t &fixed64_t::operator++()
{
	this->m_val++;
	return *this;
}

fixed64_t fixed64_t::operator++ (int)
{
	fixed64_t result(*this);
	++(*this);
	return result;
}

fixed64_t &fixed64_t::operator-=(const fixed64_t &n)
{
	m_val -= n.m_val;
	return *this;
}

fixed64_t fixed64_t::operator-(const fixed64_t &n) const
{
	fixed64_t result(*this);
	return result -= n;
}

fixed64_t &fixed64_t::operator*=(const fixed64_t &n)
{
	m_val = (m_val * n.m_val) >> FP_SHIFT;
	return *this;
}

fixed64_t &fixed64_t::operator*=(const int32_t &n)
{
	m_val = (m_val * n);
	return *this;
}

fixed64_t &fixed64_t::operator*=(const uint32_t &n)
{
	m_val = (m_val * n);
	return *this;
}

fixed64_t fixed64_t::operator*(const fixed64_t &n) const
{
	fixed64_t result(*this);
	return result *= n;
}

fixed64_t fixed64_t::operator*(const int32_t &n) const
{
	fixed64_t result(*this);
	return result *= n;
}

fixed64_t fixed64_t::operator*(const uint32_t &n) const
{
	fixed64_t result(*this);
	return result *= n;
}

fixed64_t &fixed64_t::operator/=(const fixed64_t &n)
{
	m_val = (m_val << FP_SHIFT) / n.m_val;
	return *this;
}

fixed64_t &fixed64_t::operator/=(const int32_t &n)
{
	m_val /= n;
	return *this;
}

fixed64_t fixed64_t::operator/(const fixed64_t &n) const
{
	fixed64_t result(*this);
	return result /= n;
}

fixed64_t fixed64_t::operator/(const int32_t &n) const
{
	fixed64_t result(*this);
	return result /= n;
}

fixed64_t fixed64_t::abs(const fixed64_t &n)
{
	if (n.m_val >= 0) return n;
	return fixed64_t(0) - n;
}

//////////
//fixed 32
//////////

fixed32_t::fixed32_t(fixed64_t v)
	: m_val(v.m_val)
{}

fixed32_t &fixed32_t::operator>>=(const int &s)
{
	m_val >>= s;
	return *this;
}

fixed32_t fixed32_t::operator>>(const int &s) const
{
	fixed32_t result(*this);
	return result >>= s;
}

fixed32_t &fixed32_t::operator+=(const fixed32_t &n)
{
	m_val += n.m_val;
	return *this;
}

fixed32_t fixed32_t::operator+(const fixed32_t &n) const
{
	fixed32_t result(*this);
	return result += n;
}

fixed32_t &fixed32_t::operator++()
{
	this->m_val++;
	return *this;
}

fixed32_t fixed32_t::operator++ (int)
{
	fixed32_t result(*this);
	++(*this);
	return result;
}

fixed32_t &fixed32_t::operator-=(const fixed32_t &n)
{
	m_val -= n.m_val;
	return *this;
}

fixed32_t fixed32_t::operator-(const fixed32_t &n) const
{
	fixed32_t result(*this);
	return result -= n;
}

fixed32_t &fixed32_t::operator*=(const fixed32_t &n)
{
	m_val = (m_val * n.m_val) >> FP_SHIFT;
	return *this;
}

fixed32_t &fixed32_t::operator*=(const int32_t &n)
{
	m_val = (m_val * n);
	return *this;
}

fixed32_t &fixed32_t::operator*=(const uint32_t &n)
{
	m_val = (m_val * n);
	return *this;
}

fixed32_t fixed32_t::operator*(const fixed32_t &n) const
{
	fixed32_t result(*this);
	return result *= n;
}

fixed32_t fixed32_t::operator*(const int32_t &n) const
{
	fixed32_t result(*this);
	return result *= n;
}

fixed32_t fixed32_t::operator*(const uint32_t &n) const
{
	fixed32_t result(*this);
	return result *= n;
}

fixed32_t &fixed32_t::operator/=(const fixed32_t &n)
{
	m_val = (m_val << FP_SHIFT) / n.m_val;
	return *this;
}

fixed32_t &fixed32_t::operator/=(const int32_t &n)
{
	m_val /= n;
	return *this;
}

fixed32_t fixed32_t::operator/(const fixed32_t &n) const
{
	fixed32_t result(*this);
	return result /= n;
}

fixed32_t fixed32_t::operator/(const int32_t &n) const
{
	fixed32_t result(*this);
	return result /= n;
}

fixed32_t fixed32_t::abs(const fixed32_t &n)
{
	if (n.m_val >= 0) return n;
	return fixed32_t(0) - n;
}
