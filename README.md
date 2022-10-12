# 适用范围

如果你正在进行人工智能相关的项目，需要从大量视频集中截取特定帧数、特定画面的剪辑视频用于训练，那么 Mini Video Processor 值得一试！



# 使用帮助

**请下载 release 下的软件安装包。源代码已开源，可按需进行代码审查。**

1. 完成安装后，请将 `mvp.exe` 所在的文件夹路径添加至系统环境变量path中。

2. 在待处理视频文件所在的文件夹路径下，使用 cmd 输入 `mvp`。

   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012145511233.png" alt="image-20221012145511233" style="zoom:50%;" />

3. 若第一次处理该路径下的视频集，程序将在当前目录下生成 `mvp_config.json` 配置文件。请根据需求更改好配置文件重新启动 mvp。

   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012145733285.png" alt="image-20221012145733285" style="zoom:50%;" />

4. 再次启动 mvp 后，输入起始的视频序号即可开始处理视频。

   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012150056934.png" alt="image-20221012150056934" style="zoom:50%;" />

   

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012151141667.png" alt="image-20221012151141667" style="zoom:50%;" />

Mini Video Processor 所有的输出信息都会在 cmd 中显示。



# 程序命令行启动参数

Mini Video Processor 自带文件重命名功能，使用 `mvp -r` 将将默认按数字递增方式重命名视频文件。

同时，支持自定义视频名前缀，前缀作为参数输入即可，例如：`mvp -r cats`，结果如下：

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012152718523.png" alt="image-20221012152718523" style="zoom:50%;" />

命名完成后按任意键即可继续正常处理程序。

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012152837688.png" alt="image-20221012152837688" style="zoom:50%;" />



# 程序快捷键

1. 在视频播放界面

   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012151637378.png" alt="image-20221012151637378" style="zoom:50%;" />

   - 使用空格键来切换 播放/暂停 状态。
   - 在 播放/暂停 状态下都可以通过 左右方向键 调整播放进度。
   - 按 `Q` 退出 VideoProcessor。
   - 按 `J` 跳过当前视频，以处理下一个视频。
   - 在暂停状态下，按 `Enter` 进入视频编辑界面。

3. 在视频编辑界面

   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012151704336.png" alt="image-20221012151704336" style="zoom:50%;" />
   
   - 进入视频编辑界面后，窗口四周会出现蓝色剪辑框。
     使用鼠标重新选择剪辑框位置，只截取有用部分导入至新视频中。
   
   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012151734064.png" alt="image-20221012151734064" style="zoom:50%;" />
   
   - 使用空格键来切换 播放/暂停 状态。
   - 如果视频截取段的起始帧不合适，使用 `ESC` 退出视频编辑界面，回到视频播放界面重新选择。
   - 如果发现视频不合要求，按 `J` 跳过当前视频，以处理下一个视频。
   - 使用 上下方向键 来调整间隔抽帧方式。
   - 使用 `Enter` 确定剪辑，此时 控制台 会显示视频类别。**注意：无需更换程序焦点至 cmd，直接在 Video Player 界面按对应分类序号的数字键即可。**若发现视频存在瑕疵需要重新框选，按 `q` 取消生成视频。
   
   <img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012151835902.png" alt="image-20221012151835902" style="zoom:50%;" />
   - 考虑到一个视频可能有多个有效的子视频，在完成子视频处理后，程序会继续播放当前视频，请在视频完成处理完毕后使用 `j` 键处理下一个视频。
   - 子视频会储存在以类别名命名的文件夹中，命名格式为序号+类别名。
   

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012152406034.png" alt="image-20221012152406034" style="zoom:50%;" />



# 配置文件语法

配置文件内容如下：

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012150225567.png" alt="image-20221012150225567" style="zoom:70%;" />

文件格式为 `json`，参数含义：

- **FPS:** 视频播放器的刷新率。
- **Frames skipped per fast-forward:** 视频播放器调整进度条时跨越的帧数。
- **new video frames:** 新生成的视频总帧数。
- **Categories:** 视频属于的分类。上限为 9 种。



# video_list.json 文件说明

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012152946557.png" alt="image-20221012152946557" style="zoom:50%;" />

该文件用于储存工作目录下的视频集，当工作目录下的视频集新增或删除视频后，下一次启动 `Mini Video Processor` 后程序会更新列表、更新视频处理进度，防止重复处理某一视频或者漏处理视频。

**注意：请不要手动修改此文件。**

<img src="D:\Github\Repositories\Mini-Video-Processor\assets\image-20221012153342294.png" alt="image-20221012153342294" style="zoom:50%;" />

在输入起始视频序号时，mvp 会提示上次处理进度，请根据此参考调整此次起始处理位置。



# 常见问题

- 如果遇到 `can not open file...` 的提示，请使用管理员权限打开 cmd 运行程序。
- 如果程序生成文件失败，请使用管理员权限打开 cmd 运行程序。



# 免责声明

1. 本仓库发布的 `Mini-Video-Processor` (下文均用本项目代替) 项目中涉及的任何内容，仅用于测试和学习研究。
2. 本项目内所有资源文件，禁止任何公众号、自媒体进行任何形式的转载、发布，禁止直接改项目名二次发布。
3. 作者对任何由于程序使用不当产生的问题概不负责，包括但不限于由任何程序错误导致的任何损失或损害.
4. 以任何方式查看此项目的人或直接或间接使用本项目的使用者都应仔细阅读此声明。作者保留随时更改或补充此免责声明的权利。一旦使用并复制了本项目，则视为您已接受此免责声明。
5. 本项目遵循Apache License 2.0协议，如果本特别声明与Apache License 2.0协议有冲突之处，以本特别声明为准。



# 反馈

有关程序的问题，可通过 `iuuses` 反馈。





