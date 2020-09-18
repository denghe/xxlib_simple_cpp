local this = ...
local t = {}
t.i = 2
this:Set_onUpdate(function()
    t.i = t.i + 1
    this:Set_n(t.i)
end)
this:Set_onSerialize(function()
    GT = t
end)
this:Set_onDeserialize(function()
    local t_ = GT
    for k, v in pairs(t_) do
        t[k] = v
    end
end)
this:Set_onToStringCore(function()
    return ",\"lua_t\":{\"i\":"..t.i.."}"
end)
