#include "shared/pch.hh"

#include "shared/world/climate_noise.hh"

#include "shared/world/noise_cache.hh"

static std::unique_ptr<NoiseCache_2D> s_temperature;
static std::unique_ptr<NoiseCache_2D> s_humidity;
static std::unique_ptr<NoiseCache_2D> s_continentalness;
static std::unique_ptr<NoiseCache_2D> s_erosion;
static std::unique_ptr<NoiseCache_2D> s_weirdness;

void climate_noise::init(std::mt19937_64& seeder)
{
    fnl_state temperature = fnlCreateState();
    temperature.noise_type = FNL_NOISE_OPENSIMPLEX2;
    temperature.frequency = 0.0007f;
    temperature.seed = static_cast<int>(seeder());

    fnl_state humidity = fnlCreateState();
    humidity.noise_type = FNL_NOISE_OPENSIMPLEX2;
    humidity.frequency = 0.0007f;
    humidity.seed = static_cast<int>(seeder());

    fnl_state continentalness = fnlCreateState();
    continentalness.noise_type = FNL_NOISE_OPENSIMPLEX2;
    continentalness.frequency = 0.00035f;
    continentalness.seed = static_cast<int>(seeder());

    fnl_state erosion = fnlCreateState();
    erosion.noise_type = FNL_NOISE_OPENSIMPLEX2;
    erosion.frequency = 0.00075f;
    erosion.seed = static_cast<int>(seeder());

    fnl_state weirdness = fnlCreateState();
    weirdness.noise_type = FNL_NOISE_OPENSIMPLEX2;
    weirdness.frequency = 0.001f;
    weirdness.seed = static_cast<int>(seeder());

    s_temperature = std::make_unique<NoiseCache_2D>(std::move(temperature), Eigen::Vector2i(8, 8));
    s_humidity = std::make_unique<NoiseCache_2D>(std::move(humidity), Eigen::Vector2i(8, 8));
    s_continentalness = std::make_unique<NoiseCache_2D>(std::move(continentalness), Eigen::Vector2i(8, 8));
    s_erosion = std::make_unique<NoiseCache_2D>(std::move(erosion), Eigen::Vector2i(8, 8));
    s_weirdness = std::make_unique<NoiseCache_2D>(std::move(weirdness), Eigen::Vector2i(8, 8));
}

void climate_noise::shutdown(void)
{
    s_temperature.reset();
    s_humidity.reset();
    s_continentalness.reset();
    s_erosion.reset();
    s_weirdness.reset();
}

ClimateSample climate_noise::sample(const BlockPosXZ& pos)
{
    ClimateSample sample {};
    sample.temperature = s_temperature->sample(pos);
    sample.humidity = s_humidity->sample(pos);
    sample.continentalness = s_continentalness->sample(pos);
    sample.erosion = s_erosion->sample(pos);
    sample.weirdness = s_weirdness->sample(pos);
    return sample;
}
