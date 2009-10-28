--[[
page_css   : D:\xuni\code\php\xuni\skel\site\page\main\index\css.css
          -> D:\xuni\code\php\xuni\skel\pack\main\page\index.tpl
block_css  : D:\xuni\code\php\xuni\skel\site\block\header\css.css
          -> D:\xuni\code\php\xuni\skel\view\block\header.tpl
layout_css : D:\xuni\code\php\xuni\skel\site\layout\main\css.css
          -> D:\xuni\code\php\xuni\skel\view\layout\main.tpl
sheet_css  : D:\xuni\code\php\xuni\skel\site\sheet\pager\css.css
          -> D:\xuni\code\php\xuni\skel\view\sheet\pager.tpl
]]--
function css_open_tpl()
    local file_path = props['FilePath']
    local tpl_file  = ''
    if string.find(file_path, '\\page\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        tpl_file = string.gsub(file_path, '\\site\\page\\([a-z][0-9a-z_]*)\\([a-z][0-9a-z_]*)\\css\.css$', '\\pack\\%1\\page\\%2.tpl')
    elseif string.find(file_path, '\\block\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        tpl_file = string.gsub(file_path, '\\site\\block\\([a-z][0-9a-z_]*)\\css\.css$', '\\view\\block\\%1.tpl')
    elseif string.find(file_path, '\\layout\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        tpl_file = string.gsub(file_path, '\\site\\layout\\([a-z][0-9a-z_]*)\\css\.css$', '\\view\\layout\\%1.tpl')
    elseif string.find(file_path, '\\sheet\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        tpl_file = string.gsub(file_path, '\\site\\sheet\\([a-z][0-9a-z_]*)\\css\.css$', '\\view\\sheet\\%1.tpl')
    else
        print('no related tpl file')
        return
    end
    scite.Open(tpl_file)
end

--[[
page_css   : D:\xuni\code\php\xuni\skel\site\page\main\index\css.css
          -> D:\xuni\code\php\xuni\skel\pack\main\pack.php
block_css  : D:\xuni\code\php\xuni\skel\site\block\header\css.css
          -> D:\xuni\code\php\xuni\skel\view\block\header.php
]]--
function css_open_php()
    local file_path = props['FilePath']
    local php_file  = ''
    if string.find(file_path, '\\page\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        php_file = string.gsub(file_path, '\\site\\page\\([a-z][0-9a-z_]*)\\([a-z][0-9a-z_]*)\\css\.css$', '\\pack\\%1\\pack.php')
    elseif string.find(file_path, '\\block\\[a-z][0-9a-z_]*\\css\.css$') ~= nil then
        php_file = string.gsub(file_path, '\\site\\block\\([a-z][0-9a-z_]*)\\css\.css$', '\\view\\block\\%1.php')
    else
        print('no related php file')
        return
    end
    scite.Open(php_file)
end

function css_open_js()
end
