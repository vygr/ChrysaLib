#ifndef LIB_FIXED_H
#define LIB_FIXED_H

#include <stdint.h>
#include <algorithm>

const uint32_t FP_SHIFT = 16;

class fixed64_t;

class fixed32_t
{
public:
	fixed32_t() : m_val(0) {}
	fixed32_t(int32_t n) : m_val(n << FP_SHIFT) {}
	fixed32_t(int64_t n) : m_val(n << FP_SHIFT) {}
	fixed32_t(uint32_t n) : m_val(n << FP_SHIFT) {}
	fixed32_t(uint64_t n) : m_val(n << FP_SHIFT) {}
	fixed32_t(float n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed32_t(double n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed32_t(fixed64_t n);
	operator int32_t() const { return m_val >> FP_SHIFT; }
	operator uint32_t() const { return m_val >> FP_SHIFT; }
	operator int64_t() const { return m_val >> FP_SHIFT; }
	operator uint64_t() const { return m_val >> FP_SHIFT; }
	bool operator==(const fixed32_t &n) const { return m_val == n.m_val; }
	bool operator!=(const fixed32_t &n) const { return m_val != n.m_val; }
	bool operator<(const fixed32_t &n) const { return m_val < n.m_val; }
	bool operator<=(const fixed32_t &n) const { return m_val <= n.m_val; }
	fixed32_t &operator++();
	fixed32_t operator++ (int);
	fixed32_t &operator--();
	fixed32_t operator-- (int);
	fixed32_t &operator+=(const fixed32_t &n);
	fixed32_t operator+(const fixed32_t &n) const;
	fixed32_t &operator-=(const fixed32_t &n);
	fixed32_t operator-(const fixed32_t &n) const;
	fixed32_t &operator*=(const fixed32_t &n);
	fixed32_t &operator*=(const int32_t &n);
	fixed32_t &operator*=(const uint32_t &n);
	fixed32_t operator*(const fixed32_t &n) const;
	fixed32_t operator*(const int32_t &n) const;
	fixed32_t operator*(const uint32_t &n) const;
	fixed32_t &operator/=(const fixed32_t &n);
	fixed32_t &operator/=(const int32_t &n);
	fixed32_t operator/(const fixed32_t &n) const;
	fixed32_t operator/(const int32_t &n) const;
	fixed32_t &operator>>=(const int &n);
	fixed32_t operator>>(const int &n) const;
	static fixed32_t abs(const fixed32_t &n);
	int32_t m_val;
};

class fixed64_t
{
public:
	fixed64_t() : m_val(0) {}
	fixed64_t(int32_t n) : m_val(n << FP_SHIFT) {}
	fixed64_t(int64_t n) : m_val(n << FP_SHIFT) {}
	fixed64_t(uint32_t n) : m_val(n << FP_SHIFT) {}
	fixed64_t(uint64_t n) : m_val(n << FP_SHIFT) {}
	fixed64_t(float n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed64_t(double n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed64_t(fixed32_t n);
	operator int32_t() const { return m_val >> FP_SHIFT; }
	operator uint32_t() const { return m_val >> FP_SHIFT; }
	operator int64_t() const { return m_val >> FP_SHIFT; }
	operator uint64_t() const { return m_val >> FP_SHIFT; }
	bool operator==(const fixed64_t &n) const { return m_val == n.m_val; }
	bool operator!=(const fixed64_t &n) const { return m_val != n.m_val; }
	bool operator<(const fixed64_t &n) const { return m_val < n.m_val; }
	fixed64_t &operator++();
	fixed64_t operator++ (int);
	fixed64_t &operator--();
	fixed64_t operator-- (int);
	fixed64_t &operator+=(const fixed64_t &n);
	fixed64_t operator+(const fixed64_t &n) const;
	fixed64_t &operator-=(const fixed64_t &n);
	fixed64_t operator-(const fixed64_t &n) const;
	fixed64_t &operator*=(const fixed64_t &n);
	fixed64_t &operator*=(const double &n);
	fixed64_t &operator*=(const int32_t &n);
	fixed64_t &operator*=(const uint32_t &n);
	fixed64_t operator*(const fixed64_t &n) const;
	fixed64_t operator*(const double &n) const;
	fixed64_t operator*(const int32_t &n) const;
	fixed64_t operator*(const uint32_t &n) const;
	fixed64_t &operator/=(const fixed64_t &n);
	fixed64_t &operator/=(const int32_t &n);
	fixed64_t operator/(const fixed64_t &n) const;
	fixed64_t operator/(const int32_t &n) const;
	fixed64_t &operator>>=(const int &n);
	fixed64_t operator>>(const int &n) const;
	static fixed64_t abs(const fixed64_t &n);
	int64_t m_val;
};

#endif
