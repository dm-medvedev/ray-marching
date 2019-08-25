#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

using namespace glm;
using namespace std;

class Material {
  public:
	float reflection; // reflection coefficient
	float refraction; // refraction coefficient
	Material(float refl = 0, float refr = 0);
};

class Object {
  public:
	mat4 transform_mat;
	vec3 color;
	Material material;
	virtual float dist(vec3 p) = 0;
	Object();
	Object(mat4 transform_mat, vec3 color, Material material);
};

class Scene {
  public:
	std::vector<Object*> objects;
};

class Ray {
  public:
	vec3 pos; // origin in constant coordinates
	vec3 dir; // direction
	Ray(vec3 pos, vec3 dir);
};

class Light {
  public:
	vec3 color;
	vec3 p;
	Light(vec3 color, vec3 p);
};

class Hit {
  public:
	bool exist; // True if exist 
	float t; // distance to hit
	vec3 normal;
	int obj_num;
	Material material;
	Hit(bool exist, float t, Material material, vec3 normal = vec3(0, 0, 0), int obj_num = -1);
};

vec3 Shade(vec3 hit_point, vec3 normal, const Light &light);