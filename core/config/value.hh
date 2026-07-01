#ifndef E4110CD3_6B96_4EF8_A9E8_026E1D3FC381
#define E4110CD3_6B96_4EF8_A9E8_026E1D3FC381

namespace config
{
class IValue {
public:
    virtual ~IValue(void) = default;
    virtual std::string_view value(void) const noexcept = 0;
    virtual bool set_value(std::string_view new_value) noexcept = 0;
};
} // namespace config

#endif /* E4110CD3_6B96_4EF8_A9E8_026E1D3FC381 */
