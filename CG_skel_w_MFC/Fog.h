#pragma once

#include "vec.h"
#define CLAMP(x,min,max) ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

/*float clamp(const float value, const float min_num, const float max_num)
{
	return max(min_num, min(value, max_num));
}
*/

class Fog
{
private:
	vec4 color = vec4(0.3, 0.3, 0.3, 1);
	float m_start = 0;
	float m_end = 5;
public:
	Fog() : color(vec4(0.3, 0.3, 0.3, 1)), m_start(0), m_end(5) {}
	Fog(vec4 color, float start, float end) : color(color), m_start(start), m_end(end) {}

	void setColor(const vec4& new_color) { color = new_color; }
	void setStart(const float new_start) { m_start = new_start; }
	void setEnd(const float new_end) { m_end = new_end; }

	vec4 getFogColor() const { return color; }
	float getFogStart() const { return m_start; }
	float getFogEnd() const { return m_end; }

	float ComputeFog(const float distance) const { float dist = (m_end - distance) / (m_end - m_start); return CLAMP(dist, 0, 1); }
};

