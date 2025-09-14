#include "shared/pch.hh"

#include "shared/splash.hh"

#include "core/io/physfs.hh"

constexpr static std::string_view SPLASHES_FILENAME_CLIENT = "misc/splashes_client.txt";
constexpr static std::string_view SPLASHES_FILENAME_SERVER = "misc/splashes_server.txt";
constexpr static std::size_t SPLASH_SERVER_MAX_LENGTH = 32;

static std::mt19937_64 splash_random;
static std::vector<std::string> splash_lines;

static std::string sanitize_line(const std::string& line)
{
    std::string result;

    for(auto chr : line) {
        if(chr != '\r' && chr != '\n') {
            result.push_back(chr);
        }
    }

    return result;
}

static void splash_init_filename(std::string_view filename)
{
    if(auto file = PHYSFS_openRead(std::string(filename).c_str())) {
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
        splash_lines.push_back(std::format("{}: {}", filename, io::physfs_error()));
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

std::string_view splash::get(void)
{
    std::uniform_int_distribution<std::size_t> dist(0, splash_lines.size() - 1);
    return splash_lines.at(dist(splash_random));
}
