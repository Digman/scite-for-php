------------------------------------------
--     PACKAGE Path                     --
------------------------------------------
package.cpath = props["SciteDefaultHome"].."\\additions\\Lib\\?.dll;" .. package.cpath
require 'lpeg'
require 'gui'
require 'shell'

------------------------------------------
--      SciTE Event Functions          --
------------------------------------------
function AddEventHandler(EventName, Handler)
	local prime_Event = _G[EventName]
	if prime_Event ~= nil then
		_G[EventName] = function (...)
			return prime_Event(...) or Handler(...)
		end
	else
		_G[EventName] = function (...)
			return Handler(...)
		end
	end
end

------------------------------------------
--      SciTE Event Functions          --
------------------------------------------
function OnOpen(filename)
    buildNames() --build autocomplete names
	update_status_bar() -- update status bar
end 
  
function OnClose(filename)
end
  
function  OnSwitchFile(filename)
    buildNames() -- build autocomplete names
	update_status_bar() -- update status bar
end

function OnSave(filename)
    buildNames()
end

function OnBeforeSave(filename)
end

function OnChar(c)
    handleChar() --autocomplete handler char
end

function OnKey(key, shift, ctrl, alt)
end
  
function OnSavePointReached()
end 

function OnSavePointLeft()
end

function OnDwellStart()
end
  
function OnDoubleClick()
end

function OnMarginClick()
	scite.MenuCommand(IDM_BOOKMARK_TOGGLE)
end

function OnUserListSelection(listType, text)
end

-- added by Kimha
function OnMenuCommand(msg, source)
	repeat
		--case:
		--Copy methods
		if msg == IDM_COPYFILEDIR then
			editor:CopyText(props['FileDir'])
			break
		end
		if msg == IDM_COPYFILENAMEEXT then
			editor:CopyText(props['FileNameExt'])
			break
		end
		if msg == IDM_COPYFILENAME then
			editor:CopyText(props['FileName'])
			break
		end
		if msg == IDM_COPYFILEEXT then
			editor:CopyText(props['FileExt'])
			break
		end
		--SVN methods
		if msg == IDM_SVN_UPDATE then
			svn_exec('update',props['FilePath'])
			break
		end
		if msg == IDM_SVN_COMMIT then
			svn_exec('commit',props['FilePath'])
			break
		end
		if msg == IDM_SVN_ADD then
			svn_exec('add',props['FilePath'])
			break
		end
		if msg == IDM_SVN_DIFF then
			svn_exec('diff',props['FilePath'])
			break
		end
		if msg == IDM_SVN_REVERT then
			svn_exec('revert',props['FilePath'])
			break
		end
		if msg == IDM_SVN_SHOWLOG then
			svn_exec('log',props['FilePath'])
			break
		end
		--default:
		break
	until true
end

------------------------------------------
-- Custom Common Functions           --
------------------------------------------
--------------------------
-- 编辑颜色
--------------------------
function edit_colour ()
    local function get_prevch (i)
        return editor:textrange(i-1,i)
	end
    local function get_nextch (i)
	    return editor:textrange(i,i+1)
	end
	local function hexdigit(c)
	    return c:find('[0-9a-fA-F]')==1
	end
	local i = editor.CurrentPos
	-- let's find the extents of this colour field...
	local ch = get_prevch(i)
    -- 先向前查找颜色编码
	while i > 0 and ch ~= '#' and hexdigit(get_prevch(i)) do
		i = i - 1
		--ch = get_prevch(i)
	end
 	if i == 0 then return end
    local istart = i
    -- skip the '#'
    if ch == '#' then
        istart = istart - 1 
    end
    if get_nextch(i) == '#' then
        i = i+1
    end
    --反向查找颜色编码
 	while hexdigit(get_nextch(i)) do 
        i = i + 1 
    end
	-- extract the colour!
 	local colour = editor:textrange(istart,i)
 	colour = gui.colour_dlg(colour)
 	if colour then -- replace the colour in the document
 		editor:SetSel(istart,i)
		editor:ReplaceSel(colour)
 	end
end

--------------------------
-- 检查UTF8编码,暂无应用
--------------------------
function DetectUTF8()
	local text = editor:GetText()
	local cont = lpeg.R("\128\191")   -- continuation byte
	local utf8 = lpeg.R("\0\127")^1
			+ (lpeg.R("\194\223") * cont)^1
			+ (lpeg.R("\224\239") * cont * cont)^1
			+ (lpeg.R("\240\244") * cont * cont * cont)^1
	local latin = lpeg.R("\0\127")^1
	local searchpatt = latin^0 * utf8 ^1 * -1
	if searchpatt:match(text) then
		scite.MenuCommand(IDM_ENCODING_UCOOKIE)
        update_status_bar()
	end
end

----------------------------------
-- 全局调用检查编码函数，暂无应用
----------------------------------
function CheckUTF()
    if props["utf8.check"] == "1" then
        if editor.CodePage ~= SC_CP_UTF8 then
            DetectUTF8()
        end
	end    
end
--------------------------
-- 转换输出区码
--------------------------
function switch_encoding()
	--editor:BeginUndoAction()
    editor:SelectAll()
    editor:Copy()
    if editor.CodePage == SC_CP_UTF8 then
        scite.MenuCommand(IDM_ENCODING_DEFAULT)
    else
        scite.MenuCommand(IDM_ENCODING_UCOOKIE)
    end
	editor:Paste()
	--editor:EndUndoAction()
    update_status_bar()
end
--------------------------
-- 更新状态栏编码
--------------------------
function update_status_bar()
    if editor.CodePage == SC_CP_UTF8 then
        props['statusbar.encoding'] = 'UTF-8'
    else
        props['statusbar.encoding'] = 'GB2312'
    end
    scite.UpdateStatusBar()
end
---------------------------
-- 滚屏并保持光标原屏幕位置
---------------------------
function screen_up()
    editor:LineScrollUp()
    editor:LineUp()
end

function screen_down()
    editor:LineScrollDown()
    editor:LineDown()
end
-------------------------
-- 使用Shell执行SVN命令
-------------------------
function svn_exec(cmd,path)
	local svnexec = props['ext.subversion.path']
	if cmd == nil then cmd = 'update' end
	if path == nil or path == '' then path = props['FileDir'] end
	local command = "\""..svnexec.."\" /command:"..cmd.." /path:\""..path.."\" /notempfile /closeonend:0"
	shell.exec(command)
end
------------------------------------------
-- 为hypertext lexer的文档添加html注释
------------------------------------------
function add_html_comment()
	local old_comment_start = props['comment.stream.start.hypertext']
	local old_comment_end   = props['comment.stream.end.hypertext']
	props['comment.stream.start.hypertext'] = '<!--'
	props['comment.stream.end.hypertext']   = '-->'
	scite.MenuCommand(IDM_STREAM_COMMENT)
	props['comment.stream.start.hypertext'] = old_comment_start
	props['comment.stream.end.hypertext']   = old_comment_end
end
------------------------------------------
-- EditorMarkText
------------------------------------------
function EditorMarkText(start, length, style_number)
	local current_mark_number = scite.SendEditor(SCI_GETINDICATORCURRENT)
	scite.SendEditor(SCI_SETINDICATORCURRENT, style_number)
	scite.SendEditor(SCI_INDICATORFILLRANGE, start, length)
	scite.SendEditor(SCI_SETINDICATORCURRENT, current_mark_number)
end
------------------------------------------
-- EditorClearMarks
------------------------------------------
function EditorClearMarks(style_number, start, length)
	local _first_style, _end_style, style
	local current_mark_number = scite.SendEditor(SCI_GETINDICATORCURRENT)
	if style_number == nil then
		_first_style, _end_style = 0, 31
	else
		_first_style, _end_style = style_number, style_number
	end
	if start == nil then
		start, length = 0, editor.Length
	end
	for style = _first_style, _end_style do
		scite.SendEditor(SCI_SETINDICATORCURRENT, style)
		scite.SendEditor(SCI_INDICATORCLEARRANGE, start, length)
	end
	scite.SendEditor(SCI_SETINDICATORCURRENT, current_mark_number)
end
