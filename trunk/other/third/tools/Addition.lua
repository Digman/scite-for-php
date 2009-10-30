--scite_Command 'Edit Colour|edit_colour|Ctrl+Shift+C'

--------------------------
-- 编辑颜色
--------------------------
function edit_colour ()
    local function getch (i)
	    return editor:textrange(i,i+1)
	end
	local function hexdigit (c)
	    return c:find('[0-9A-F]')==1
	end
	local i = editor.CurrentPos
	-- let's find the extents of this colour field...
	local ch = getch(i)
	while i > 0 and ch ~= '#' and hexdigit(ch) do
		i = i - 1
		ch = getch(i)
	end
	if i == 0 then return end
	local istart = i
	--i = i + 1 -- skip the '#'
	while hexdigit(getch(i)) do i = i + 1 end
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
