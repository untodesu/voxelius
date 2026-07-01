blocks.add("vgame::dirt", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "block/cube.json",
    animated = false,

    textures = {
        default = [
            "block/dirt01.png",
            "block/dirt02.png",
            "block/dirt03.png",
            "block/dirt04.png",
        ]
    }
})

blocks.tick("vgame::dirt", function(world, bx, by, bz)
    local above = world.bget(bx, by + 1, bz)
    local light = world.lgetv(bx, by + 1, bz)

    if above ~= blocks.NULL_BLOCK or light < 9 then
        return false
    end

    local has_grass = false
    local grass_id = blocks.get("vgame::grass")

    for dx = -1, 1 do
        for dz = -1, 1 do
            local block = world.bget(bx + dx, by, bz + dz)

            if block == grass_id then
                has_grass = true
                break
            end
        end

        if has_grass then
            break
        end
    end

    if has_grass then
        world.bset(bx, by, bz, grass_id)
        return true
    end

    return false
end)
