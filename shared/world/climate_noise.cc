#include "shared/pch.hh"

#include "shared/world/climate_noise.hh"

#include "shared/world/noise_cache_2D.hh"

static std::unique_ptr<NoiseCache2D_8x8> s_temperature;
static std::unique_ptr<NoiseCache2D_8x8> s_humidity;
static std::unique_ptr<NoiseCache2D_8x8> s_continentalness;
static std::unique_ptr<NoiseCache2D_8x8> s_erosion;
static std::unique_ptr<NoiseCache2D_8x8> s_weirdness;

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

    s_temperature = std::make_unique<NoiseCache2D_8x8>(std::move(temperature));
    s_humidity = std::make_unique<NoiseCache2D_8x8>(std::move(humidity));
    s_continentalness = std::make_unique<NoiseCache2D_8x8>(std::move(continentalness));
    s_erosion = std::make_unique<NoiseCache2D_8x8>(std::move(erosion));
    s_weirdness = std::make_unique<NoiseCache2D_8x8>(std::move(weirdness));
}

void climate_noise::shutdown(void)
{
    s_temperature.reset();
    s_humidity.reset();
    s_continentalness.reset();
    s_erosion.reset();
    s_weirdness.reset();
}

ClimateSample climate_noise::sample_block(const BlockPosXZ& pos)
{
    ClimateSample sample {};
    sample.temperature = s_temperature->get_slow(pos);
    sample.humidity = s_humidity->get_slow(pos);
    sample.continentalness = s_continentalness->get_slow(pos);
    sample.erosion = s_erosion->get_slow(pos);
    sample.weirdness = s_weirdness->get_slow(pos);
    return sample;
}

ClimateSampleArray climate_noise::sample_array(const ChunkPosXZ& pos)
{
    ClimateSampleArray samples {};

    auto& temperature = s_temperature->get(pos);
    auto& humidity = s_humidity->get(pos);
    auto& continentalness = s_continentalness->get(pos);
    auto& erosion = s_erosion->get(pos);
    auto& weirdness = s_weirdness->get(pos);

    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        LocalPosXZ lpos;
        lpos[0] = static_cast<LocalPosXZ::value_type>(i % constant::CHUNK_SIZE);
        lpos[1] = static_cast<LocalPosXZ::value_type>(i / constant::CHUNK_SIZE);

        samples[i].temperature = NoiseCache2D_8x8::sample(temperature, lpos);
        samples[i].humidity = NoiseCache2D_8x8::sample(humidity, lpos);
        samples[i].continentalness = NoiseCache2D_8x8::sample(continentalness, lpos);
        samples[i].erosion = NoiseCache2D_8x8::sample(erosion, lpos);
        samples[i].weirdness = NoiseCache2D_8x8::sample(weirdness, lpos);
    }

    return samples;
}
