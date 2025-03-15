#include "shared/pch.hh"
#include "shared/splash.hh"

constexpr static const char *SPLASHES_FILENAME = "misc/splashes.txt";

static std::mt19937_64 splash_random;
static std::vector<std::string> splash_lines;

void splash::init(void)
{
    if(auto file = PHYSFS_openRead(SPLASHES_FILENAME)) {
        auto source = std::string(PHYSFS_fileLength(file), char(0x00));
        PHYSFS_readBytes(file, source.data(), source.size());
        PHYSFS_close(file);

        std::string line;
        std::istringstream stream(source);

        while(std::getline(stream, line)) {
            splash_lines.push_back(line);
        }

        splash_random.seed(std::random_device()());
    }
    else {
        splash_lines.push_back(fmt::format("{}: {}", SPLASHES_FILENAME, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
    }
}

const char* splash::get(void)
{
    std::uniform_int_distribution<std::size_t> dist(0, splash_lines.size() - 1);
    return splash_lines.at(dist(splash_random)).c_str();
}
