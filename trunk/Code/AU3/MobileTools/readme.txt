thesnow's Mobile Tools 2.0.0.1
=================================

配置文件说明:
配置文件名称等于可执行文件.ini
比如可执行文件名称为 thesnow.exe ,配置文件,名称就应该为thesnow.exe.ini
下方带[说明:]的行,不要用于配置文件中,这里只是做配置解释.
[Run]
VStart=%DRV%\OftenUse\VStart\VStart.exe
说明:左方为便于认识的名称,右方为路径

[ExecExt]
.e@e=支持.e@e为可执行文件
说明:将.e@e设置成可执行文件,右方数据无用.(可以设置无数个可执行文件的扩展名,请不要修改使用常见扩展名)

[LocalizedResourceName]
@thesnow,-65409=工作区域
说明:desktop.ini中不少字符串要求是调用DLL中的,可以用这个来配置.[@DLL名称,-字符串编号=字符串]
参考desktop.ini的写法.

[SystemSet]
%DRV%=Z:
说明:修改可移动设备的盘符
ShowHideFileExt=1
说明:资源管理器中显示隐藏的文件关联,比如 .exe 文件默认是不显示扩展名的.
MainPage=http://www.google.com/ncr
设置主页
OpenFileUseNotepad=1
说明:文件使用记事本打开(右键)
CommandLineHere=1
说明:文件夹,驱动器使用命令行打开(右键)
WallPaper=%DRV%\FreeSoft\System\Beautify\精美壁纸\系统品牌桌面\9.jpg
说明:设置桌面壁纸
DisktopIcon=0
说明:设置桌面图标显示方式,默认为0,大图标为1,列表为2,小图标为3,Tile为4
[ToolSet]
Favorite=1
说明:恢复可移动设备上主程序目录下的收藏夹备份.(要备份收藏夹,请用托盘菜单中的工具)
[RemoveDriver]
IMAGE=前面是卷标,哈.
说明:左方为要移除驱动器盘符的卷标,右方无用

[ImageFileExecutionOptions]
AllFile=-
taskmgr.exe=-
说明:镜像文件劫持,左方ALLFILE代表所有的劫持项目,左方为要劫持的文件名;右方为劫持到的路径,"-"表示删除这个项目

[traymenu]
1=我的工具
2=你的工具
3=谁的工具
9=微软工具
说明:本段用于向托盘菜单中添加菜单.左方为编号,无用,便于识别顺序.右方为要显示的字符串,空字符串代表分隔符

[trayitem]
1=纸牌;sol.exe
2=命令行;cmd.exe
3=记事本;notepad.exe
4=autoruns;%RAR%%DRV%\OftenUse\SysinternalsSuite.zip;autoruns.exe
4=Process Explorer;%RAR%%DRV%\OftenUse\SysinternalsSuite.zip;procexp.exe
4=Process Monitor;%RAR%%DRV%\OftenUse\SysinternalsSuite.zip;Procmon.exe
说明:本段用于向托盘菜单中添加菜单项目.
说明:左方对应[traymenu]下的每一个项目(按顺序),比如"9=微软工具",实际上是4(第4行).右方为"显示的名称;路径"
说明:右方的路径支持压缩文件中的路径,但只能是压缩文件中第一级路径中的文件.
说明:%FULLRAR%代表解压缩整个压缩包,%RAR%代表只解压缩需要的可执行文件.

路径表示中可使用的变量如下:
%DRV%=修改的盘符,这个是唯一需要定义的.不定义默认为Z:(上面定义)
%WINDIR%=windows目录
%SYSTEM32%=windows\system32目录

====================================================
版本更新:
2.0.0.1
[+]增加UnlockHomePage=1选项,解锁主页
[!]将收藏夹功能从[SystemSet]改到[ToolSet].并修正收藏夹功能可能失效的问题
[+]新增uTorrent BT工具
2.0.0.2
[!]修复使用OpenFileUseNotepad=0和CommandLineHere=0时没有相关功能的问题.