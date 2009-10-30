--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, tonumber('0xFF6633', 16))
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, 0xFF6633)
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, 0xFF6633)
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, 0xFF6633)
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, 0xFF6633)
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, 0xFF6633)
--scite.SendEditor(SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, 0xFF6633)

--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, 0x0000FF)
--scite.SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0x0000FF)

--scite.SendEditor(SCI_STYLESETFORE, STYLE_LINENUMBER, 0xFFFF00)
--scite.SendEditor(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xFF00FF)

------------------------------------------------
-- 工具
------------------------------------------------

local snippets = {
    ['for'] = '3;for (%{1} = 0; %{1} < %{2}; %{1}++) {\n    %{3}\n}%{end}',
    ['class'] = '2;class %{1} {\n    function __construct() {\n         %{2}\n    }\n}%{end}',
    ['func'] = '3;function %{1}(%{2}) \n{\n    %{3}\n}%{end}',
    ['foreach'] ='3;foreach (%{1} as %{2}) \n{\n    %{3}\n}%{end}';
    ['if'] ='2;if (%{1}) \n{\n    %{2}\n}%{end}';
}
local in_snippet
local total_num
local current_num
local begin_pos
local function php_snippet_init()
    in_snippet  = false
    total_num   = 0
    current_num = 0
    begin_pos   = 0
end


------------------------------------------------
--  自动注释
------------------------------------------------

local definition = {['function'] = {};['variable'] = {};
['doc'] = {'access', 'author', 'copyright', 'license', 'package', 'param', 'return', 'throws', 'var', 'version'}}
focus = {['start'] = 0;['end'] = 0}
access = {['public'] = true;['private'] = true;['protected'] = true;['var'] = true}

function findDefinedFunctionParams(params)
    local struct = {}
    --在编辑器中寻找
    for class, var in string.gmatch(params, "(%w*)%s*\$(%w+)") do
        if string.len(class) > 0 then
            table.insert(struct, class .. " $" .. var)
        else
            table.insert(struct, "unknown $" .. var)
        end
    end
    
    return struct;
end

local function formatEditorDoc(str, tab, line)
    scite.SendEditor(SCI_INSERTTEXT, editor.CurrentPos, str)
   
    local selStart = editor:PositionFromLine(line) + string.len(tab) + 3
    local selEnd = selStart + string.len("description...")
    
    scite.SendEditor(SCI_SETSELECTIONSTART, selStart)
    scite.SendEditor(SCI_SETSELECTIONEND, selEnd)
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
    
    local author = ''
    if props['document.author'] ~= nil and props['document.author'] ~= '' then
        author = props['document.author']
    else
        author = os.getenv('USERNAME')
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
                    str = str .. tab .. " * @author ".. author .. eol
                    str = str .. tab .. " * @category Project" .. eol
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
                    
                    --var to be public access
                    if iaccess == string.lower('var') then
                        iaccess = 'public'
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
   			str = str .. tab .. " * @author ".. author .. eol
   			str = str .. tab .. " * @category Project" .. eol
   			str = str .. tab .. " * @package None" .. eol
   			str = str .. tab .. " * @copyright Copyright (c) ".. os.date('%Y') .. eol
   			str = str .. tab .. " * @version v1.0 (".. os.date('%m/%d/%Y') ..")" .. eol
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


------------------------------------------------
-- 事件
------------------------------------------------

function OnChar(c)
    local toClose = {
       --防止与智能提示冲突
       -- ['('] = ')',
        ['['] = ']',
        ['{'] = '}',
        ['"'] = '"',
        ["'"] = "'"
    }
    local toPass = {
        --[')'] = '(',
        [']'] = '[',
        ['}'] = '{'
    }
    -- 自动闭合
    if toClose[c] then
        editor:InsertText(editor.CurrentPos, toClose[c])
    end
    -- 智能越过
    if toPass[c] then
        if toPass[c] == string.char(editor.CharAt[editor.CurrentPos - 2]) then
            editor:GotoPos(editor.CurrentPos + 1)
            scite.SendEditor(SCI_DELETEBACK);
        end
    end
    -- 增加注释
    checkDoc(c)
    
end
function OnKey(k, shift, ctrl, alt)
    local toDelete = {
        ['('] = ')',
        ['['] = ']',
        ['{'] = '}',
        ['"'] = '"',
        ["'"] = "'"
    }
    -- 自动删除。8 是回退键
    if k == 8 then
        local pos = editor.CurrentPos - 1
        local chr = editor.CharAt[pos]
        if chr < 0 then
            return
        end
        chr = string.char(chr)
        local mth = string.char(editor.CharAt[pos + 1])
        if toDelete[chr] and toDelete[chr] == mth then
            editor:SetSel(pos, pos + 2)
            editor:ReplaceSel('')
            return true
        end
    -- 回车键
    elseif k == 13 then
        local left_char = editor.CharAt[editor.CurrentPos - 1]
        if left_char < 0 then
            return false
        end
        local left  = string.char(left_char)
        local right = string.char(editor.CharAt[editor.CurrentPos])
        if (left == '{' and right == '}') or
            (left == '(' and right == ')' or
            left == '[' and right == ']') then
            local line = editor:LineFromPosition(editor.CurrentPos)
            scite.SendEditor(SCI_NEWLINE)
            scite.SendEditor(SCI_NEWLINE)
            editor.LineIndentation[line + 2] = editor.LineIndentation[line]
            if left ~= '{' then
                editor.LineIndentation[line + 1] = editor.LineIndentation[line] + 4
            end
            editor:GotoPos(editor.LineEndPosition[line + 1])
            return true
        end
        return false
    -- Tab 键
    elseif k == 9 and in_snippet then
        php_snippet()
        return true
    end
end
function OnUserListSelection(listType, s)
    print(listType, s)
end

------------------------------------------------
-- 命令
------------------------------------------------

function php_snippet()
    if in_snippet then
        current_num = current_num + 1
        if current_num > total_num then
            local b, e = editor:findtext('%{end}', 0, begin_pos)
            if b ~= nil and e ~= nil then
                editor:SetSel(b, e)
                editor:ReplaceSel('')
            end
            php_snippet_init()
            return
        end
        editor:ClearSelections()
        local str = '%{' .. current_num .. '}'
        local begin = begin_pos
        local end_pos = editor:findtext('%{end}', 0, begin)
        local has_main = false
        -- 寻找所有 str 并选中它
        while true do
            local b, e = editor:findtext(str, 0, begin)
            if b ~= nil then
                if not has_main then
                    editor:SetSelection(b, e)
                    has_main = true
                else
                    editor:AddSelection(b, e)
                end
                begin = e
            else
                break
            end
        end
    else
        local word = props['CurrentWord']
        if snippets[word] == nil then return end
        --editor:DelWordLeft()
        editor:ReplaceSel('')
        total_num     = tonumber(string.sub(snippets[word], 1, 1))
        current_num   = 0
        begin_pos     = editor.CurrentPos
        local curline = scite.SendEditor(SCI_LINEFROMPOSITION, begin_pos)
        local indent  = scite.SendEditor(SCI_GETLINEINDENTATION, curline)
        local snippet = string.sub(snippets[word], 3)
        local lines   = 0
        for i = 1, #snippet do
            local s = string.sub(snippet, i, i)
            if s == '\n' then
                lines = lines + 1
            end
        end
        editor:AddText(snippet)
        for i = 1, lines do
            local line_indent = scite.SendEditor(SCI_GETLINEINDENTATION, curline + i)
            scite.SendEditor(SCI_SETLINEINDENTATION, curline + i, indent + line_indent)
        end 
        in_snippet = true
        php_snippet()
    end
end

function php_select_to_quote()
    -- ''  ""
    -- 'abc'  "abc"
    local right_pos = editor.CurrentPos
    local right_chr = string.char(editor.CharAt[right_pos])
    local left_pos  = right_pos - 1
    local left_chr  = string.char(editor.CharAt[left_pos])
    if left_chr == right_chr then
        return false
    end
    if right_chr == '"' or right_chr == "'" then
        editor:SearchAnchor()
        local pos = editor:SearchPrev(0, right_chr) + 1
        editor:SetSel(pos, right_pos)
    end
    if left_chr == '"' or left_chr == "'" then
        editor:SearchAnchor()
        local pos = editor:SearchNext(0, left_chr)
        editor:SetSel(left_pos + 1, pos)
    end
end

--[[
--
-- 从 PHP 打开 TPL
-- action : D:\diogin\code\php\xuniapp\xuniapp\pack\front\main\index.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\view\front\page\main\index.tpl
-- helper : D:\diogin\code\php\xuniapp\xuniapp\view\front\block\header.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\view\front\block\header.tpl
]]--
function php_open_tpl()
    local file_path = props['FilePath']
    local tpl_file  = ''
    if string.find(file_path, '\\pack\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.php$') ~= nil then
        tpl_file = string.gsub(file_path, '\.php$', '.tpl')
        tpl_file = string.gsub(tpl_file, '\\pack\\([a-z][0-9a-z_]*)\\', '\\view\\%1\\page\\')
    elseif string.find(file_path, '\\view\\[a-z][0-9a-z_]*\\block\\[a-z][0-9a-z_]*\.php$') ~= nil then
        tpl_file = string.gsub(file_path, '\.php$', '.tpl')
    else
        print('no related tpl file')
        return
    end
    scite.Open(tpl_file)
end

--[[
--
-- 从 PHP 打开 CSS
-- action : D:\diogin\code\php\xuniapp\xuniapp\pack\front\main\index.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\site\front\default\page\main\index\css.css
-- helper : D:\diogin\code\php\xuniapp\xuniapp\view\front\block\header.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\site\front\default\block\header\css.css
]]--
function php_open_css()
    local file_path = props['FilePath']
    local css_file  = ''
    if string.find(file_path, '\\pack\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.php$') ~= nil then
        css_file = string.gsub(file_path, '\.php$', '\\css\.css')
        css_file = string.gsub(css_file, '\\pack\\([a-z][0-9a-z_]*)\\', '\\site\\%1\\default\\page\\')
    elseif string.find(file_path, '\\view\\[a-z][0-9a-z_]*\\block\\[a-z][0-9a-z_]*\.php$') ~= nil then
        css_file = string.gsub(file_path, '\\view\\([a-z][0-9a-z_]*)\\block\\([a-z][0-9a-z_]*)\.php$', '\\site\\%1\\default\\block\\%2\\css.css')
    else
        print('no related css file')
        return
    end
    scite.Open(css_file)
end

--[[
--
-- 从 PHP 打开 JavaScript
-- action : D:\diogin\code\php\xuniapp\xuniapp\pack\front\main\index.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\site\front\default\page\main\index\js.js
-- helper : D:\diogin\code\php\xuniapp\xuniapp\view\front\block\header.php
--       -> D:\diogin\code\php\xuniapp\xuniapp\site\front\default\block\header\js.js
]]--
function php_open_js()
    local file_path = props['FilePath']
    local js_file  = ''
    if string.find(file_path, '\\pack\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.php$') ~= nil then
        js_file = string.gsub(file_path, '\.php$', '\\js\.js')
        js_file = string.gsub(js_file, '\\pack\\[a-z][0-9a-z_]*\\', '\\site\\%1\\default\\page\\')
    elseif string.find(file_path, '\\view\\[a-z][0-9a-z_]*\\block\\[a-z][0-9a-z_]*\.php$') ~= nil then
        js_file = string.gsub(file_path, '\\view\\([a-z][0-9a-z_]*)\\block\\([a-z][0-9a-z_]*)\.php$', '\\site\\%1\\default\\block\\%2\\js.js')
    else
        print('no related js file')
        return
    end
    scite.Open(js_file)
end


function View_In_Localhost()
    local file_path = props['FilePath']
    local localhost = props['localhost.path']
    local host_path = props['localhost.root']
    local this_path = ''
    if localhost == nil or host_path == nil then
        print("Please set web path in [hypertext.properties]")
        return
    elseif string.find(localhost,'http://') == nil then
        localhost = 'http://'..localhost
    end
    
    if string.find(string.lower(file_path),string.lower(host_path)) ~= nil then
        this_path = string.gsub(string.lower(file_path),string.lower(host_path),localhost)
    else
        this_path = file_path
        local copy_name = '\\'..props['FileName']..'.temp.'..props['FileExt']
        if shell.msgbox("Copy file '"..file_path .."' to '"..host_path..copy_name.."'?", "Copy file to web root", 4) == 6 then
            if os_copy(file_path,host_path..copy_name) then
                this_path = localhost..copy_name
            end
        end
    end
    this_path = string.gsub(this_path,"\\","/")
    if this_path ~= nil and  props['browser.path'] ~= nil then
        --local shellcmd = "\""..props['browser.path'].."\" "..this_path
        local rs,info=shell.exec(this_path) 
	  if(rs ~= true) then
            print(shellcmd,info)
        end 
    end
end
