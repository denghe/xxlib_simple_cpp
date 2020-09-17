local this = ...
local t = {}
t.n = 2
--this:Set_onUpdate(function()
--    this:Set_n(t.n)
--end)
this:Set_onLoadData(function(t_)
    for k, v in pairs(t_) do
        t[k] = v
    end
end)
this:Set_onSaveData(function()
    return t
end)
