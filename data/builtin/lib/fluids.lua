local fluids = {}

local HORIZONTAL = {
  { 1, 0, 0 },
  { -1, 0, 0 },
  { 0, 0, 1 },
  { 0, 0, -1 },
}

local SLOPE_INF = 1000

local function defaults(options)
  options = options or {}
  return {
    max_level = options.max_level or 8,
    delay = options.delay or 5,
    infinite = options.infinite ~= false,
    -- Minecraft water searches 4 blocks for a drop; lava uses 2.
    slope_range = options.slope_range or 4,
  }
end

--- Build `states` for a Minecraft-like fluid block.
function fluids.states(options)
  local cfg = defaults(options)
  local level_hints = {}

  for level = 1, cfg.max_level do
    level_hints[#level_hints + 1] = tostring(level)
  end

  return {
    level = {
      default = tostring(cfg.max_level),
      hint = level_hints,
    },
    source = {
      default = "false",
      hint = { "true", "false" },
    },
  }
end

--- Build `variants` mapping level/source to `fluid_level` (1/16ths).
--- Full fluid is 14/16 of a block — never a completely filled cube.
function fluids.variants(options)
  local cfg = defaults(options)
  local full_height = 14
  local result = {
    {
      when = { source = "true" },
      overrides = {
        fluid_level = full_height,
      },
    },
  }

  for level = 1, cfg.max_level do
    local height = math.max(1, math.floor(level * full_height / cfg.max_level))

    result[#result + 1] = {
      when = { source = "false", level = tostring(level) },
      overrides = {
        fluid_level = height,
      },
    }
  end

  return result
end

function fluids.on_place(options)
  local cfg = defaults(options)

  return function(bx, by, bz, target, occupant, actor)
    if not occupant.replaceable then
      return nil
    end

    world.schedule(bx, by, bz, cfg.delay)

    return {
      level = tostring(cfg.max_level),
      source = "true",
    }
  end
end

local function read_cell(block_id, bx, by, bz)
  if world.get_block(bx, by, bz) ~= block_id then
    return nil
  end

  return {
    level = tonumber(world.get_state(bx, by, bz, "level")) or 0,
    source = world.get_state(bx, by, bz, "source") == "true",
  }
end

local function effective_level(cell, max_level)
  if not cell then
    return 0
  end

  if cell.source then
    return max_level
  end

  return cell.level
end

local function is_solid(block_id, bx, by, bz)
  local id = world.get_block(bx, by, bz)

  if id == blocks.NULL_BLOCK or id == block_id then
    return false
  end

  return not blocks.is_replaceable(id)
end

local function can_occupy(block_id, bx, by, bz)
  local id = world.get_block(bx, by, bz)

  if id == block_id then
    return true
  end

  return blocks.is_replaceable(id)
end

local function write_cell(block_id, bx, by, bz, level, source)
  local existing = read_cell(block_id, bx, by, bz)

  if existing and existing.level == level and existing.source == source then
    return false
  end

  if not existing then
    if not blocks.is_replaceable(world.get_block(bx, by, bz)) then
      return false
    end

    world.set_block(bx, by, bz, block_id)
  end

  world.set_state(bx, by, bz, "level", tostring(level))
  world.set_state(bx, by, bz, "source", source and "true" or "false")
  return true
end

local function flow_into(block_id, cfg, bx, by, bz, level)
  if level < 1 or not can_occupy(block_id, bx, by, bz) then
    return false
  end

  local existing = read_cell(block_id, bx, by, bz)

  if existing then
    if existing.source then
      return false
    end

    if existing.level >= level then
      return false
    end
  end

  if not write_cell(block_id, bx, by, bz, level, false) then
    return false
  end

  -- Newly written cells are not themselves notified by set_block, so they
  -- must be scheduled or flow stops after the first ring of neighbours.
  world.schedule(bx, by, bz, cfg.delay)
  return true
end

local function compute_level(block_id, max_level, bx, by, bz)
  local above = read_cell(block_id, bx, by + 1, bz)

  -- Same fluid above → mesher draws a full-height column; stay non-source.
  if above then
    return max_level
  end

  local best = 0

  for _, d in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + d[1], by + d[2], bz + d[3])
    local value = effective_level(neighbour, max_level)

    if value > best then
      best = value
    end
  end

  return best - 1
end

local function try_infinite_source(block_id, bx, by, bz)
  if not is_solid(block_id, bx, by - 1, bz) then
    return false
  end

  local sources = 0

  for _, d in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + d[1], by + d[2], bz + d[3])

    if neighbour and neighbour.source then
      sources = sources + 1

      if sources >= 2 then
        return true
      end
    end
  end

  return false
end

-- Shortest walk from (bx,by,bz) to a cell that can fall, capped at max_depth.
-- `from_dx/from_dz` is the step we just took, so we never reverse immediately.
local function slope_distance(block_id, bx, by, bz, depth, max_depth, from_dx, from_dz)
  if can_occupy(block_id, bx, by - 1, bz) then
    return depth
  end

  if depth >= max_depth then
    return SLOPE_INF
  end

  local best = SLOPE_INF

  for _, d in ipairs(HORIZONTAL) do
    if d[1] ~= -from_dx or d[3] ~= -from_dz then
      local nx = bx + d[1]
      local nz = bz + d[3]

      if can_occupy(block_id, nx, by, nz) then
        local dist = slope_distance(block_id, nx, by, nz, depth + 1, max_depth, d[1], d[3])

        if dist < best then
          best = dist
        end
      end
    end
  end

  return best
end

-- Minecraft: assign each side a weight = distance to nearest drop (or 1000),
-- then only spread into the minimum-weight directions.
local function preferred_spread_dirs(block_id, cfg, bx, by, bz)
  local best = SLOPE_INF
  local dirs = {}

  for _, d in ipairs(HORIZONTAL) do
    local nx = bx + d[1]
    local ny = by + d[2]
    local nz = bz + d[3]

    if can_occupy(block_id, nx, ny, nz) then
      local weight = slope_distance(block_id, nx, ny, nz, 1, cfg.slope_range, d[1], d[3])

      if weight < best then
        best = weight
        dirs = { d }
      elseif weight == best then
        dirs[#dirs + 1] = d
      end
    end
  end

  return dirs
end

local function update_fluid(block_id, cfg, bx, by, bz)
  local cell = read_cell(block_id, bx, by, bz)

  if not cell then
    return
  end

  local level = effective_level(cell, cfg.max_level)
  local source = cell.source

  if cfg.infinite and try_infinite_source(block_id, bx, by, bz) then
    if not source or cell.level ~= cfg.max_level then
      write_cell(block_id, bx, by, bz, cfg.max_level, true)
      source = true
      level = cfg.max_level
    end
  elseif not source then
    local new_level = compute_level(block_id, cfg.max_level, bx, by, bz)

    if new_level < 1 then
      world.set_block(bx, by, bz, blocks.NULL_BLOCK)
      return
    end

    if new_level ~= cell.level then
      write_cell(block_id, bx, by, bz, new_level, false)
      level = new_level
    end
  end

  if can_occupy(block_id, bx, by - 1, bz) then
    flow_into(block_id, cfg, bx, by - 1, bz, cfg.max_level)
  end

  local spread = level - 1

  if spread < 1 then
    return
  end

  for _, d in ipairs(preferred_spread_dirs(block_id, cfg, bx, by, bz)) do
    flow_into(block_id, cfg, bx + d[1], by + d[2], bz + d[3], spread)
  end
end

--- Produce an `on_tick` handler for a fluid block family.
---
--- options:
---   max_level (default 8)
---   delay (default 5) — ticks between neighbour debounce and flow update
---   infinite (default true) — 2+ adjacent sources form a new source
---   slope_range (default 4) — how far to search for a drop when choosing sides
function fluids.on_tick(block_name, options)
  local cfg = defaults(options)

  return function(bx, by, bz, source)
    if source == blocks.TICK_NEIGHBOUR then
      world.schedule(bx, by, bz, cfg.delay)
      return
    end

    if source ~= blocks.TICK_SCRIPTED and source ~= blocks.TICK_RANDOM then
      return
    end

    update_fluid(blocks.get(block_name), cfg, bx, by, bz)
  end
end

return fluids
