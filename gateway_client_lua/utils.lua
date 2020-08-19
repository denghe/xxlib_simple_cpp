-- 表内容转为 string 并返回( 创建 table 的 lua 语句 style )
function ToString(o)
    if type(o) == 'table' then
        local s = '{'
        for k, v in pairs(o) do
            s = s .. '[' .. ToString(k) .. ']=' .. ToString(v) .. ','
        end
        if #o > 0 then
            s = string.sub(s, 0, #s - 1)
        end
        return s .. '}'
    elseif type(o) == 'string' then
        return '"' .. tostring(o) .. '"'
    else
        return tostring(o)
    end
end

-- 打印表内容
function Dump(o)
    print(ToString(o))
end

-- 深拷贝成员到另一个表
function CopyTo(src, dest)
    for k, v in pairs(src) do
        if type(v) == "table" then
            dest[k] = {}
            CopyTo(v, dest[k])
        else
            dest[k] = v
        end
    end
end

-- 构造简单的 List 结构
function MakeList()
    local t = {}

    -- 追加一个元素并返回其下标
    t.Add = function(o)
        local idx = #t + 1
        t[idx] = o
        return idx
    end

    -- 和最后一个成员交换移除。如果成员有存自己所在 idx, 则可能要先同步: t[#t].idx = idx
    t.SwapRemoveAt = function(idx)
        local len = #t
        assert(idx > 0 and idx <= len)
        if idx < len then
            t[idx] = t[len]
        end
        t[len] = nil
    end

    -- 正序查找一个成员, 返回下标。返回 -1 表示没找到
    t.Find = function(o)
        for i = 1, #t do
            if t[i] == o then
                return i
            end
        end
        return -1
    end

    -- 正序遍历所有成员并将成员套用到传入的执行函数中。 cb 参数为 item, index. 如果 cb 返回 false/nil 则停止遍历
    t.Foreach = function(cb)
        for i = 1, #t do
            if not cb(t[i], i) then
                return
            end
        end
    end

    -- 倒序遍历所有成员并将成员套用到传入的执行函数中。 cb 参数为 item, index. 如果 cb 返回 false/nil 则停止遍历
    t.ForeachReverse = function(cb)
        for i = #t, 1, -1 do
            if not cb(t[i], i) then
                return
            end
        end
    end

    return t
end

-- 脚本对象容器
scripts = scripts or { }

-- 用于加载脚本时自动分配一个 scripts 里面的 key
scriptsAutoId = scriptsAutoId or 0

-- 自增 scriptsAutoId 并返回
function MakeScriptId()
    scriptsAutoId = scriptsAutoId + 1
    return scriptsAutoId
end

-- 检查 t 是否符合脚本对象接口. 返回 nil 表示没问题
function CheckScript(t)
    if type(v) ~= "table" then
        return "t is not table"
    end
    if t.MakeArgs == nil then
        -- 参数说明
        return "t.MakeArgs == nil"
    end
    if t.Create == nil then
        -- 构造
        return "t.Create == nil"
    end
    if t.Dispose == nil then
        -- 析构
        return "t.Dispose == nil"
    end
    if t.Update == nil then
        -- 更新
        return "t.Update == nil"
    end
    if t.CollisionDetect == nil then
        -- 碰撞检测
        return "t.MoveTo == nil"
    end
    if t.Event == nil then
        -- 事件通知
        return "t.MakeArgs == nil"
    end
end

-- 加载一个脚本对象. 成功返回 id
function AddScript(t)
    local id = MakeScriptId()
    scripts[id] = t
    return id
end

-- 卸载一个脚本对象
function RemoveScript(id)
    scripts[id].Dispose()
    scripts[id] = nil
end

-- 执行所有脚本对象( call 其 Update )
function UpdateScripts(elapsedSeconds)
    local d = {}
    for k, v in pairs(scripts) do
        if not v.Update(elapsedSeconds) then
            v.Dispose()
            d[#d + 1] = k
        end
    end
    for i = #d, 1, -1 do
        scripts[d[i]] = nil
    end
end

-- 加载指定文件，创建脚本对象
function MakeScript(fn)
    return require(fn)()
end
