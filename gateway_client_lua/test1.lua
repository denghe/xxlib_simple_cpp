-- 返回一个创建脚本对象的函数
return function()
    local t = {}

    t.MakeArgs = function()
        return { lineGroupName = "xx" }
    end

    t.Create = function(c, ot)
        CopyTo(ot, t)
    end

    t.Dispose = function()
    end

    t.Update = function(elapsedSeconds)
        print("update " .. elapsedSeconds.." "..Now())
        Add(1, 2);
        return true
    end

    t.MoveTo = function(x, y, a)
    end

    t.CollisionDetect = function(x, y, r, id)
        return id
    end

    t.Event = function(...)
    end

    return t
end
