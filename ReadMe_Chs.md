# 哈希校验工具 Hash Checksum Tool

利用一点闲暇时间做出一个哈希校验工具，主要实现了对文件或字符串计算MD5、SHA1和CRC32校验值的功能，并可以和验证值进行比较达到校验文件完整性的效果，对于文件，还可以查看文件的字节数和修改时间，有助于进一步验证文件是否被修改。
本工具基于WTL框架开发，编译了32位和64位两个版本，支持Windows 7的Aero界面效果（提供切换系统Aero效果的命令），支持文件拖放，也可以添加Windows资源管理器右键菜单命令方便使用。绿色软件，可以直接使用，如果添加了右键菜单命令，建议在删除本工具的时候去掉该选项的勾选。

## 注意事项：
Aero仅在Vista或更高版本系统上可用，且系统已开启该功能，在XP下只能显示基本的Windows窗口界面。如果不喜欢全界面Aero效果，可以在EXE所在目录下的配置文件Config.ini中新增一行：NoAero=1，或者通过命令行选项-noaero启动。
如果选择的文件是快捷方式，程序会提示是否转到其目标文件进行计算，可以在Config.ini中新增一行配置LnkOption，如果LnkOption=0则每次都提示，LnkOption=1则自动解析目标文件，LnkOption=2则不进行解析（Vista以上版本系统下可以在弹出询问对话框时选择不再提示，将自动保存该次的选择）。
非常小的工具，无需做更多介绍，目前还不是很完善，待有时间再做改进。另外，哈希值也有一定的重复概率（尽管很小），如果再提供更多的验证算法将会更好，这可能作为这个小工具以后的更新目标。

## 运行环境：
Windows XP 或更高版本操作系统

## 截图：
![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/001.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/002.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/003.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/004.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/005.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/006.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/007.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/008.png)

![image](https://raw.githubusercontent.com/singun-lxd/CalcHash/master/ScreenShots/009.png)
