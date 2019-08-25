#include "../common/glm/glm.hpp"
#include "../common/glm/gtc/matrix_transform.hpp"
#include "../common/glm/gtc/type_ptr.hpp"
#include "../common/io.h"
#include "../common/common.h"
#include "head.h"

#define MAX_COUNT_STEPS 50
#define MIN_STEP 0.001/10
#define STOP_STEP 0.05/10
#define IMG_ROWS 512*2+1 // odd
#define IMG_COLS 512*2+1 // odd
#define CAMERA_IMG_DIST 2
#define PIX_IMG_SIZE 0.1/100 // distance between two  adjacent rays that go from camera to IMG
#define SHADE_CONST 1
#define REFL_COUNT 2
#define REFR_COUNT 10
using namespace glm;

vec3 EstimateNormal(Object* obj, vec3 z) {
	float eps = 1 / 100.0;
	vec3 z1 = z + vec3(eps, 0, 0);
	vec3 z2 = z - vec3(eps, 0, 0);
	vec3 z3 = z + vec3(0, eps, 0);
	vec3 z4 = z - vec3(0, eps, 0);
	vec3 z5 = z + vec3(0, 0, eps);
	vec3 z6 = z - vec3(0, 0, eps);
	float dx = obj->dist(z1) - obj->dist(z2);
	float dy = obj->dist(z3) - obj->dist(z4);
	float dz = obj->dist(z5) - obj->dist(z6);
	float coef = (1 / (2.0 * eps));

	return normalize( vec3(dx, dy, dz) * coef );
}

bool Visible(const vec3 &hit_point, const Light &light, const Scene &scene, int obj_num) {
	int steps_count = 0;
	vec3 p = hit_point;
	vec3 dir = normalize(light.p - p);
	float dist = length(light.p - p);
	float min = 0, t = 0;
	int start = (0 != obj_num) ? 0 : 1;
	if (scene.objects.size() == 1) {
		p += (1.0f) * dir;
		while (steps_count < MAX_COUNT_STEPS) {
			min = scene.objects[0]->dist(p);
			if (MIN_STEP > min) min = MIN_STEP;
			p +=  min * dir;
			t += min;
			steps_count++;
			if (min < STOP_STEP and t <= dist) return false;
			if (min < STOP_STEP and t > dist) return true;
		}
		return true;
	}
	while (steps_count < MAX_COUNT_STEPS) {
		min = scene.objects[start]->dist(p);
		for (int i = start + 1; i < scene.objects.size(); ++i) {
			float curr = 0;
			if (i != obj_num) {
				curr = scene.objects[i]->dist(p);
				if (curr < min) min = curr;
			}
		}
		if (MIN_STEP > min) min = MIN_STEP;
		p +=  min * dir;
		t += min;
		steps_count++;
		if (min < STOP_STEP and t <= dist) return false;
		if (min < STOP_STEP and t > dist) return true;
	}
	return true;
}

float AmbientOcclusion(vec3 point , vec3 normal , vec3 color, const Scene &scene, float delta, float k) {
	float ao = 0, coef1 = 1.0;
	for (int i = 0; i < 10; i++) {
		vec3 p = point + normal * (i * delta);
		coef1 = 9 * coef1 / 10.0;

		float rho = scene.objects[0]->dist(p);
		for (int j = 1; j < scene.objects.size(); ++j) {
			float curr = scene.objects[j]->dist(p);
			if (curr < rho) rho = curr;
		}

		if (rho < 0) break;
		ao += coef1 * ((i + 1) * delta - rho);
	}
	return 1 - ao * k;
}

Hit RaySceneIntersection(const Ray& ray, const Scene &scene) {
	int steps_count = 0;
	vec3 p = ray.pos;
	float min = 0, t = 0;
	int obj_num;
	while (steps_count < MAX_COUNT_STEPS) {
		min = scene.objects[0]->dist(p);
		obj_num = 0;
		for (int i = 1; i < scene.objects.size(); ++i) {
			float curr = 0;
			curr = scene.objects[i]->dist(p);
			if (curr < min) {min = curr; obj_num = i;}
		}
		if (MIN_STEP > min) min = MIN_STEP;
		p +=  min * ray.dir;
		t += min;
		steps_count++;
		if (min < STOP_STEP) {
			vec3 norm = EstimateNormal(scene.objects[obj_num], p);
			return Hit(true, t, scene.objects[obj_num]->material, norm, obj_num);
		}
	}
	return Hit(false, 0, scene.objects[0]->material);
}

Ray reflect(const Ray& ray, const Hit& hit, vec3 hit_point, const Scene &scene) {
	vec3 p = hit_point;
	float min = scene.objects[0]->dist(p);
	int obj_num = 0;

	for (int i = 1; i < scene.objects.size(); ++i) {

		float curr = 0;

		curr = scene.objects[i]->dist(p);
		if (curr < min) {min = curr; obj_num = i;}
	}
	if (MIN_STEP > min) min = MIN_STEP;

	vec3 new_dir = normalize(glm::reflect(ray.dir, hit.normal));

	return Ray(hit_point + new_dir, new_dir);
}

vec3 RayTrace(const Ray& ray, const Scene &scene, std::vector<Light> lights, int count = 0) {
	vec3 color(0.0f, 0.0f, 0.0f);

	count += 1;
	Hit hit = RaySceneIntersection(ray, scene);
	if (!hit.exist) {
		return color;
	}

	vec3 hit_point = ray.pos + ray.dir * hit.t;

	for (int i = 0; i < lights.size(); i++)
		if (Visible(hit_point, lights[i], scene, hit.obj_num)) {
			vec3 shd = Shade(hit_point, hit.normal, lights[i]);
			color.x += (scene.objects[hit.obj_num]->color).x * shd.x;
			color.y += (scene.objects[hit.obj_num]->color).y * shd.y;
			color.z += (scene.objects[hit.obj_num]->color).z * shd.z;
		}

	if (hit.material.reflection > 0 and count < REFL_COUNT) {
		Ray reflRay = reflect(ray, hit, hit_point, scene);
		vec3 refl_color = RayTrace(reflRay, scene, lights, count);
		if (refl_color == vec3(0, 0, 0))
			count = REFL_COUNT;
		color += hit.material.reflection * refl_color;
	}

	//Ambient Occlusion
	color = AmbientOcclusion(hit_point, hit.normal, color, scene, 0.002, 10) * color;
	return color;
}

Fractal::Fractal(float x, float y, float z, int Iterations, float Bailout,
                 float Power, vec3 color, Material material)
	: Object(inverse(glm::translate( glm::mat4(1.0f), vec3(x, y, z))), color, material),
	  x(x), y(y), z(z), Iterations(Iterations), Bailout(Bailout), Power(Power) {}

float Fractal::dist(vec3 pos) {
	vec3 z = pos;
	vec4 new_p(z.x, z.y, z.z, 1);
	new_p = transform_mat * new_p; // translates into a coordinate system relative to the sphere
	vec3 last_p(new_p);
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < Iterations ; i++) {
		r = length(last_p);
		if (r > Bailout) break;

		// convert to polar coordinates
		float theta = acos(last_p.z / r);
		float phi = atan(last_p.y, last_p.x);
		dr =  pow( r, Power - 1.0) * Power * dr + 1.0;

		// scale and rotate the point
		float zr = pow(r, Power);
		theta = theta * Power;
		phi = phi * Power;

		// convert back to cartesian coordinates
		last_p = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
		last_p += pos;
	}
	return 0.5 * log(r) * r / dr;
}

int main() {
	std::vector<Light> lights;
	float intens = 2;
	lights.push_back(Light(vec3(20 * intens, 40 * intens, 20 * intens), vec3(2.5, -1.5, -0.5)));
	lights.push_back(Light(vec3(40 * intens, 20 * intens, 40 * intens), vec3(2.5, 2.5, 2.5 / 4.0)));

	Scene scene;
	Fractal it0 = Fractal(0, 0, 0, //x,y,z
	                      15, 5, 8, //Iters, Bailout, Power
	                      vec3(1 , 1, 1),// color
	                      Material(0, 0));//mater

	Object *item0 = &it0;
	scene.objects.push_back(item0);
	Image img(512, 512); //IMG_ROWS, IMG_COLS
	Matrix <std::tuple<float, float, float>> prev_img(IMG_ROWS, IMG_COLS);

	float my_dist = 3.5 + 2;
	vec3 eye(my_dist + CAMERA_IMG_DIST / 3, my_dist / 2.0 + CAMERA_IMG_DIST / 3,
	         my_dist / 5.0 + CAMERA_IMG_DIST / 3); // coordinates of camera in the world system
	vec3 center(0, 0, 0);
	vec3 up(0, 1, 0);
	mat4 M_view = lookAt(eye, center, up); // translate world coordinates to coordinates related with camera
	mat4 inverse_M_view = inverse(M_view);
	int ind_center_row_img = (IMG_ROWS - 1) / 2;
	int ind_center_col_img = (IMG_COLS - 1) / 2;

	vec3 min(1000000, 1000000, 1000000), max(-1, -1, -1);

	#pragma omp parallel for collapse(2) schedule(dynamic, 512)
	for (int i = 0; i < IMG_ROWS; ++i) {
		for (int j = 0; j < IMG_COLS; ++j) {
			vec3 v((i - ind_center_row_img)*PIX_IMG_SIZE,
			       (j - ind_center_col_img)*PIX_IMG_SIZE,
			       -1 * CAMERA_IMG_DIST);
			vec4 camera_pix_point_coords( v.x, v.y, v.z, 1);
			vec3 n_v = normalize(v);
			vec4 camera_pix_dir_coords( n_v.x, n_v.y, n_v.z, 0);
			vec3 pos(inverse_M_view * camera_pix_point_coords);
			vec3 dir(inverse_M_view * camera_pix_dir_coords);
			Ray ray(pos, dir);
			vec3 color = RayTrace(ray, scene, lights);

			prev_img(i, j) = std::make_tuple(color.x, color.y, color.z);
			if (min.x > color.x) min.x = color.x;
			if (min.y > color.y) min.y = color.y;
			if (min.z > color.z) min.z = color.z;
			if (max.x < color.x) max.x = color.x;
			if (max.y < color.y) max.y = color.y;
			if (max.z < color.z) max.z = color.z;
		}
	}
	std::cout << "min: " << min.x << " | " << min.y << " | " << min.z << std::endl;
	std::cout << "max: " << max.x << " | " << max.y << " | " << max.z << std::endl;
	float min_p = min.x, max_p = max.x;
	if (min.y < min_p) min_p = min.y;
	if (min.z < min_p) min_p = min.z;
	if (max.y > max_p) max_p = max.y;
	if (max.z > max_p) max_p = max.z;

	for (int i = 0; i < IMG_ROWS - 1; i += 2) {
		for (int j = 0; j < IMG_COLS - 1; j += 2) {
			float mean_x = 0, mean_y = 0, mean_z = 0;
			for (int k = 0; k < 2; k++)
				for (int l = 0; l < 2; l++) {
					float x, y, z;
					std::tie(x, y, z) = prev_img(i + k, j + l);
					x = (x - min_p) / (max_p - min_p);
					y = (y - min_p) / (max_p - min_p);
					z = (z - min_p) / (max_p - min_p);
					mean_x += x;
					mean_y += y;
					mean_z += z;
				}
			mean_x = mean_x / 4.0;
			mean_y = mean_y / 4.0;
			mean_z = mean_z / 4.0;

			vec3 color(mean_x, mean_y, mean_z);
			float CONSTANT = 255 * 1.5;
			color = glm::max(glm::min(color * CONSTANT, 255.0f), 0.0f);
			img(i / 2, j / 2) = std::make_tuple( color.x / 1, color.y / 1, color.z / 1);
		}
	}
	save_image(img, "result/cabbage.bmp");
	return 0;
}