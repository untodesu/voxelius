blocks.add("builtin:dirt", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "builtin:cube",
    animated = false,

    textures = {
        default = [
            "builtin:dirt01.png",
            "builtin:dirt02.png",
            "builtin:dirt03.png",
            "builtin:dirt04.png",
        ]
    },

    hardness = 0.5,
    tool = blocks.TOOL_SHOVEL,
    sounds = "builtin:dirt",

    drops = [
        {
            item = "builtin:dirt",
            count = 1
        }
    ],

    random_tick = function(world, bx, by, bz)
        local above = world.bget(bx, by + 1, bz)
        local light = world.lgetv(bx, by + 1, bz)

        if above ~= blocks.NULL_BLOCK or light < 9 then
            return
        end

        local grass_id = blocks.get("builtin:grass")
        local has_grass = false

        for x = -1, 1 do
            for z = -1, 1 do
                local block = world.bget(bx + x, by, bz + z)
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
    end
})
