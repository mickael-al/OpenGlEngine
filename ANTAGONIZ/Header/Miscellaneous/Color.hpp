#ifndef __COLOR__
#define __COLOR__

#include <iostream>

class Color
{
public:
	static const Color& ClearBlack();
	static const Color& Black();
	static const Color& White();
	static const Color& Red();
	static const Color& Green();
	static const Color& Blue();
public:
	Color();
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	unsigned char GetR() const;
	unsigned char GetG() const;
	unsigned char GetB() const;
	unsigned char GetA() const;
	void SetR(unsigned char r);
	void SetG(unsigned char g);
	void SetB(unsigned char b);
	void SetA(unsigned char a);
public:
	Color& operator=(const Color& rhs);
	Color& operator+=(const Color& rhs);
	Color& operator-=(const Color& rhs);
	Color& operator*=(const Color& rhs);
	Color& operator/=(const Color& rhs);

	Color operator+(const Color& rhs) const;
	Color operator-(const Color& rhs) const;
	Color operator*(const Color& rhs) const;
	Color operator/(const Color& rhs) const;
	Color operator/(const float& rhs) const;

	bool operator==(const Color& rhs) const;
	bool operator!=(const Color& rhs) const;
	
	unsigned char& operator[](int index);

	friend std::ostream& operator<<(std::ostream& out, const Color& c);
private:
	unsigned char m_array[4];
};

#endif //!__COLOR__