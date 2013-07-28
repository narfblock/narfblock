#ifndef NARF_COLOR_H
#define NARF_COLOR_H

namespace narf {

	class Color {
	public:
		float r, g, b, a;
		Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
		Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
	};

} // namespace narf

#endif // NARF_COLOR_H
