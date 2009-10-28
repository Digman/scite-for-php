--[[
page   : D:\diogin\code\php\xunisns\xunisns\view\page\main\index.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\default\page\main\index\css.css
block  : D:\diogin\code\php\xunisns\xunisns\view\block\header.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\default\block\header\css.css
layout : D:\diogin\code\php\xunisns\xunisns\view\layout\main.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\default\layout\main\css.css
sheet  : D:\diogin\code\php\xunisns\xunisns\view\sheet\pager.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\default\sheet\pager\css.css
]]--
function tpl_open_css()
    local file_path = props['FilePath']
    local css_file  = ''
    if string.find(file_path, '\\view\\page\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        css_file = string.gsub(file_path, '\\view\\page\\([a-z][0-9a-z_]*)\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\page\\%1\\%2\\css.css')
    elseif string.find(file_path, '\\view\\block\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        css_file = string.gsub(file_path, '\\view\\block\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\block\\%1\\css.css')
    elseif string.find(file_path, '\\view\\layout\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        css_file = string.gsub(file_path, '\\view\\layout\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\layout\\%1\\css.css')
    elseif string.find(file_path, '\\view\\sheet\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        css_file = string.gsub(file_path, '\\view\\sheet\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\sheet\\%1\\css.css')
    else
        print('no related css file')
        return
    end
    scite.Open(css_file)
end

--[[
page   : D:\diogin\code\php\xunisns\xunisns\view\page\main\index.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\page\main\index\js.js
block  : D:\diogin\code\php\xunisns\xunisns\view\block\header.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\block\header\js.js
layout : D:\diogin\code\php\xunisns\xunisns\view\layout\main.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\layout\main\js.js
sheet  : D:\diogin\code\php\xunisns\xunisns\view\sheet\pager.tpl
      -> D:\diogin\code\php\xunisns\xunisns\site\sheet\pager\js.js
]]--
function tpl_open_js()
    local file_path = props['FilePath']
    local js_file  = ''
    if string.find(file_path, '\\view\\page\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        js_file = string.gsub(file_path, '\\view\\page\\([a-z][0-9a-z_]*)\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\page\\%1\\%2\\js.js')
    elseif string.find(file_path, '\\view\\block\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        js_file = string.gsub(file_path, '\\view\\block\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\block\\%1\\js.js')
    elseif string.find(file_path, '\\view\\layout\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        js_file = string.gsub(file_path, '\\view\\layout\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\layout\\%1\\js.js')
    elseif string.find(file_path, '\\view\\sheet\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        js_file = string.gsub(file_path, '\\view\\sheet\\([a-z][0-9a-z_]*)\.tpl$', '\\site\\default\\sheet\\%1\\js.js')
    else
        print('no related js file')
        return
    end
    scite.Open(js_file)
end

--[[
page   : D:\diogin\code\php\xunisns\xunisns\view\page\main\index.tpl
      -> D:\diogin\code\php\xunisns\xunisns\pack\main\index.php
block  : D:\diogin\code\php\xunisns\xunisns\view\block\header.tpl
      -> D:\diogin\code\php\xunisns\xunisns\view\block\header.php
]]--
function tpl_open_php()
    local file_path = props['FilePath']
    local php_file  = ''
    if string.find(file_path, '\\view\\page\\[a-z][0-9a-z_]*\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        php_file = string.gsub(file_path, '\\view\\page\\([a-z][0-9a-z_]*)\\([a-z][0-9a-z_]*)\.tpl$', '\\pack\\%1\\%2.php')
    elseif string.find(file_path, '\\view\\block\\[a-z][0-9a-z_]*\.tpl$') ~= nil then
        php_file = string.gsub(file_path, '\\view\\block\\([a-z][0-9a-z_]*)\.tpl$', '\\view\\block\\%1.php')
    else
        print('no related php file')
        return
    end
    scite.Open(php_file)
end
