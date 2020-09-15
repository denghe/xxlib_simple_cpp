return function( this )
    local t = {}
    t.n = 0

    this:Set_onUpdate(function()
        t.n = t.n + 2
        this:Set_n(t.n)
    end)

    this:Set_onLoadData(function(t_)
        -- todo: copy t.* to t
    end)

    this:Set_onSaveData(function()
        return t
    end)
end
