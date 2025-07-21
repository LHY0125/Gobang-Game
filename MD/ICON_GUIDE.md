# 为五子棋游戏添加图标指南

## 问题说明

在尝试为可执行文件添加图标时，发现icon文件夹中的图标文件格式不正确：
- `gobang_icon1.ico` 和 `gobang_icon2.ico` 实际上是HTML文件和PNG文件，而不是真正的ICO格式文件

## 解决方案

### 方法一：获取正确的ICO文件

1. **下载或创建真正的ICO文件**
   - 使用在线ICO转换工具将PNG/JPG转换为ICO格式
   - 推荐网站：https://www.icoconverter.com/
   - 或使用GIMP、Photoshop等图像编辑软件导出ICO格式

2. **替换现有文件**
   - 将正确的ICO文件保存为 `icon/gobang_icon.ico`
   - 确保文件是真正的ICO格式（文件头应为 00 00 01 00）

3. **修改资源文件**
   - 编辑 `gobang.rc` 文件
   - 取消注释图标行：`IDI_APPLICATION ICON "icon\\gobang_icon.ico"`

4. **重新编译**
   ```bash
   windres gobang.rc -o gobang.res
   gcc -std=c17 -o gobang.exe *.c gobang.res -lws2_32
   ```

### 方法二：使用现有PNG文件（需要转换）

如果你有PNG格式的图标文件，可以：

1. **在线转换**
   - 访问 https://convertio.co/png-ico/
   - 上传PNG文件并转换为ICO格式
   - 下载转换后的ICO文件

2. **使用ImageMagick（如果已安装）**
   ```bash
   magick convert icon/your_image.png icon/gobang_icon.ico
   ```

### 方法三：使用Windows资源编辑器

1. 编译不带图标的exe文件（当前状态）
2. 使用Resource Hacker等工具后期添加图标
3. 下载Resource Hacker：http://www.angusj.com/resourcehacker/

## 当前状态

- ✅ 程序可以正常编译和运行
- ✅ 包含版本信息资源
- ❌ 暂时没有应用程序图标
- ✅ 提供了完整的构建脚本

## 编译指令

### 不带图标版本（当前可用）
```bash
gcc -std=c17 -o gobang.exe *.c -lws2_32
```

### 带图标版本（需要正确的ICO文件）
```bash
windres gobang.rc -o gobang.res
gcc -std=c17 -o gobang.exe *.c gobang.res -lws2_32
```

## 验证ICO文件格式

可以使用以下PowerShell命令检查文件是否为真正的ICO格式：
```powershell
Get-Content icon/gobang_icon.ico -AsByteStream -TotalCount 4 | ForEach-Object { '{0:X2}' -f $_ }
```

正确的ICO文件应该显示：`00 00 01 00`