# PJ1

本目录包含计算机图形学 PJ1 的说明文件与完成后的代码实现。

## 目录说明

- `Project1说明.pptx`：课程提供的 PJ1 说明 PPT
- `starter1/`：作业代码目录
- `starter1/swp/`：样例输入文件
- `starter1/build/a1`：编译后的可执行程序
- `starter1/run_pj1.sh`：在当前 WSL/Ubuntu 环境下推荐使用的启动脚本

## 已完成内容

已根据 PPT 要求完成以下任务：

- 分段三次 Bezier 曲线绘制
- 三次 B-spline 曲线绘制
- 曲线局部坐标系 `T / N / B` 计算
- 旋转曲面 `makeSurfRev`
- 广义圆柱面 `makeGenCyl`
- 闭合扫掠曲线的首尾法向插值修正
- WSL 下的构建与运行适配

主要实现文件：

- `starter1/src/curve.cpp`
- `starter1/src/surf.cpp`
- `starter1/src/main.cpp`
- `starter1/CMakeLists.txt`

## 构建方法

在项目根目录执行：

```bash
cd /home/lzx/my_workspace/CG/PJ1
cmake -S starter1 -B starter1/build
cmake --build starter1/build -j4
```

## 运行方法

推荐使用封装好的脚本：

```bash
cd /home/lzx/my_workspace/CG/PJ1
starter1/run_pj1.sh starter1/swp/core.swp
```

也可以直接运行可执行文件：

```bash
starter1/build/a1 starter1/swp/core.swp
```

若需要导出 OBJ：

```bash
starter1/run_pj1.sh starter1/swp/weirder.swp /tmp/pj1_out
```

会生成类似：

```bash
/tmp/pj1_out_weirder.obj
```

## 推荐检查样例

建议至少运行以下样例做验收：

```bash
starter1/run_pj1.sh starter1/swp/core.swp
starter1/run_pj1.sh starter1/swp/weird.swp
starter1/run_pj1.sh starter1/swp/weirder.swp
starter1/run_pj1.sh starter1/swp/wineglass.swp
```

含义如下：

- `core.swp`：检查曲线与局部坐标系
- `weird.swp`：检查广义圆柱
- `weirder.swp`：检查闭合修正
- `wineglass.swp`：检查旋转曲面

## 交互按键

- `c`：切换曲线局部坐标系显示
- `p`：切换控制点显示
- `s`：切换曲面显示模式
- 空格：重置视角

鼠标可用于旋转、缩放和平移视角。

## WSL 图形说明

当前环境为 WSL Ubuntu。若窗口无法打开，通常不是代码问题，而是 WSL 图形会话未接通。

可按如下方式处理：

1. 在 Windows PowerShell 中执行：

```powershell
wsl --shutdown
```

2. 重新打开 Ubuntu
3. 再次运行：

```bash
cd /home/lzx/my_workspace/CG/PJ1
starter1/run_pj1.sh starter1/swp/core.swp
```

`starter1/run_pj1.sh` 会优先按 WSLg 环境启动，并在图形环境不可用时给出提示。

## 当前完成状态

代码任务已完成，并已完成以下验证：

- 工程可成功编译
- 多个 `swp` 样例可正常导出 OBJ
- 导出的顶点和法线无 `NaN/Inf`
- `weirder.swp` 的闭合接缝已修正

## 提交前还需要做的事

- 逐个运行样例并截图
- 撰写 PJ1 报告
- 将 `starter1/` 与报告 PDF 一并打包提交
