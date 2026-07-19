local fluids = require("builtin:fluids.lua")

local FLUID = {
  max_level = 8,
  delay = 5,
  infinite = true,
}

blocks.add("water", {
  render = blocks.RENDER_NONE,

  fluid_name = "water",

  replaceable = true,
  touch = blocks.TOUCH_NONE,

  emission = 0,
  dissipation = 1,

  states = fluids.states(FLUID),
  variants = fluids.variants(FLUID),

  on_place = fluids.on_place(FLUID),
  on_tick = fluids.on_tick("water", FLUID),
})
