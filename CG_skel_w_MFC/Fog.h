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

	vec3 color = vec3(0.3, 0.3, 0.3);
	float m_start = 0;
	float m_end = 5;
	float m_inverse_length;
	float m_end_intensity = 0.8; // what is the alpha channel at max distance (0-1)
public:
	Fog() : color(vec3(0.3, 0.3, 0.3)), m_start(0), m_end(5),m_inverse_length(1/(m_end-m_start)), m_end_intensity(0.9) {}
	Fog(vec3 color, float start, float end) : color(color), m_start(start), m_end(end) {}

	void setColor(const vec3& new_color) { color = new_color; }
	void setStart(const float new_start) { m_start = new_start; m_inverse_length = 1/(m_end-m_start); }
	void setEnd(const float new_end) { m_end = new_end; m_inverse_length = 1/(m_end-m_start); }

	vec3 getFogColor() const { return color; }
	float getFogStart() const { return m_start; }
	float getFogEnd() const { return m_end; }

	float ComputeFog(const float distance) const { float dist = (m_end - distance) * m_inverse_length; return m_end_intensity*CLAMP(dist, 0, 1); }
};

