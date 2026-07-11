#ifndef F8834F01_219B_4D3A_86E7_91FA52B4F625
#define F8834F01_219B_4D3A_86E7_91FA52B4F625

#include "shared/coord.hh"

namespace config
{
template<typename T>
class Ref;
} // namespace config

class Camera;

namespace camera
{
extern config::Ref<float> vertical_fov;
extern config::Ref<unsigned> view_distance;
} // namespace camera

namespace camera
{
extern Camera instance;
} // namespace camera

namespace camera
{
extern Eigen::Vector3f angles;
extern Eigen::Vector3f forward;
extern Eigen::Vector3f right;
extern Eigen::Vector3f up;
extern Eigen::Vector3f local;
extern chunk_pos chunk;
} // namespace camera

namespace camera
{
void init(void);
void update(void);
} // namespace camera

#endif /* F8834F01_219B_4D3A_86E7_91FA52B4F625 */
