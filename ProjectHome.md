**注：因各种原因本项目已停止更新，有兴趣的同学请从SVN里下载源代码自行修改，分享！(2013.3.30)**

# 项目说明 #

专为PHP重新编译和定制的SciTE编辑器，欢迎免费下载修改使用。如果您的修改使SciTE变得更加强大易用，衷心希望您能提交分享，同时欢迎大家加入\*SciTE/PHP交流群：16820754

**最新的PHP手册可以从这里下载：http://u.115.com/file/f7ab550b4d (带评论示例)**

用法：下载手册解压缩，并改名为 PHP5.manual.chm 复制到你的scite/docs目录下，以后在php文件里光标定位到关键字按F1即可出来相应的帮助信息.

**原版更新请点击这里：http://www.scintilla.org/ScintillaHistory.html**

**重新编译修改项：**
  1. 增强PHP代码高亮颜色;
  1. 增加项目管理文件树, 对项目管理更加方便;
  1. 增加二级菜单支持，工具栏、Tab右键菜单、编辑区右键菜单均支持二级菜单;
  1. 增强Snippets功能，编写代码效率可以变得更加高效;
  1. 修改状态栏,增加部件显示而不用去点击切换，状态栏增加实时显示文件编码;
  1. 内置UTF8编码自动判断而非通过lua实现，打开文件速度更快内存占用更低了;
  1. 新增OnMenuCommand事件，方便在执行菜单命令行调用lua代码;
  1. 新增OnStatusBarClick事件，单击状态栏相应part可以处理不同事件;
  1. 新增save.convert.indent属性，可以设置保存时是否统一(已设置的)缩进格式;
  1. 新增margin.click.bookmark属性，可以配置单击左边空白区域是否增加书签;
  1. 新增[sidebar](sidebar.md).dock属性，可以配置是否自动隐藏侧边栏。


---

**基础改进：**
  1. 右键菜单常用操作项;
  1. 文件/项目和函数管理的侧边栏;在侧边栏的文件管理中点右键可以新建文件，如果有选定的了文件，可以新建与之类型相同的文件;
  1. 增加颜色选择器,便于调整颜色;
  1. php和html页面可以在按F12键在浏览器中预览，如果php文件没有在web服务器根目录下，则会提示拷贝临时文件到根目录下，利用localhost浏览;
  1. 自动识别UTF8编码,打开页面不再出现乱码了;
  1. 右键增加简单的跳转到当前页class、function、const、variables定义的菜单项;
  1. 添加对SVN的支持，不过要求安装TortoiseSVN;
  1. 自动PHP Documentor注释功能;
  1. 增加html标签匹配lua插件;
  1. 增加Tab与空格互换lua插件;
  1. 增加高亮显示选定相同代码插件;
  1. 其他一些人性化的改进。

以上修改都是围绕本人在实际使用过程中遇到的问题进行的，在此发布的目的是使大家能和我一样，能够使用一款够爽的PHP编辑器，如果您觉得本款编辑器好用，别忘了推荐给你身边的同事和朋友们！


# 屏幕截图 #
  * 侧边栏与目录树
> ![http://scite-for-php.googlecode.com/files/scite_1.jpg](http://scite-for-php.googlecode.com/files/scite_1.jpg)
  * 工具栏菜单
> ![http://scite-for-php.googlecode.com/files/scite_2.jpg](http://scite-for-php.googlecode.com/files/scite_2.jpg)
  * 代码缩写栏与Snnipets
> ![http://scite-for-php.googlecode.com/files/scite_3.jpg](http://scite-for-php.googlecode.com/files/scite_3.jpg)
  * Tab子菜单
> ![http://scite-for-php.googlecode.com/files/scite_4.jpg](http://scite-for-php.googlecode.com/files/scite_4.jpg)