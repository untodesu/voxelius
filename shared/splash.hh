#ifndef F83DEC19_C701_4154_8E7D_362093A1EDBA
#define F83DEC19_C701_4154_8E7D_362093A1EDBA

constexpr static std::string_view SPLASH_CLIENT = "splashes_client";
constexpr static std::string_view SPLASH_SERVER = "splashes_server";

namespace splash
{
void init(std::string_view filename);
} // namespace splash

namespace splash
{
std::string_view get(void);
} // namespace splash

#endif /* F83DEC19_C701_4154_8E7D_362093A1EDBA */
