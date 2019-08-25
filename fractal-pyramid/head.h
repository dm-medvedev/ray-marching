#include "../common/glm/glm.hpp"
#include "../common/glm/gtc/matrix_transform.hpp"
#include "../common/glm/gtc/type_ptr.hpp"
#include <vector>

using namespace glm;
using namespace std;

class Fractal: public Object
{
public:
	int Iterations;
	float Scale; // angle in degrees for glm::gtx::transform::rotate
	float x, y, z;
	vec2 h;
	float dist(vec3 z);
	Fractal(float x, float y, float z, int Iterations, float Scale, 
			vec3 color, Material material);
};
