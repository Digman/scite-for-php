-- COMMON.lua
-- Version: 1.4.2
---------------------------------------------------
-- ����� �������, �������������� �� ������ ��������
---------------------------------------------------

-- ���� ������ ������������ lua-���������
package.cpath = props["SciteDefaultHome"].."\\other\\third\\tools\\LuaLib\\?.dll;"..package.cpath

--------------------------------------------------------
-- ������ �-��� string.lower() � string.upper()
-- �������� � ������ ������������� �����������
-- ������� ������� ��������� ��������� ��������� chars.accented = ����...
if props["chars.accented"] ~= "" then
	local old_lower = string.lower
	function string.lower(s)
		local locale = props["chars.accented"]
		if not s:find('['..locale..']') then return old_lower(s) end -- ��� ���. �������� => ���������� ������ ��������
		local res = "" -- ����� ����� �������� ���������
		local ch -- ������
		local pos -- ������� � ������
		for i = 1, #s do
			ch = s:sub(i,i)
			pos = locale:find(ch,1,true)
			if pos then
				if pos%2==1 then res = res..locale:sub(pos+1,pos+1)
				else res = res..ch
				end
			else --if not in locale
				res = res..old_lower(ch)
			end
		end
		return res
	end

	local old_upper = string.upper
	function string.upper(s)
		local locale = props["chars.accented"]
		if not s:find('['..locale..']') then return old_upper(s) end -- ��� ���. �������� => ���������� ������ ��������
		local res = "" -- ����� ����� �������� ���������
		local ch -- ������
		local pos -- ������� � ������
		for i = 1, #s do
			ch = s:sub(i,i)
			pos = locale:find(ch,1,true)
			if pos then
				if pos%2==0 then res = res..locale:sub(pos-1,pos-1)
				else res = res..ch
				end
			else --if not in locale
				res = res..old_upper(ch)
			end
		end
		return res
	end
	
end -- IFDEF chars.accented

--------------------------------------------------------
-- ��������� �������� �� nil � ���� ��� ��� �� ���������� default ����� ���������� ��� ��������
function ifnil(val, default)
  if val == nil then
    return default
  else
    return val
  end
end

--------------------------------------------------------
-- ����������� ������������� �� ����� ������� ����� �����������
function IsComment(pos)
	local style = editor.StyleAt[pos]
	local lexer = editor.LexerLanguage
	local comment = {
		abap = {1, 2},
		ada = {10},
		asm = {1, 11},
		au3 = {1, 2},
		baan = {1, 2},
		bullant = {1, 2, 3},
		caml = {12, 13, 14, 15},
		cpp = {1, 2, 3, 15, 17, 18},
		csound = {1, 9},
		css = {9},
		d = {1, 2, 3, 4, 15, 16, 17},
		escript = {1, 2, 3},
		flagship = {1, 2, 3, 4, 5, 6},
		forth = {1, 2, 3},
		gap = {9},
		hypertext = {9, 20, 29, 42, 43, 44, 57, 58, 59, 72, 82, 92, 107, 124, 125},
		xml = {9, 29},
		inno = {1, 7},
		latex = {4},
		lua = {1, 2, 3},
		mmixal = {1, 17},
		nsis = {1, 18},
		opal = {1, 2},
		pascal = {1, 2, 3},
		perl = {2},
		bash = {2},
		pov = {1, 2},
		ps = {1, 2, 3},
		python = {1, 12},
		rebol = {1, 2},
		ruby = {2},
		scriptol = {2, 3, 4, 5},
		smalltalk = {3},
		specman = {2, 3},
		spice = {8},
		sql = {1, 2, 3, 13, 15, 17, 18},
		tcl = {1, 2, 20, 21},
		verilog = {1, 2, 3},
		vhdl = {1, 2}
	}

	-- ��� ��������, ������������� � �������:
	for l,ts in pairs(comment) do
		if l == lexer then
			for _,s in pairs(ts) do
				if s == style then
					return true
				end
			end
			return false
		end
	end
	-- ��� ��������� ��������:
	-- asn1, ave, blitzbasic, cmake, conf, eiffel, eiffelkw, erlang, euphoria, fortran, f77, freebasic, kix, lisp, lout, octave, matlab, metapost, nncrontab, props, batch, makefile, diff, purebasic, vb, yaml
	if style == 1 then return true end
	return false
end


------[[ T E X T   M A R K S ]]-------------------------

-- ��������� ������ �������� ������������� �����
function EditorMarkText(start, length, style_number)
	scite.SendEditor(SCI_SETINDICATORCURRENT, style_number)
	scite.SendEditor(SCI_INDICATORFILLRANGE, start, length)
end

-- ������� ������ �� ���������� ��������� ��������� �����
--   ���� ��������� ���������� - ��������� ��� ����� �� ���� ������
--   ���� �� ������� ������� � ����� - ��������� ���� �����
function EditorClearMarks(style_number, start, length)
	local _first_style, _end_style, style
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
end

----------------------------------------------------------------------------
-- ������� ����� ��� �������� (����� ��� ������� ����� ����� ������������ � ��������, ������� �� �� ������)

-- Translate color from RGB to win
local function encodeRGB2WIN(color)
	if string.sub(color,1,1)=="#" and string.len(color)>6 then
		return tonumber(string.sub(color,6,7)..string.sub(color,4,5)..string.sub(color,2,3), 16)
	else
		return color
	end
end

local function InitMarkStyle(style_number, indic_style, color, alpha_fill)
	editor.IndicStyle[style_number] = indic_style
	editor.IndicFore[style_number] = encodeRGB2WIN(color)
	editor.IndicAlpha[style_number] = alpha_fill
end

local function style(mark_string)
	local mark_style_table = {
	plain    = INDIC_PLAIN,    squiggle = INDIC_SQUIGGLE,
	tt       = INDIC_TT,       diagonal = INDIC_DIAGONAL,
	strike   = INDIC_STRIKE,   hidden   = INDIC_HIDDEN,
	roundbox = INDIC_ROUNDBOX, box      = INDIC_BOX
	}
	for st,st_num in pairs(mark_style_table) do
		if string.match(mark_string, st) ~= nil then
			return st_num
		end
	end
end

local function EditorInitMarkStyles()
	for style_number = 0, 31 do
		local mark = props["find.mark."..style_number]
		if mark ~= "" then
			local mark_color = mark:match("#%x%x%x%x%x%x") or (props["find.mark"]):match("#%x%x%x%x%x%x") or "#0F0F0F"
			local mark_style = style(mark) or INDIC_ROUNDBOX
			local alpha_fill = tonumber((mark:match("%@%d+") or ""):sub(2)) or 30
			InitMarkStyle(style_number, mark_style, mark_color, alpha_fill)
		end
	end
end

-- Add user event handler OnOpen
local old_OnOpen = OnOpen
local OnOpenOne = true
function OnOpen(file)
	local result
	if old_OnOpen then result = old_OnOpen(file) end
	if OnOpenOne then
		EditorInitMarkStyles()
		OnOpenOne = false
	end
	return result
end

----------------------------------------------------------------------------
-- �������������� ��������� ��������� ��������� (������������ ��� ������/��������� "�����" � ����)
function CheckChange(prop_name)
	if tonumber(props[prop_name]) == 1 then
		props[prop_name] = 0
	else
		props[prop_name] = 1
	end
end

-- ==============================================================
-- ������� ����������� os_copy(source_path,dest_path)
-- ����� z00n <http://www.lua.ru/forum/posts/list/15/89.page>
--// "������������" �������
local function unwind_protect(thunk,cleanup)
	local ok,res = pcall(thunk)
	if cleanup then cleanup() end
	if not ok then error(res,0) else return res end
end

--// ����� ������� ��� ������ � ��������� �������
local function with_open_file(name,mode)
	return function(body)
	local f = assert(io.open(name,mode))
	return unwind_protect(function()return body(f) end,
		function()return f and f:close() end)
	end
end

--// ���������� os-copy --
function os_copy(source_path,dest_path)
	return with_open_file(source_path,"rb") (function(source)
		return with_open_file(dest_path,"wb") (function(dest)
			assert(dest:write(assert(source:read("*a"))))
			return true
		end)
	end)
end
-- ==============================================================
