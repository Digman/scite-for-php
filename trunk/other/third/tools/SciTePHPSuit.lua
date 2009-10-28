--[[
Optimize SciTe For PHP
Powered by qining(http://www.joyqi.com)
version: 0.2
Features:
 * Support for autocomplete (both variable and function)
 * Support for PHP Doc
 * Something more...

Usage:
 * Copy the script file to your SciTe Path
 * Set "autocompleteword.automatic=0"
 * Add "ext.lua.startup.script=$(SciteDefaultHome)/SciTePHPSuit.lua" to your config properties
 * Restart your editor
]]

-- require("gui");

local definition = {['function'] = {};['variable'] = {};
['doc'] = {'access', 'author', 'copyright', 'license', 'package', 'param', 'return', 'throws', 'var', 'version'}}
focus = {['start'] = 0;['end'] = 0}
access = {['public'] = true;['private'] = true;['protected'] = true}

local function isInDefinition(name, part)
    for k, line in pairs(definition[part]) do
        if line == name then
            return true
        end
    end
    
    return false
end

local function findDefinition()
    local w,l = editor:GetText()

    definition['function'] = {}
    definition['variable'] = {}

    local functionPattern = "function ([_a-zA-Z0-9]+)\([a-zA-Z0-9,_ \$\=\&%s]*\)"
    for name, args in string.gmatch(w, functionPattern) do
        if false == isInDefinition(name, 'function') then
            table.insert(definition['function'], name)
        end
    end

    local variablePattern = "\$([_a-zA-Z0-9]+)"
    for name in string.gmatch(w, variablePattern) do
        if false == isInDefinition(name, 'variable') then
            table.insert(definition['variable'], name)
        end
    end
end

local function findInDefinition(str, part)
    local found = '$'

    for k, line in pairs(definition[part]) do
        if nil ~= string.find(line, '^' .. str) and line ~= str then
            found = found .. ' ' .. line
        end
    end

    return string.sub(found, 3, -1);
end

local function checkAutoComplete(char)
    local find = ''
    local curPos = editor.CurrentPos - 1
    local word = ''

    while curPos >= 0 do
        if(editor.CharAt[curPos] < 0) then
            break
        end
        
        curChar = string.char(editor.CharAt[curPos])
        
        if nil ~= string.match(curChar, "[_a-zA-Z0-9]") then
            find = curChar .. find
            curPos = curPos - 1
        else
            focus['start'] = curPos;
            focus['end'] = editor.CurrentPos;

            if '$' == curChar then
                word = findInDefinition(find, 'variable')
            elseif '@' == curChar then
                word = findInDefinition(find, 'doc')
            elseif '>' == curChar and '-' == editor.CharAt[curPos - 1] then
                word = findInDefinition(find, 'function')
            elseif curPos < editor.CurrentPos - 1 then
                word = findInDefinition(find, 'function')
            end

            if string.len(word) > 0 then
                editor:UserListShow(editor.CurrentPos, word)
            end
            break
        end
    end
end

local function formatEditorDoc(str, tab, line)
    scite.SendEditor(SCI_INSERTTEXT, editor.CurrentPos, str)
    
    local selStart = editor:PositionFromLine(line) + string.len(tab) + 3
    local selEnd = selStart + string.len("description...")
    
    scite.SendEditor(SCI_SETSELECTIONSTART, selStart)
    scite.SendEditor(SCI_SETSELECTIONEND, selEnd)
end

local function findDefinedFunctionParams(params)
    local struct = {}
    for class, var in string.gmatch(params, "([_a-zA-Z0-9]*)%s*\$([_a-zA-Z0-9]+)") do
        if string.len(class) > 0 then
            table.insert(struct, class .. " $" .. var)
        else
            table.insert(struct, "unknown $" .. var)
        end
    end
    
    return struct;
end

local function checkDoc(char)
    if "\n" ~= char and "\r" ~= char and " " ~= char then
        return
    end

	if " " == char and editor.CurrentPos >= 3 and '/**' == editor:textrange(editor.CurrentPos - 4, editor.CurrentPos - 1) then
		local str = "description... */"
		scite.SendEditor(SCI_INSERTTEXT, editor.CurrentPos, str)
		scite.SendEditor(SCI_SETSELECTIONSTART, editor.CurrentPos)
		scite.SendEditor(SCI_SETSELECTIONEND, editor.CurrentPos + string.len("description..."))
		return
	end

    local line = editor:LineFromPosition(editor.CurrentPos)
    local lineStart = editor:PositionFromLine(line - 1)
    local prevLine = editor:GetLine(line - 1)
    local lineEnd = lineStart
    
    if nil ~= prevLine then
        lineEnd = lineEnd + string.len(prevLine)
    end
    
    local nextLine = nil
    local eol = "\n"
    local eolLength = 1

    if 0 == editor.EOLMode then
        eol = "\r\n"
        eolLength = 2
    end
    
    if 1 == editor.EOLMode then
        eol = "\r"
    end
    
    if(lineEnd < lineStart + eolLength + 3) then
        return
    end

    local tab = ''
    local tabLen = 0
    for findTab in string.gmatch(prevLine, '(%s*)[^%s]*') do
        tab = findTab
        tabLen = string.len(findTab)
        break
    end

    if ("\n" == char or "\r" == char) and lineEnd >= 3 and '/**' == editor:textrange(lineEnd - 3 - eolLength, lineEnd - eolLength) then
        if editor.LineCount <= line then
            nextLine = nil
        else
            nextLine = editor:GetLine(line + 1)
        end

        local str = ''
        local trim = ''

        if editor.CurrentPos <= editor:PositionFromLine(line) then
            trim = tab
        end

        if nextLine ~= nil then
            for access1, access2, functionName in string.gmatch(nextLine, "(%w*)%s*(%w*)%s*function%s+([_a-zA-Z0-9]+)%s*\(.*\)") do
                if functionName ~= nil then
                    local iaccess = ''
                    if true == rawget(access, access2) then
                        iaccess = access2
                    else
                        iaccess = access1
                    end
                    
                    local struct = findDefinedFunctionParams(nextLine)
                    
                    str = trim .. " * description..." .. eol
                    str = str .. tab .. " * " .. eol
                    
                    if string.len(iaccess) > 0 then
                        str = str .. tab .. " * @access " .. iaccess .. eol
                    end
                    
                    for key, class in pairs(struct) do
                        str = str .. tab .. " * @param " .. class .. eol
                    end
                    
                    str = str .. tab .. " * @return unknown" .. eol
                    str = str .. tab .. " */"
                    
                    formatEditorDoc(str, tab, line)
                    return
                end
            end
        end

        if nextLine ~= nil then
            for className in string.gmatch(nextLine, "class%s+([_a-zA-Z0-9]+)") do
                if className ~= nil then
                    str = trim .. " * description..." .. eol
                    str = str .. tab .. " * " .. eol
                    str = str .. tab .. " * @author qining" .. eol
                    str = str .. tab .. " * @category typecho" .. eol
                    str = str .. tab .. " * @package " .. className .. eol
                    str = str .. tab .. " */"
                    
                    formatEditorDoc(str, tab, line)
                    return
                end
            end
        end

        if nextLine ~= nil then
            for access1, access2, variableName in string.gmatch(nextLine, "(%w*)%s*(%w*)%s+\$([_a-zA-Z0-9]+)") do
                if variableName ~= nil then
                    local iaccess = ''
                    if true == rawget(access, access2) then
                        iaccess = access2
                    else
                        iaccess = access1
                    end
                    
                    str = trim .. " * description..." .. eol
                    str = str .. tab .. " * " .. eol
                    str = str .. tab .. " * @access " .. iaccess .. eol
                    str = str .. tab .. " * @var unknown" .. eol
                    str = str .. tab .. " */"
                    
                    formatEditorDoc(str, tab, line)
                    return
                end
            end
        end
        

        if nextLine ~= nil then
            local realLine = string.gsub(nextLine, "%s*", "");
        else
            local realLine = '';
        end

		if realLine ~= nil then
			str = trim .. " * description..." .. eol
			str = str .. tab .. " * " .. eol
			str = str .. tab .. " */"
			formatEditorDoc(str, tab, line)
            return
		else
			str = trim .. " * description..." .. eol
			str = str .. tab .. " * " .. eol
			str = str .. tab .. " * @author qining" .. eol
			str = str .. tab .. " * @category typecho" .. eol
			str = str .. tab .. " * @package default" .. eol
			str = str .. tab .. " * @copyright Copyright (c) 2008 Typecho team (http://www.typecho.org)" .. eol
			str = str .. tab .. " * @license GNU General Public License 2.0" .. eol
			str = str .. tab .. " * @version $Id$" .. eol
			str = str .. tab .. " */"
			formatEditorDoc(str, tab, line)
            return
		end
    end

    if ("\n" == char or "\r" == char) and lineEnd >= 2 and '*' == editor:textrange(lineStart + tabLen, lineStart + tabLen + 1) and '/' ~= editor:textrange(lineStart + tabLen + 1, lineStart + tabLen + 2) then
        if 0 == editor.EOLMode then
            editor:InsertText(editor.CurrentPos, '* ')
            editor:GotoPos(editor.CurrentPos + 2)
        else
            editor:InsertText(editor.CurrentPos, tab .. '* ')
            editor:GotoPos(editor.CurrentPos + 2 + tabLen)
        end
    end
end

local old_OnOpen = OnOpen
function OnOpen(path)
    editor.MouseDwellTime = 500
    findDefinition()
end

local old_OnSwitchFile = OnSwitchFile
function OnSwitchFile(path)
    editor.MouseDwellTime = 500
    findDefinition()
end

local old_OnSave = OnSave
function OnSave(path)
    findDefinition()
end

local old_OnChar = OnChar
function OnChar(char)
    checkAutoComplete(char)
    checkDoc(char)
end
