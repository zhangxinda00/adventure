# adventrue terminal (C++14)

一个受“冒险岛”启发的终端横版小游戏（MVP）：
- 单人跑跳
- 怪物巡逻/追击
- 碰撞与重力
- 近战攻击
- 胜负判定

## 依赖
- Linux
- CMake >= 3.10
- 支持 C++14 的编译器（g++ / clang++）

## 构建
```bash
cmake -S . -B build
cmake --build build -j
```

## 运行
```bash
./build/adventrue
```

## 键位
- `A` / `D`: 左右移动
- `W` 或 `↑`: 跳跃
- `J`: 攻击
- `R`: 在胜利/失败界面重开
- `Q`: 退出

## 注意
- 运行时会切换终端到 raw 模式，并使用 ANSI 控制屏幕。
- 程序正常退出时会自动恢复终端状态。
- 建议终端窗口至少 80x24。
