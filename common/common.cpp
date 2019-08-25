#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "common.h"
#include <vector>

using namespace glm;
using namespace std;

Material::Material(float reflection, float refraction)
	: reflection(reflection), refraction(refraction) {}

Object::Object() {}

Object::Object(mat4 transform_mat, vec3 color, Material material)
	: transform_mat(transform_mat), color(color), material(material) {}

Ray::Ray(vec3 pos, vec3 dir)
	: pos(pos), dir(normalize(dir)) {}

Light::Light(vec3 color, vec3 pos)
	: color(color), p(pos) {}

Hit::Hit(bool exist, float t, Material material, vec3 normal, int obj_num)
	: exist(exist), t(t), material(material), normal(normal), obj_num(obj_num) {}

vec3 Shade(vec3 hit_point, vec3 normal, const Light & light) {
	vec3 main_vec = light.p - hit_point;
	float product = glm::dot(normalize(main_vec), normal);
	if (product < 0) return vec3(0, 0, 0);
	float rho = length(main_vec) * length(main_vec) + 1;
	return light.color * (product / rho);
}