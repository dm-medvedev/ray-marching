#include "../common/glm/glm.hpp"
#include "../common/glm/gtc/matrix_transform.hpp"
#include "../common/glm/gtc/type_ptr.hpp"
#include <vector>

using namespace glm;
using namespace std;

class HexagonalPrism: public Object
{
public:
	float angle; // угол в градусах для glm::gtx::transform::rotate
	float x; //  можно еще заюзать glm::gtx::transform::translate,
	float y; // но он просто переносит походу без поворота
	float z;
	vec2 h;
	//mat4 transform_mat;
	//Material material;
	float dist(vec3 p);
	HexagonalPrism(float angle, float x, float y, float z, vec2 h, vec3 color, Material material);
};

class Box: public Object
{
public:
	float angle; // угол в градусах для glm::gtx::transform::rotate
	float x; //  можно еще заюзать glm::gtx::transform::translate,
	float y; // но он просто переносит походу без поворота
	float z;
	vec3 b; // вектор длин половин сторон!!!???!!
	//mat4 transform_mat;
	//Material material;
	float dist(vec3 p);
	Box(float angle, float x, float y, float z, vec3 b, vec3 color, Material material);
};

class Sphere: public Object
{
public:
	//float angle; // угол в градусах для glm::gtx::transform::rotate
	float x; //  можно еще заюзать glm::gtx::transform::translate,
	float y; // но он просто переносит походу без поворота
	float z; // координаты относительно константной
	float s; //радиус
	//mat4 transform_mat;
	//Material material;
	float dist(vec3 p);
	Sphere(float x, float y, float z, float s, vec3 color, Material material); //float angle, 
};
