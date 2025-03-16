#include "shared/pch.hh"
#include "shared/splash.hh"

constexpr static const char *SPLASHES_FILENAME_CLIENT = "misc/splashes_client.txt";
constexpr static const char *SPLASHES_FILENAME_SERVER = "misc/splashes_server.txt";
constexpr static std::size_t SPLASH_SERVER_MAX_LENGTH = 32;

static std::mt19937_64 splash_random;
static std::vector<std::string> splash_lines;

static std::string sanitize_line(const std::string &line)
{
    std::string result;

    for(auto chr : line) {
        if((chr == '\r') || (chr == '\n'))
            continue;
        result.push_back(chr);
    }

    return result;
}

static void splash_init_filename(const char *filename)
{
    if(auto file = PHYSFS_openRead(filename)) {
        auto source = std::string(PHYSFS_fileLength(file), char(0x00));
        PHYSFS_readBytes(file, source.data(), source.size());
        PHYSFS_close(file);

        std::string line;
        std::istringstream stream(source);

        while(std::getline(stream, line))
            splash_lines.push_back(sanitize_line(line));
        splash_random.seed(std::random_device()());
    }
    else {
        splash_lines.push_back(fmt::format("{}: {}", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())));
        splash_random.seed(std::random_device()());
    }
}

void splash::init_client(void)
{
    splash_init_filename(SPLASHES_FILENAME_CLIENT);
}

void splash::init_server(void)
{
    splash_init_filename(SPLASHES_FILENAME_SERVER);

    // Server browser GUI should be able to display
    // these splash messages without text clipping over
    for(int i = 0; i < splash_lines.size(); i++) {
        splash_lines[i] = splash_lines[i].substr(0, SPLASH_SERVER_MAX_LENGTH);
    }
}

const char* splash::get(void)
{
    std::uniform_int_distribution<std::size_t> dist(0, splash_lines.size() - 1);
    return splash_lines.at(dist(splash_random)).c_str();
}
