#ifndef CORE_PCH_HH
#define CORE_PCH_HH 1
#pragma once

#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <emhash/hash_table8.hpp>

#include <enet/enet.h>

#include <glm/fwd.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <physfs.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <stb_image.h>
#include <stb_image_write.h>

#endif /* CORE_PCH_HH */
