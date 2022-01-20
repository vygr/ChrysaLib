#include "fixed.h"
#include <cmath>

//////////
//fixed 64
//////////

fixed64_t::fixed64_t(fixed32_t n)
	: m_val(n.m_val)
{}

fixed64_t &fixed64_t::operator>>=(const int &n)
{
	m_val >>= n;
	return *this;
}

fixed64_t fixed64_t::operator>>(const int &n) const
{
	fixed64_t result(*this);
	return result >>= n;
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

fixed64_t fixed64_t::operator-() const
{
	return fixed64_t(-m_val);
}

fixed64_t &fixed64_t::operator*=(const fixed64_t &n)
{
	m_val = (m_val * n.m_val) >> FP_SHIFT;
	return *this;
}

fixed64_t &fixed64_t::operator*=(const double &n)
{
	m_val = int64_t(m_val * n) >> FP_SHIFT;
	return *this;
}

fixed64_t &fixed64_t::operator*=(const int32_t &n)
{
	m_val *= n;
	return *this;
}

fixed64_t &fixed64_t::operator*=(const uint32_t &n)
{
	m_val *= n;
	return *this;
}

fixed64_t fixed64_t::operator*(const fixed64_t &n) const
{
	fixed64_t result(*this);
	return result *= n;
}

fixed64_t fixed64_t::operator*(const double &n) const
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

fixed32_t &fixed32_t::operator>>=(const int &n)
{
	m_val >>= n;
	return *this;
}

fixed32_t fixed32_t::operator>>(const int &n) const
{
	fixed32_t result(*this);
	return result >>= n;
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

fixed32_t fixed32_t::operator-() const
{
	fixed32_t result(*this);
	return result *= -1;
}

fixed32_t &fixed32_t::operator*=(const fixed32_t &n)
{
	m_val = ((int64_t)m_val * n.m_val) >> FP_SHIFT;
	return *this;
}

fixed32_t &fixed32_t::operator*=(const int32_t &n)
{
	m_val *= n;
	return *this;
}

fixed32_t &fixed32_t::operator*=(const uint32_t &n)
{
	m_val *= n;
	return *this;
}

fixed32_t &fixed32_t::operator*=(const double &n)
{
	m_val *= n;
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

fixed32_t fixed32_t::operator*(const double &n) const
{
	fixed32_t result(*this);
	return result *= n;
}

fixed32_t &fixed32_t::operator/=(const fixed32_t &n)
{
	m_val = ((int64_t)m_val << FP_SHIFT) / n.m_val;
	return *this;
}

fixed32_t &fixed32_t::operator/=(const int32_t &n)
{
	m_val /= n;
	return *this;
}

fixed32_t &fixed32_t::operator/=(const double &n)
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

fixed32_t fixed32_t::operator/(const double &n) const
{
	fixed32_t result(*this);
	return result /= n;
}

fixed32_t abs(const fixed32_t &n)
{
	if (n.m_val >= 0) return n;
	return fixed32_t(0) - n;
}

fixed32_t sqrt(const fixed32_t &n)
{
	return std::sqrt(double(n));
}

fixed32_t operator/(const double &n, const fixed32_t &f)
{
	return n / double(f);
}

fixed32_t operator*(const double &n, const fixed32_t &f)
{
	return double(f) * n;
}
