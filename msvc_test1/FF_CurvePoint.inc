CurvePoint(float const& x, float const& y, float const& tension, int const& numSegments)
	: x(x), y(y), tension(tension), numSegments(numSegments) {
}

inline bool operator==(const CurvePoint& v) const noexcept {
	return x == v.x && y == v.y;
}

inline CurvePoint& operator+=(CurvePoint const& v) noexcept {
	x += v.x;
	y += v.y;
	return *this;
}

inline CurvePoint operator+(CurvePoint const& v) const noexcept {
	return CurvePoint( x + v.x, y + v.y, 0.f, 0 );
}

inline CurvePoint operator-(CurvePoint const& v) const noexcept {
	return CurvePoint( x - v.x, y - v.y, 0.f, 0 );
}

inline CurvePoint operator*(float const& s) const noexcept {
	return CurvePoint( x * s, y * s, 0.f, 0 );
}

inline CurvePoint operator/(float const& s) const noexcept {
	return CurvePoint( x / s, y / s, 0.f, 0 );
}
