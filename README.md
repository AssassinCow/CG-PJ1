# 计算机图形学 Project 1

本仓库为计算机图形学课程 Project 1 的实现与结果整理。项目基于课程提供的 `starter1` 框架完成，主要涵盖参数曲线生成、曲线局部坐标系计算，以及旋转曲面与扫掠曲面的构造。

## 项目内容

本项目完成了以下核心功能：

- 分段三次 Bezier 曲线的生成与绘制
- 三次 B-spline 曲线的生成与绘制
- 曲线局部坐标系 `T / N / B` 的计算与可视化
- 旋转曲面 `makeSurfRev` 的构造
- 广义圆柱面 `makeGenCyl` 的构造

## 开发环境

项目构建与运行依赖以下环境：

- `CMake`
- 支持 OpenGL 的 C++ 编译环境
- `Linux`、`WSL` 或 `macOS`

## 构建说明

在仓库根目录执行以下命令完成编译：

```bash
cmake -S starter1 -B starter1/build
cmake --build starter1/build -j4
```

编译完成后，将在 `starter1/build/` 目录下生成可执行文件 `a1`。

## 运行说明

推荐通过封装脚本运行程序：

```bash
starter1/run_pj1.sh starter1/swp/core.swp
```

也可直接执行编译产物：

```bash
starter1/build/a1 starter1/swp/core.swp
```

如需导出 OBJ 模型，可执行：

```bash
starter1/run_pj1.sh starter1/swp/weirder.swp /tmp/pj1_out
```

## 样例输入与结果截图

`starter1/swp/` 目录中包含课程提供的测试样例，`results/` 目录中保存了对应的运行结果截图。为便于检索与归档，结果图片均已按照样例名称统一命名。

| 样例文件 | 内容说明 | 结果截图 |
| --- | --- | --- |
| `circles.swp` | 二维圆曲线样例 | `results/result-circles.png` |
| `core.swp` | 曲线与局部坐标系基础样例 | `results/result-core.png` |
| `flircle.swp` | 广义圆柱面样例 | `results/result-flircle.png` |
| `florus.swp` | 花形扫掠曲面样例 | `results/result-florus.png` |
| `gentorus.swp` | 环形扫掠曲面样例 | `results/result-gentorus.png` |
| `norm.swp` | 旋转曲面法向效果样例 | `results/result-norm.png` |
| `tor.swp` | 基础环面扫掠样例 | `results/result-tor.png` |
| `weird.swp` | 一般扫掠曲面样例 | `results/result-weird.png` |
| `weirder.swp` | 闭合扫掠曲面样例 | `results/result-weirder.png` |
| `wineglass.swp` | 酒杯旋转曲面样例 | `results/result-wineglass.png` |

## 目录说明

- `Project1说明.pptx`：课程作业说明文档
- `starter1/src/`：项目核心源代码
- `starter1/swp/`：输入样例文件
- `results/`：运行结果截图
- `starter1/run_pj1.sh`：项目运行脚本
- `starter1/build/`：编译输出目录

## 主要实现文件

- `starter1/src/curve.cpp`
- `starter1/src/surf.cpp`

## 交互方式

程序运行后的主要交互按键如下：

- `c`：切换曲线局部坐标系显示
- `p`：切换控制点显示
- `s`：切换曲面显示模式
- `Space`：重置视角
