# 中子伽马复合屏蔽玻璃Geant4模拟项目

## 项目概述

本项目基于Geant4官方B1示例重新开发，专门用于中子伽马复合屏蔽玻璃的辐射效应模拟。项目实现了DPA（Displacements Per Atom）计算、NIEL（Non-Ionizing Energy Loss）分析，以及多种电磁物理选项的动态切换。

## 主要特性

### 1. 物理列表
- **电磁物理**: G4EmStandardPhysics_option4, G4EmLivermorePhysics, G4EmLowEPPhysics
- **强相互作用物理**: G4ShieldingPhysics (专门用于屏蔽和辐射防护)
  - FTFP模型: 3 GeV - 100 TeV (高能区)
  - BERT模型: 1 GeV - 6 GeV (中能区)
  - BIC模型: 0 eV - 1.5 GeV (低能区)
  - 中子俘获、弹性散射、非弹性散射
- **离子物理**: G4IonElasticPhysics, G4IonPhysics, G4IonBinaryCascadePhysics
- **其他物理**: G4StoppingPhysics, G4RadioactiveDecayPhysics, G4NeutronTrackingCut

### 2. 辐射损伤分析
- **DPA计算**: 基于NRT模型和SRIM模型的原子位移损伤计算
- **NIEL分析**: 非电离能量损失分析
- **屏蔽效率**: 中子伽马复合屏蔽效率评估
- **材料性能**: 辐射损伤对材料性能的影响分析

### 3. 电磁物理选项

#### 3.1 选项对比
| 选项 | 物理列表 | 特点 | 能量范围 | 适用场景 |
|------|----------|------|----------|----------|
| 0 | G4EmStandardPhysics_option4 | 标准电磁物理优化版本 | 10eV-100TeV | 一般中子伽马屏蔽研究 |
| 1 | G4EmLivermorePhysics | 基于Livermore实验室数据 | 100eV-1GeV | 高精度中低能区域研究 |
| 2 | G4EmLowEPPhysics | 专门针对低能电磁过程优化 | 100eV-20MeV | **低能研究推荐** |

#### 3.2 选择建议
- **默认选项**: `G4EmStandardPhysics_option4` (选项0)
- **低能研究推荐**: `G4EmLowEPPhysics` (选项2) - 特别适合NIEL主要集中在0-0.2MeV的研究
- **中能研究**: `G4EmLivermorePhysics` (选项1)

#### 3.3 使用方法
```bash
# 使用默认选项 (Standard_option4)
./build/exampleB1 gamma_shielding.mac

# 使用低能电磁物理 (推荐用于低能研究)
EM_PHYSICS_OPTION=2 ./build/exampleB1 gamma_shielding.mac

# 使用Livermore物理
EM_PHYSICS_OPTION=1 ./build/exampleB1 gamma_shielding.mac
```

## 编译和安装

### 1. 环境要求
- Geant4 11.0+
- ROOT 6.0+
- CMake 3.16+
- C++17编译器

### 2. 编译步骤
```bash
# 创建构建目录
mkdir -p build
cd build

# 配置项目
cmake ..

# 编译
make -j8
```

### 3. 验证编译
```bash
# 检查可执行文件
ls -la exampleB1

# 测试运行
./exampleB1 ../gamma_shielding.mac
```

## 使用方法

### 1. 基本运行
```bash
# 使用默认电磁物理选项
./build/exampleB1 gamma_shielding.mac

# 使用低能电磁物理选项 (推荐研究低能电磁物理的时候使用)
EM_PHYSICS_OPTION=2 ./build/exampleB1 gamma_shielding.mac
```

### 2. 可用的宏文件
- `gamma_shielding.mac`: 伽马射线屏蔽测试
- `neutron_shielding.mac`: 中子屏蔽测试
- `mixed_shielding.mac`: 混合辐射屏蔽测试
- `test_cf252_detailed.mac`: Cf-252中子源详细测试
- `test_cf252_simple.mac`: Cf-252中子源简化测试

### 3. 环境变量控制
- `EM_PHYSICS_OPTION`: 控制电磁物理选项 (0/1/2)
- `PHYSLIST`: 控制整体物理列表 (已弃用，使用CustomPhysicsList)

### 4. 输出文件
- `build/scintillator_output.root`: ROOT格式的分析数据
- `build/data/bjl.txt`: 文本格式的数据输出

## 数据分析和报告生成

### 1. 自动报告生成
```bash
# 生成完整的报告材料
./generate_report.sh
```

### 2. 报告内容
- **几何结构图**: 屏蔽玻璃的三视图和材料成分
- **数据分析图**: 能量沉积、DPA、NIEL、屏蔽效率分布
- **统计信息**: 事件数、屏蔽效率等关键指标

### 3. 输出文件
- `report_images/geometry_plot.png/pdf`: 几何结构图
- `report_images/report_analysis.png/pdf`: 数据分析图
- `report_images/report_summary.txt`: 统计摘要

### 4. 手动分析
```bash
# 使用ROOT分析数据
root -l -q report_analysis.C

# 生成几何图
root -l -q geometry_plot.C
```

## 技术细节

### 1. DPA计算模型
- **NRT模型**: Norgett-Robinson-Torrens模型，标准DPA计算
- **SRIM模型**: Stopping and Range of Ions in Matter，高精度DPA计算
- **位移阈值**: 不同元素的原子位移阈值

### 2. 材料定义
- **屏蔽玻璃**: SiO2(45%) + Al2O3(15%) + CeO2(10%) + B2O3(8%) + Gd2O3(8%) + ZnO(6%) + Li2O(4%) + PbO(2%)
- **密度**: 3.2 g/cm³
- **尺寸**: 20cm × 20cm × 7.5cm

### 3. 几何结构
- **世界体积**: 50cm × 50cm × 50cm
- **屏蔽玻璃**: 位于世界中心
- **粒子源**: 表面源，圆形，半径2cm

## 故障排除

### 1. 编译问题
```bash
# 清理构建目录
rm -rf build
mkdir build
cd build
cmake ..
make -j8
```

### 2. 运行时错误
- **找不到宏文件**: 确保在项目根目录运行
- **ROOT文件错误**: 检查build目录权限
- **物理列表冲突**: 使用环境变量EM_PHYSICS_OPTION控制

### 3. 数据分析问题
- **图像生成失败**: 检查ROOT安装和字体支持
- **数据文件损坏**: 重新运行模拟
- **坐标轴标签不完整**: 调整画布大小和边距

## 项目结构

```
B1_shielding/
├── include/                 # 头文件
│   ├── ActionInitialization.hh
│   ├── CustomPhysicsList.hh  # 自定义物理列表
│   ├── DetectorConstruction.hh
│   ├── DPAModelConfig.hh     # DPA模型配置
│   ├── EventAction.hh
│   ├── PrimaryGeneratorAction.hh
│   ├── RunAction.hh
│   └── SteppingAction.hh
├── src/                     # 源文件
│   ├── ActionInitialization.cc
│   ├── CustomPhysicsList.cc  # 自定义物理列表实现
│   ├── DetectorConstruction.cc
│   ├── EventAction.cc
│   ├── PrimaryGeneratorAction.cc
│   ├── RunAction.cc
│   └── SteppingAction.cc
├── build/                   # 构建目录
├── report_images/           # 报告图像输出
├── *.mac                   # Geant4宏文件
├── *.C                     # ROOT分析脚本
├── *.sh                    # 自动化脚本
├── CMakeLists.txt          # CMake配置
├── exampleB1.cc            # 主程序
└── README.md               # 本文档，操作手册
```

## 参考文献

### 1. Geant4相关
- Agostinelli, S., et al. "GEANT4—a simulation toolkit." Nuclear instruments and methods in physics research section A: Accelerators, Spectrometers, Detectors and Associated Equipment 506.3 (2003): 250-303.
- Allison, J., et al. "Recent developments in Geant4." Nuclear instruments and methods in physics research section A: Accelerators, Spectrometers, Detectors and Associated Equipment 835 (2016): 186-225.

### 2. DPA和辐射损伤
- Norgett, M. J., M. T. Robinson, and I. M. Torrens. "A proposed method of calculating displacement dose rates." Nuclear Engineering and Design 33.1 (1975): 50-54.
- Ziegler, J. F., M. D. Ziegler, and J. P. Biersack. "SRIM–The stopping and range of ions in matter." Nuclear instruments and methods in physics research section B: Beam interactions with materials and atoms 268.11-12 (2010): 1818-1823.

### 3. 屏蔽材料研究
- 相关的中子伽马复合屏蔽玻璃研究文献
- 辐射损伤对材料性能影响的实验和理论研究

## 许可证

本项目基于Geant4软件许可证，请参考LICENSE文件。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 项目维护者：[您的姓名]
- 邮箱：[您的邮箱]
- 项目地址：[项目地址]
