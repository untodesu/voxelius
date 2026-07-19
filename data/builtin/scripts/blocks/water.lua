local level_hints = {}
local level_variants = {}

for level = 1, 14 do
  local level_str = tostring(level)
  local level_variant = {
    when = { level = level_str },
    overrides = {
      fluid_level = level,
    },
  }

  level_hints[#level_hints + 1] = level_str
  level_variants[#level_variants + 1] = level_variant
end

blocks.add("water", {
  render = blocks.RENDER_NONE,

  fluid_name = "water",

  replaceable = true,
  touch = blocks.TOUCH_NONE,

  emission = 0,
  dissipation = 1,

  states = {
    level = {
      default = "14",
      hint = level_hints,
    },
  },

  variants = level_variants,
})
