Point(float const& x, float const& y)
	: x(x), y(y) {
}

inline bool operator==(const Point& v) const noexcept {
	return x == v.x && y == v.y;
}

inline Point& operator+=(Point const& v) noexcept {
	x += v.x;
	y += v.y;
	return *this;
}

inline Point operator+(Point const& v) const noexcept {
	return Point{ x + v.x, y + v.y };
}

inline Point operator-(Point const& v) const noexcept {
	return Point{ x - v.x, y - v.y };
}

inline Point operator*(float const& s) const noexcept {
	return Point{ x * s, y * s };
}

inline Point operator/(float const& s) const noexcept {
	return Point{ x / s, y / s };
}
