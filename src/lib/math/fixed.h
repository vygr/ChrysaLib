#ifndef LIB_FIXED_H
#define LIB_FIXED_H

#include <stdint.h>
#include <algorithm>
#include <cmath>

const uint32_t FP_SHIFT = 16;

class fixed64_t;

class fixed32_t
{
public:
	fixed32_t() : m_val(0) {}
	fixed32_t(fixed64_t n);
	fixed32_t(float n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed32_t(double n) : m_val(n * (1 << FP_SHIFT)) {}
	template <typename T> fixed32_t(T n) : m_val(n << FP_SHIFT) {}
	operator double() const { return double(m_val) / (1 << FP_SHIFT); }
	operator float() const { return float(m_val) / (1 << FP_SHIFT); }
	template <typename T> operator T() const { return m_val >> FP_SHIFT; }
	bool operator==(const fixed32_t &n) const { return m_val == n.m_val; }
	bool operator==(const double &n) const { return m_val == n * (1 << FP_SHIFT); }
	bool operator!=(const fixed32_t &n) const { return m_val != n.m_val; }
	bool operator<(const fixed32_t &n) const { return m_val < n.m_val; }
	bool operator<=(const fixed32_t &n) const { return m_val <= n.m_val; }
	bool operator<=(const double &n) const { return m_val <= n * (1 << FP_SHIFT); }
	fixed32_t operator-() const { fixed32_t result(*this); return result *= -1; }
	fixed32_t &operator++() { this->m_val++; return *this; }
	fixed32_t operator++(int) { fixed32_t result(*this); ++(*this); return result; }
	fixed32_t &operator--() { this->m_val--; return *this; }
	fixed32_t operator--(int) { fixed32_t result(*this); --(*this); return result; }
	fixed32_t &operator+=(const fixed32_t &n) { m_val += n.m_val; return *this; }
	fixed32_t operator+(const fixed32_t &n) const { fixed32_t result(*this); return result += n; }
	fixed32_t &operator-=(const fixed32_t &n) { m_val -= n.m_val; return *this; }
	fixed32_t operator-(const fixed32_t &n) const { fixed32_t result(*this); return result -= n; }
	template <typename T> fixed32_t &operator*=(const T &n) { m_val *= n; return *this; }
	fixed32_t &operator*=(const fixed32_t &n) { m_val = ((int64_t)m_val * n.m_val) >> FP_SHIFT; return *this; }
	template <typename T> fixed32_t operator*(const T &n) const { fixed32_t result(*this); return result *= n; }
	template <typename T> fixed32_t &operator/=(const T &n) { m_val /= n; return *this; }
	fixed32_t &operator/=(const fixed32_t &n) { m_val = ((int64_t)m_val << FP_SHIFT) / n.m_val; return *this; }
	template <typename T> fixed32_t operator/(const T &n) const { fixed32_t result(*this); return result /= n; }
	fixed32_t &operator>>=(const int &n) { m_val >>= n; return *this; }
	fixed32_t operator>>(const int &n) const { fixed32_t result(*this); return result >>= n; }
	int32_t m_val;
};

class fixed64_t
{
public:
	fixed64_t() : m_val(0) {}
	fixed64_t(fixed32_t n);
	fixed64_t(float n) : m_val(n * (1 << FP_SHIFT)) {}
	fixed64_t(double n) : m_val(n * (1 << FP_SHIFT)) {}
	template <typename T> fixed64_t(T n) : m_val(n << FP_SHIFT) {}
	template <typename T> operator T() const { return m_val >> FP_SHIFT; }
	operator double() const { return double(m_val) / (1 << FP_SHIFT); }
	operator float() const { return float(m_val) / (1 << FP_SHIFT); }
	bool operator==(const fixed64_t &n) const { return m_val == n.m_val; }
	bool operator!=(const fixed64_t &n) const { return m_val != n.m_val; }
	bool operator<(const fixed64_t &n) const { return m_val < n.m_val; }
	fixed64_t operator-() const { fixed64_t result(*this); return result *= -1; }
	fixed64_t &operator++() { this->m_val++; return *this; }
	fixed64_t operator++(int) { fixed64_t result(*this); ++(*this); return result; }
	fixed64_t &operator--() { this->m_val--; return *this; }
	fixed64_t operator--(int) { fixed64_t result(*this); --(*this); return result; }
	fixed64_t &operator+=(const fixed64_t &n) { m_val += n.m_val; return *this; }
	fixed64_t operator+(const fixed64_t &n) const { fixed64_t result(*this); return result += n; }
	fixed64_t &operator-=(const fixed64_t &n) { m_val -= n.m_val; return *this; }
	fixed64_t operator-(const fixed64_t &n) const { fixed64_t result(*this); return result -= n; }
	template <typename T> fixed64_t &operator*=(const T &n) { m_val *= n; return *this; }
	fixed64_t &operator*=(const fixed64_t &n) { m_val = ((int64_t)m_val * n.m_val) >> FP_SHIFT; return *this; }
	template <typename T> fixed64_t operator*(const T &n) const { fixed64_t result(*this); return result *= n; }
	template <typename T> fixed64_t &operator/=(const T &n) { m_val /= n; return *this; }
	fixed64_t &operator/=(const fixed64_t &n) { m_val = ((int64_t)m_val << FP_SHIFT) / n.m_val; return *this; }
	template <typename T> fixed64_t operator/(const T &n) const { fixed64_t result(*this); return result /= n; }
	fixed64_t &operator>>=(const int &n) { m_val >>= n; return *this; }
	fixed64_t operator>>(const int &n) const { fixed64_t result(*this); return result >>= n; }
	int64_t m_val;
};

#endif
