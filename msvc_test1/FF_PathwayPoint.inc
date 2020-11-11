PathwayPoint(float const& x, float const& y, float const& a, float const& d) : pos(x, y), a(a), d(d) {}

template<typename T>
explicit PathwayPoint(T const& p) : pos(p.x, p.y) {}

template<typename T>
PathwayPoint& operator=(T const& p) {
	pos.x = p.x;
	pos.y = p.y;
	return *this;
}
