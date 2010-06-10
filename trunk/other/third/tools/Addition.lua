--scite_Command 'Edit Colour|edit_colour|Ctrl+Shift+C'
require 'iconv'
--------------------------
-- 编码转换
--------------------------
function charset_iconv(in_charset, out_charset, text)
  local cd = iconv.new(out_charset .. "//TRANSLIT", in_charset)
  local cd = iconv.open(in_charset, out_charset)
  assert(cd, "Failed to create a converter object.")
  local text_out, err = cd:iconv(text)

  if err == iconv.ERROR_INCOMPLETE then
    print("ICONV ERROR: Incomplete input.")
  elseif err == iconv.ERROR_INVALID then
    print("ICONV ERROR: Invalid input.")
  elseif err == iconv.ERROR_NO_MEMORY then
    print("ICONV ERROR: Failed to allocate memory.")
  elseif err == iconv.ERROR_UNKNOWN then
    print("ICONV ERROR: There was an unknown error.")
  end
  return text_out 
end
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
-- 检查UTF8编码
--------------------------
local function IsUTF8()
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

--------------------------
-- 全局调用检查编码函数
--------------------------
function OnCheckUTF()
    if props["utf8.check"] == "1" then
        if editor.CodePage ~= SC_CP_UTF8 then
            IsUTF8()
        end
	end    
end
--------------------------
-- 转换输出区码
--------------------------
function switch_encoding()
    scite.MenuCommand(IDM_SELECTALL)
    scite.MenuCommand(IDM_COPY)
    if editor.CodePage == SC_CP_UTF8 then
        scite.MenuCommand(IDM_ENCODING_DEFAULT)
    else
        scite.MenuCommand(IDM_ENCODING_UCOOKIE)
    end
    scite.MenuCommand(IDM_PASTE)
    update_status_bar()
end
--------------------------
-- 更新状态栏编码
--------------------------
function update_status_bar()
    if editor.CodePage == SC_CP_UTF8 then
        props['statusbar.codepage'] = '65001'
    else
        props['statusbar.codepage'] = '936'
    end
    scite.UpdateStatusBar()
end