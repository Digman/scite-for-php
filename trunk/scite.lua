------------------------------------------------
-- 工具
------------------------------------------------

local is_word_char = function (char)
    local strChar = string.char(char)
    local beginIndex = string.find(strChar, '%w')
    if beginIndex ~= nil then
        return true
    end
    if strChar == '_' then
        return true
    end
    return false
end

------------------------------------------------
-- 事件
------------------------------------------------

local on_open_handlers = {
    function (filename)
    end,
    function (filename)
    end
}
function OnOpen(filename)
    for i, handler in ipairs(on_open_handlers) do
        handler(filename)
    end
end


-- 选择单词
function select_word()
    local beginPos = editor.CurrentPos
    local endPos   = beginPos
    while is_word_char(editor.CharAt[beginPos - 1]) do
        beginPos = beginPos - 1
    end
    while is_word_char(editor.CharAt[endPos]) do
        endPos = endPos + 1
    end
    if beginPos ~= endPos then
        editor.SelectionStart = beginPos
        editor.SelectionEnd   = endPos
    end
end

-- php suit script
--dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\SciTePHPSuit.lua")
-- show calltips
dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\ShowCalltip.lua")
-- COMMON.lua
dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\COMMON.lua")
-- Extman.lua
--dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\Extman.lua")
-- Addition.lua
dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\Addition.lua")
-- SideBar.lua
dofile (props["SciteDefaultHome"].."\\other\\third\\tools\\SideBar.lua")