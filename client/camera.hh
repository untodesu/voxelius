#ifndef F8834F01_219B_4D3A_86E7_91FA52B4F625
#define F8834F01_219B_4D3A_86E7_91FA52B4F625

#include "shared/coord.hh"
#include "shared/world/block.hh"
#include "shared/world/fluid.hh"

namespace gui
{
template<typename T>
class Slider;
} // namespace gui

class Camera;

namespace camera
{
extern gui::Slider<float> vertical_fov;
extern gui::Slider<unsigned> view_distance;
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
extern ChunkPos chunk;
} // namespace camera

namespace camera
{
extern block_id_type inside_block;
extern fluid_id_type inside_fluid;
} // namespace camera

namespace camera
{
void init(void);
void update(void);
} // namespace camera

#endif /* F8834F01_219B_4D3A_86E7_91FA52B4F625 */
