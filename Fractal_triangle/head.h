#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

using namespace glm;
using namespace std;

class Material
{
public:
	float reflection; // коэффициент отражения
	float refraction; // коэффициент преломления
	Material(float refl = 0, float refr = 0);
};

class Hit
{
public:
	bool exist; // True - существует
	float t; // Расстояние до столкновения
	vec3 normal; // нормаль
	int obj_num;
	Material material;
	/*struct material
	{
		float reflection; 
		float refraction; 
	}*/

	Hit(bool exist, float t, Material material, vec3 normal = vec3(0,0,0), int obj_num = -1);// пока что //, vec3 normal);
};

class Ray
{
public:
	vec3 pos; // начало в координатах константной штуки
	vec3 dir; // направление
	Ray(vec3 pos, vec3 dir);
};

class Object
{
public:
	// возможно следует добавить сюда поля x, y, z, angle и общий конструктор для 
	mat4 transform_mat;
	vec3 color;
	Material material;
	virtual float dist(vec3 p) = 0;
};

class Scene
{
public:
	std::vector<Object*> objects;
	//Scene();//ничего не передаётся так как все вручную
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

class Fractal: public Object
{
public:
	int Iterations;
	float Scale; // угол в градусах для glm::gtx::transform::rotate
	float x; //  можно еще заюзать glm::gtx::transform::translate,
	float y; // но он просто переносит походу без поворота
	float z;
	vec2 h;
	//mat4 transform_mat;
	//Material material;
	float dist(vec3 z);
	Fractal(float x, float y, float z, int Iterations, float Scale, 
			vec3 color, Material material);
};

class Light
{
public:
	vec3 color;
	vec3 p;
	Light(vec3 color, vec3 p);
};
