#include "client/pch.hh"

#include "client/camera.hh"

#include "core/camera.hh"
#include "core/config/ref.hh"
#include "core/utils/angles.hh"

#include "shared/component/head.hh"
#include "shared/component/transform.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/fluid_registry.hh"
#include "shared/world/world.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/gui/settings.hh"

config::Ref<float> camera::vertical_fov { 70.0f };
config::Ref<unsigned> camera::view_distance { 8 };

Camera camera::instance;

Eigen::Vector3f camera::angles;
Eigen::Vector3f camera::forward;
Eigen::Vector3f camera::right;
Eigen::Vector3f camera::up;
Eigen::Vector3f camera::local;
ChunkPos camera::chunk;

block_id_type camera::inside_block;
fluid_id_type camera::inside_fluid;

static LocalPos s_eye_lpos;
static BlockPos s_eye_bpos;

static void reset_camera(void)
{
    camera::angles.setZero();

    camera::forward = Eigen::Vector3f::UnitZ();
    camera::right = Eigen::Vector3f::UnitX();
    camera::up = Eigen::Vector3f::UnitY();

    camera::local.setZero();
    camera::chunk.setZero();

    camera::inside_block = BLOCK_ID_NULL;
    camera::inside_fluid = FLUID_ID_NULL;

    s_eye_lpos.setZero();
    s_eye_bpos.setZero();
}

void camera::init(void)
{
    vertical_fov.bind(globals::client_config, "camera.vertical_fov");
    view_distance.bind(globals::client_config, "camera.view_distance");

    settings::slider<float>(1, settings_location::GENERAL, "camera.vertical_fov", 45.0f, 110.0f, true, "%.0f");
    settings::slider<unsigned>(0, settings_location::VIDEO, "camera.view_distance", 1, 32, false);

    reset_camera();
}

void camera::update(void)
{
    if(!world::basic_entities.valid(globals::player)) {
        reset_camera();
        return;
    }

    const auto& head = world::basic_entities.get<Head_Component_Intr>(globals::player);
    const auto& transform = world::basic_entities.get<Transform_Intr>(globals::player);

    camera::angles = head.angles;
    camera::chunk = transform.chunk;
    camera::local = transform.local + head.offset;

    auto z_near = 0.01f;
    auto z_far = 1.25f * static_cast<float>(constant::CHUNK_SIZE * view_distance.value());

    instance.set_perspective(utils::radians(vertical_fov.value()), globals::aspect, z_near, z_far);
    instance.set_view(camera::local, camera::angles);
    instance.update();

    forward = instance.forward_vector();
    right = instance.right_vector();
    up = instance.up_vector();

    s_eye_lpos = camera::local.array().floor().cast<LocalPos::value_type>();
    s_eye_bpos = utils::to_block(camera::chunk, s_eye_lpos);

    inside_block = world::get_block(s_eye_bpos);
    inside_fluid = FLUID_ID_NULL;

    auto block_def = block_registry::find_definition(inside_block);

    if(block_def && block_def->fluid && block_def->fluid_level) {
        auto fluid_def = fluid_registry::find_definition(block_def->fluid);

        if(fluid_def) {
            auto frac_y = static_cast<float>(local.y() - static_cast<float>(s_eye_lpos.y()));
            auto height = 0.0625f * static_cast<float>(block_def->fluid_level);
            auto above = s_eye_bpos;

            if(fluid_def->gravity == FLUID_GRAVITY_DOWN) {
                above.y() += 1;
            }
            else {
                above.y() -= 1;
            }

            auto above_def = block_registry::find_definition(world::get_block(above));

            if(above_def && above_def->fluid == block_def->fluid) {
                height = 1.0f;
            }

            if(fluid_def->gravity == FLUID_GRAVITY_DOWN) {
                if(frac_y <= height) {
                    inside_fluid = block_def->fluid;
                }
            }
            else {
                if(frac_y >= (1.0f - height)) {
                    inside_fluid = block_def->fluid;
                }
            }
        }
    }
}
