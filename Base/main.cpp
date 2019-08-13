#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "head.h"
#include "io.h" // работа с image и matrix

#define MAX_COUNT_STEPS 80
#define MIN_STEP 0.001/100
#define STOP_STEP 0.05/100
#define IMG_ROWS 512*4+1// нечетная
#define IMG_COLS 512*4+1// нечетная
#define CAMERA_IMG_DIST 200
#define PIX_IMG_SIZE 0.5/3 //расстояние меду двумя соседними ray из камеры на IMG
#define SHADE_CONST 1//16*1000
#define REFL_COUNT 2
#define REFR_COUNT 10
using namespace glm;

Material::Material(float reflection, float refraction)
{
	this->reflection = reflection;
	this->refraction = refraction;
}

Ray::Ray(vec3 pos, vec3 dir)
{
	this->pos = pos;
	this->dir = normalize(dir);
}

Light::Light(vec3 color, vec3 pos)
{
	this->color = color;
	this-> p = pos;
}

float Sphere::dist(glm::vec3 p)
{
	vec4 new_p(p.x, p.y, p.z, 1);
	new_p = this->transform_mat * new_p;// переводит в систему координат относительно сферы
	vec3 last_p(new_p); // первые 3 берёт
	return length(last_p)-this->s;
}

Sphere::Sphere(float x, float y, float z, float s, vec3 color, Material material)
{
	this->s = s;
	this->x = x; // возможно это не нужно
	this->y = y; // возможно это не нужно
	this->z = z; // возможно это не нужно
	this->color = color;
	this->material = material;
	//std::cout<<"AAAKAKMAKKAAK"<<this->color.x<<"\n";
	this->transform_mat = inverse(glm::translate( glm::mat4(1.0f), vec3(x, y, z)));
}

Box::Box(float angle, float x, float y, float z, vec3 b, vec3 color, Material material)
{
	this->b = b;
	this->x = x; // возможно это не нужно
	this->y = y; // возможно это не нужно
	this->z = z; // возможно это не нужно
	this->z = z; // возможно это не нужно
	this->angle = angle;
	this->color = color;
	this->material = material;
	this->transform_mat = glm::translate( glm::mat4(1.0f), vec3(x, y, z));
	this->transform_mat = inverse(glm::rotate(this->transform_mat, angle, glm::vec3(1.0f, 0.0f, 0.0f)));
}

float Box::dist(glm::vec3 p)
{
	vec4 new_p(p.x, p.y, p.z, 1);
	new_p = this->transform_mat * new_p;// переводит в систему координат относительно сферы
	vec3 last_p(new_p); // первые 3 берёт
  	return length(glm::max(glm::abs(last_p)-this->b, 0.0f));
}

HexagonalPrism::HexagonalPrism(float angle, float x, float y, float z, vec2 h, vec3 color, Material material)
{
	this->h = h;
	this->x = x; // возможно это не нужно
	this->y = y; // возможно это не нужно
	this->z = z; // возможно это не нужно
	this->z = z; // возможно это не нужно
	this->angle = angle;
	this->color = color;
	this->material = material;
	this->transform_mat = glm::translate( glm::mat4(1.0f), vec3(x, y, z));
	this->transform_mat = inverse(glm::rotate(this->transform_mat, angle, glm::vec3(1.0f, 0.0f, 0.0f)));
}

float HexagonalPrism::dist(glm::vec3 p)
{
	vec4 new_p(p.x, p.y, p.z, 1);
	new_p = this->transform_mat * new_p;// переводит в систему координат относительно сферы
	vec3 last_p(new_p); // первые 3 берёт
	vec3 q = abs(last_p);
    return glm::max(q.z-(this->h).y, glm::max((q.x*0.866025f+q.y*0.5f),q.y)-(this->h).x);
}

Hit::Hit(bool exist, float t, Material material, vec3 normal, int obj_num)// пока что //, vec3 normal)
{
	this->exist = exist;
	this->t = t;
	this->obj_num = obj_num;
	this->normal = normal;
	this->material = material;
}

/*{	
	std::vector<Object*> objects;
	Sphere it = Sphere(0, 0, 0, 10);
    Object *item = &it;
	this->objects.push_back(item);
}*/ //Возникли некоторые сложности, поэтому я засуну это в main

vec3 EstimateNormal(Object* obj, vec3 z)
{
	float eps = 1;
	vec3 z1 = z + vec3(eps, 0, 0);
	vec3 z2 = z - vec3(eps, 0, 0);
	vec3 z3 = z + vec3(0, eps, 0);
	vec3 z4 = z - vec3(0, eps, 0);
	vec3 z5 = z + vec3(0, 0, eps);
	vec3 z6 = z - vec3(0, 0, eps);

	float dx = obj->dist(z1) - obj->dist(z2);
	float dy = obj->dist(z3) - obj->dist(z4);
	float dz = obj->dist(z5) - obj->dist(z6);
	float coef = (1/(2.0*eps));

	return normalize( vec3(dx, dy, dz)*coef );
}

Hit RaySceneIntersection(const Ray& ray, const Scene &scene)
{
	int steps_count = 0;
	vec3 p = ray.pos;
	float min = 0, t = 0;
	int obj_num;
	while(steps_count < MAX_COUNT_STEPS)
	{
		min = scene.objects[0]->dist(p);
		obj_num = 0;
		for(int i=1; i < scene.objects.size(); ++i)
		{	
			float curr = 0;
			curr = scene.objects[i]->dist(p); 
			if(curr < min) {min = curr; obj_num = i;}
		}
		
		if(MIN_STEP > min) min = MIN_STEP;
		
		p +=  min*ray.dir;
		t+=min;
		//std::cout<<"min: "<<min<<"\n";
		steps_count++;
		if(min < STOP_STEP)
		{ 
			vec3 norm = EstimateNormal(scene.objects[obj_num], p); 
			return Hit(true, t, scene.objects[obj_num]->material, norm, obj_num);
		}
	} 
	//std::cout<<"FINAL t: "<<t<<"\n";
	return Hit(false, 0, scene.objects[0]->material);
	//scene.objects[0];
}

bool Visible(const vec3 &hit_point, const Light &light, const Scene &scene, int obj_num)
{
	int steps_count = 0;
	vec3 p = hit_point;
	vec3 dir = normalize(light.p - p);
	float dist = length(light.p - p);
	float min = 0, t = 0;
	int start = (0 != obj_num) ? 0: 1;
	while(steps_count < MAX_COUNT_STEPS)
	{
		min = scene.objects[start]->dist(p);
		for(int i=start+1; i < scene.objects.size(); ++i)
		{	
			float curr = 0;
			if(i != obj_num)
			{
				curr = scene.objects[i]->dist(p); 
				if(curr < min) min = curr;
			}
		}
		
		if(MIN_STEP > min) min = MIN_STEP;
		
		p +=  min*dir;
		t+=min;
		//std::cout<<"min: "<<min<<"\n";
		steps_count++;
		if(min < STOP_STEP and t <= dist) return false;
		if(min < STOP_STEP and t > dist) return true;
	} 
	//std::cout<<"FINAL t: "<<t<<"\n";
	return true;	
}

vec3 Shade(vec3 hit_point, vec3 normal, const Light &light)
{
	vec3 main_vec = light.p - hit_point;
	float product = glm::dot(normalize(main_vec),normal);
	if(product < 0) return vec3(0,0,0);
	float rho = length(main_vec)*length(main_vec)+1;
	//std::cout<<"HI: color is "<<(product*SHADE_CONST/rho)<<std::endl;
	return light.color*(product/rho);
}

Ray reflect(const Ray& ray, const Hit& hit, vec3 hit_point, const Scene &scene)
{	
	vec3 p = hit_point;
	float min = scene.objects[0]->dist(p);
	int obj_num = 0;
	for(int i=1; i < scene.objects.size(); ++i)
	{	
		float curr = 0;
		curr = scene.objects[i]->dist(p); 
		if(curr < min) {min = curr; obj_num = i;}
	}
	
	if(MIN_STEP > min) min = MIN_STEP;

	//vec3 new_dir = normalize(ray.dir-hit.normal*(2*glm::dot(hit.normal, ray.dir)));
	vec3 new_dir = normalize(glm::reflect(ray.dir, hit.normal));
	return Ray(hit_point + new_dir, new_dir); //немного отступ
}

float AmbientOcclusion(vec3 point ,vec3 normal ,vec3 color, const Scene &scene, float delta, float k)
{
	float ao=0, coef1 = 1.0;
	for(int i =0; i<5; i++)
	{
		vec3 p = point + normal*(i*delta);
		coef1 = coef1/2.0;

		float rho = scene.objects[0]->dist(p);
		for(int j=1; j < scene.objects.size(); ++j)
		{	
			float curr = scene.objects[j]->dist(p); 
			if(curr < rho) rho = curr;
		}

		if(rho < 0) break;
		ao+= coef1*((i+1)*delta - rho);
	}

	return 1-ao*k;
}

vec3 RayTrace(const Ray& ray, const Scene &scene, std::vector<Light> lights, int count = 0)
{
	vec3 color(0.0f,0.0f,0.0f);
	count+=1;// для ограничения рекурсии
	Hit hit = RaySceneIntersection(ray, scene);
	if (!hit.exist)
		return color;
	/*else
	{	 
		std::cout<<"t:"<<hit.t<<std::endl;
		float rho = hit.t*hit.t+1;
		color.x = 1/rho;
		color.y = 1/rho;
		color.z = 1/rho;
		return color;
	}*/
	
	vec3 hit_point = ray.pos + ray.dir*hit.t;
	
	// затенение, используется локальная модель освещения для источников света
	//
	for(int i=0; i < lights.size();i++)
		if(Visible(hit_point, lights[i], scene, hit.obj_num)){
			vec3 shd = Shade(hit_point, hit.normal, lights[i]);
			color.x += (scene.objects[hit.obj_num]->color).x*shd.x;
			color.y += (scene.objects[hit.obj_num]->color).y*shd.y;
			color.z += (scene.objects[hit.obj_num]->color).z*shd.z;
			//std::cout<<"WHY?: "<<color.x<<" | "<<shd.x<<" | "<<hit.obj_num<<"\n";
		}

	if (hit.material.reflection > 0 and count < REFL_COUNT)
	{
		//std::cout<<"HI!!!!!!!!!!!!!!!!!!"<<'\n';
		Ray reflRay = reflect(ray, hit, hit_point, scene);
		vec3 refl_color = RayTrace(reflRay, scene, lights, count);
		if(refl_color == vec3(0,0,0))
			count = REFL_COUNT;
		color+=hit.material.reflection*refl_color;
	}
	
	//Ambient Occlusion
	color = AmbientOcclusion(hit_point, hit.normal, color, scene, 0.1, 1)*color;
	/*if (hit.material.refraction > 0)
	{
		Ray refrRay = refract(ray, hit);
		color += hit.material.refraction*RayTrace(refrRay);
	}*/
	
	return color;
}

/*
https://www.cs.utexas.edu/~theshark/courses/cs354/lectures/cs354-10.pdf
https://alexander-bobkov.ru/opengl/tutorials/tutorial3
*/
int main()
{
	std::vector<Light> lights;
	lights.push_back(Light(vec3(0,10,0), vec3(0,0,0)));
	lights.push_back(Light(vec3(40,40,40), vec3(310,310, 20))); // цвет 10 10 10 хорош

	Scene scene;
	Sphere it0 = Sphere(150, 220, 4, 5, 
						vec3(1 , 1, 1),
						Material(0.5, 0));
	
	Sphere it1 = Sphere(0, 150, 0, 50, 
						vec3(0.01 ,0.2, 1),
						Material(0, 0));
	
	Box it2 = Box(10, 150, -10, 0, vec3 (50, 50, 50), 
				  vec3(1, 0.7, 0),
				  Material(0, 0));
	
	HexagonalPrism it3 = HexagonalPrism(0, 0,  0, 150, 
						vec2(50.0f, 50.0f), vec3(1,0.25,1),
						Material(0, 0));
	
	Box it4 = Box(45, -50,  -50, -50, 
			  vec3(10.0f, 10.0f, 10.0f), vec3(1,1,1),
			  Material(0, 0));
	
	Sphere it5 = Sphere(0, 0, -800, 700, 
					vec3(1 ,0.1 ,0.01),
					Material(1, 0));

    Object *item1 = &it1;
    Object *item0 = &it0;
    Object *item2 = &it2;
    Object *item3 = &it3;
    Object *item4 = &it4;
    Object *item5 = &it5;

    scene.objects.push_back(item0);
	scene.objects.push_back(item1);
	scene.objects.push_back(item2);
	scene.objects.push_back(item3);
	scene.objects.push_back(item4);
	scene.objects.push_back(item5);
	//std::cout<<"AAAAAAAA!!!!!!"<<scene.objects[4]->color.x<<"\n";
	
	Image img(512*2, 512*2); //IMG_ROWS, IMG_COLS
	Matrix <std::tuple<float, float, float>> prev_img(IMG_ROWS, IMG_COLS);
	
	vec3 eye(280+50, 310+50, 20); //координаты в мировой системе где камера стоит
	vec3 center(0,0,0);
	vec3 up(0,1,0);
	mat4 M_view = lookAt(eye, center, up); // из мировой к камере
	mat4 inverse_M_view = inverse(M_view);
	int ind_center_row_img = (IMG_ROWS-1)/2;
	int ind_center_col_img = (IMG_COLS-1)/2;

	vec3 min(1000000, 1000000, 1000000), max(-1,-1,-1);
	
	#pragma omp parallel for collapse(2) schedule(dynamic, 512)
	for(int i=0; i<IMG_ROWS; ++i){
		for(int j=0; j<IMG_COLS; ++j){
			/*std::cout<<"********************\n";
			std::cout<<"i: "<<i<<" j: "<<j<<endl;
			std::cout<<"********************\n";*/

			/*if( (i*IMG_COLS+j)%10000 == 0) std::cout<<"completeness: "<< 
				(i*IMG_COLS+j)/(1.0*IMG_ROWS*IMG_COLS)<<"\n";*/

			vec3 v((i-ind_center_row_img)*PIX_IMG_SIZE,
					  (j-ind_center_col_img)*PIX_IMG_SIZE,
					   -1*CAMERA_IMG_DIST);
			
			vec4 camera_pix_point_coords( v.x, v.y, v.z, 1);

			vec3 n_v = normalize(v);
			vec4 camera_pix_dir_coords( n_v.x, n_v.y, n_v.z, 0);

			vec3 pos(inverse_M_view*camera_pix_point_coords);
			vec3 dir(inverse_M_view*camera_pix_dir_coords);
			//std::cout<<"HIII"<<" i: "<<i<<" j: "<<j<<"\n";
			Ray ray(pos, dir);
			vec3 color = RayTrace(ray, scene, lights);
			//if(color.x != 0) std::cout<<"HERE!!!";
			/*color = glm::max( glm::min(color*255.0f, 255.0f), 0.0f);
			img(i,j) = std::make_tuple(color.x/1, color.y/1, color.z/1);*/
			prev_img(i, j) = std::make_tuple(color.x, color.y, color.z);
			if(min.x > color.x) min.x = color.x;
			if(min.y > color.y) min.y = color.y;
			if(min.z > color.z) min.z = color.z;
			if(max.x < color.x) max.x = color.x;
			if(max.y < color.y) max.y = color.y;
			if(max.z < color.z) max.z = color.z;
		}
	}
	std::cout<<"min: "<<min.x<<" | "<<min.y<<" | "<<min.z<<std::endl;
	std::cout<<"max: "<<max.x<<" | "<<max.y<<" | "<<max.z<<std::endl;
	float min_p = min.x, max_p = max.x;
	if(min.y < min_p) min_p = min.y;
	if(min.z < min_p) min_p = min.z;

	if(max.y > max_p) max_p = max.y;
	if(max.z > max_p) max_p = max.z;

	/*for(int i=0; i<IMG_ROWS; ++i){
		for(int j=0; j<IMG_COLS; ++j){
			float x, y, z;
			std::tie(x, y, z) = prev_img(i, j);*/

			/*x = (x - min.x)/(max.x-min.x);
			y = (y - min.y)/(max.y-min.y);
			z = (z - min.z)/(max.z-min.z);*/
			
			/*x = (x - min_p)/(max_p-min_p);
			y = (y - min_p)/(max_p-min_p);
			z = (z - min_p)/(max_p-min_p);
			vec3 color(x,y,z); 
			float CONSTANT = 255*1.5; //немного странное решение
			color = glm::max(glm::min(color*CONSTANT, 255.0f), 0.0f);
			img(i, j) = std::make_tuple( color.x/1, color.y/1, color.z/1); 
		}
	}*/

	for(int i=0; i<IMG_ROWS-1; i+=2){
		for(int j=0; j<IMG_COLS-1; j+=2){
			float mean_x=0, mean_y=0, mean_z=0;
			for(int k = 0; k < 2; k++)
				for(int l = 0; l < 2; l++)
				{
					float x, y, z;
					std::tie(x, y, z) = prev_img(i + k, j + l);
					x = (x - min_p)/(max_p-min_p);
					y = (y - min_p)/(max_p-min_p);
					z = (z - min_p)/(max_p-min_p);
					mean_x += x;
					mean_y += y;
					mean_z += z;
				}
			mean_x = mean_x/4.0;
			mean_y = mean_y/4.0;
			mean_z = mean_z/4.0;
			
			vec3 color(mean_x, mean_y, mean_z); 
			float CONSTANT = 255*1.5; //немного странное решение
			color = glm::max(glm::min(color*CONSTANT, 255.0f), 0.0f);
			img(i/2, j/2) = std::make_tuple( color.x/1, color.y/1, color.z/1); 
		}
	}
	save_image(img, "box8_ao_0.1_1.bmp");
	return 0;
}