#include "Color.hpp"

const Color& Color::ClearBlack()
{
	static Color value = Color(0, 0, 0, 0);
	return value;
}

const Color& Color::Black()
{
	static Color value = Color(0, 0, 0, 255);
	return value;
}
const Color& Color::White()
{
	static Color value = Color(255, 255, 255, 255);
	return value;
}

const Color& Color::Red()
{
	static Color value = Color(255, 0, 0, 255);
	return value;
}

const Color& Color::Green()
{
	static Color value = Color(0, 255, 0, 255);
	return value;
}

const Color& Color::Blue()
{
	static Color value = Color(0, 0, 255, 255);
	return value;
}

Color::Color() 
{
	m_array[0] = 0;
	m_array[1] = 0;
	m_array[2] = 0;
	m_array[3] = 0;
}

Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	m_array[0] = r;
	m_array[1] = g;
	m_array[2] = b;
	m_array[3] = a;
}

unsigned char Color::GetR() const
{
	return m_array[0];
}

unsigned char Color::GetG() const
{
	return m_array[1];
}

unsigned char Color::GetB() const
{
	return m_array[2];
}

unsigned char Color::GetA() const
{
	return m_array[3];
}

void Color::SetR(unsigned char r)
{
	m_array[0] = r;
}

void Color::SetG(unsigned char g)
{
	m_array[1] = g;
}

void Color::SetB(unsigned char b)
{
	m_array[2] = b;
}

void Color::SetA(unsigned char a)
{
	m_array[3] = a;
}

Color& Color::operator=(const Color& rhs)
{
	m_array[0] = rhs.m_array[0];
	m_array[1] = rhs.m_array[1];
	m_array[2] = rhs.m_array[2];
	m_array[3] = rhs.m_array[3];

	return *this;
}

Color& Color::operator+=(const Color& rhs)
{
	m_array[0] += rhs.m_array[0];
	m_array[1] += rhs.m_array[1];
	m_array[2] += rhs.m_array[2];
	m_array[3] += rhs.m_array[3];

	return *this;
}

Color& Color::operator-=(const Color& rhs)
{
	m_array[0] -= rhs.m_array[0];
	m_array[1] -= rhs.m_array[1];
	m_array[2] -= rhs.m_array[2];
	m_array[3] -= rhs.m_array[3];
	return *this;
}

Color& Color::operator*=(const Color& rhs)
{
	m_array[0] *= rhs.m_array[0];
	m_array[1] *= rhs.m_array[1];
	m_array[2] *= rhs.m_array[2];
	m_array[3] *= rhs.m_array[3];
	return *this;
}

Color& Color::operator/=(const Color& rhs)
{
	m_array[0] /= rhs.m_array[0];
	m_array[1] /= rhs.m_array[1];
	m_array[2] /= rhs.m_array[2];
	m_array[3] /= rhs.m_array[3];
	return *this;
}

Color Color::operator+(const Color& rhs) const
{
	return Color(m_array[0] + rhs.m_array[0],
	m_array[1] + rhs.m_array[1],
	m_array[2] + rhs.m_array[2],
	m_array[3] + rhs.m_array[3]);
}

Color Color::operator-(const Color& rhs) const
{
	return Color(m_array[0] - rhs.m_array[0],
	m_array[1] - rhs.m_array[1],
	m_array[2] - rhs.m_array[2],
	m_array[3] - rhs.m_array[3]);
}

Color Color::operator*(const Color& rhs) const
{
	return Color(m_array[0] * rhs.m_array[0],
	m_array[1] * rhs.m_array[1],
	m_array[2] * rhs.m_array[2],
	m_array[3] * rhs.m_array[3]);
}

Color Color::operator/(const Color& rhs) const
{
	return Color(m_array[0] / rhs.m_array[0],
	m_array[1] / rhs.m_array[1],
	m_array[2] / rhs.m_array[2],
	m_array[3] / rhs.m_array[3]);
}

Color Color::operator/(const float& rhs) const
{
	return Color(m_array[0] / rhs,
		m_array[1] / rhs,
		m_array[2] / rhs,
		m_array[3] / rhs);
}

bool Color::operator==(const Color& rhs) const
{
	return
		m_array[0] == rhs.m_array[0] &&
		m_array[1] == rhs.m_array[1] &&
		m_array[2] == rhs.m_array[2] &&
		m_array[3] == rhs.m_array[3];
}

bool Color::operator!=(const Color& rhs) const
{
	return
		m_array[0] != rhs.m_array[0] ||
		m_array[1] != rhs.m_array[1] ||
		m_array[2] != rhs.m_array[2] ||
		m_array[3] != rhs.m_array[3];
}

unsigned char& Color::operator[](int index)
{
	if (index < 0 || index >= 4)
	{
		throw std::out_of_range("Index out of range");
	}

	return m_array[index];
}

std::ostream& operator<<(std::ostream& out, const Color& c)
{
	out << "(" << (int)c.GetR() << ", " << (int)c.GetG() << ", " << (int)c.GetB() << ", " << (int)c.GetA() << ")";
	return out;
}
