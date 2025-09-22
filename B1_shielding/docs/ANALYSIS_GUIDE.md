# 中子伽马复合屏蔽玻璃综合分析系统使用指南

## 概述

本分析系统专为研究计划设计，支持建立中子和伽马射线综合输运模型，研究玻璃组成与屏蔽性能的数学规律，并实现±15%精度的实验验证。

## 系统架构

```
B1_shielding/
├── run_comprehensive_analysis.C          # 主控制脚本
├── comprehensive_shielding_analysis.C    # 基础屏蔽分析
├── mathematical_modeling_analysis.C      # 数学建模分析
├── experimental_validation_analysis.C    # 实验验证分析
├── ANALYSIS_GUIDE.md                     # 使用指南（本文件）
└── analysis_results/                     # 结果输出目录
    ├── basic_analysis/
    ├── mathematical_models/
    ├── experimental_validation/
    └── comprehensive_reports/
```

## 快速开始

### 1. 交互式运行（推荐）

```bash
cd B1_shielding
root -l
.L run_comprehensive_analysis.C
run_comprehensive_analysis()
```

系统将显示菜单，您可以选择：
- 1: 基础屏蔽分析
- 2: 数学建模分析  
- 3: 实验验证分析
- 4: 综合分析报告
- 5: 运行全部分析
- 6: 生成研究时间线
- 7: 导出结果数据

### 2. 批处理运行

```bash
root -l -q 'run_comprehensive_analysis.C()'
# 或者
root -l
.L run_comprehensive_analysis.C
run_all_analysis_batch()
```

## 分析模块详解

### 1. 基础屏蔽分析 (comprehensive_shielding_analysis.C)

**功能：**
- 能量-透射率关系分析
- 成分-效率关系矩阵
- 综合响应面分析
- 协同效应分析

**输出图表：**
- `energy_transmission_relations.png/pdf`
- `composition_efficiency_matrix.png/pdf`
- `comprehensive_response_surface.png/pdf`
- `experimental_validation.png/pdf`

**关键物理量：**
- 伽马射线透射率 vs 能量
- 中子透射率 vs 能量
- 综合屏蔽效率
- 元素贡献权重矩阵

### 2. 数学建模分析 (mathematical_modeling_analysis.C)

**功能：**
- 衰减定律数学建模
- 多元回归分析
- 多目标优化
- 模型验证

**输出图表：**
- `mathematical_attenuation_models.png/pdf`
- `composition_efficiency_models.png/pdf`
- `multi_objective_optimization.png/pdf`
- `model_validation_analysis.png/pdf`

**数学模型：**
- 伽马衰减系数：`μ(E) = a·E^(-b) + c·E^(-d)`
- 中子吸收截面：`σ(E) = σ₀/√E + σ_res + σ_fast`
- 综合效率模型：`η = f(组成, 能量, 厚度)`

### 3. 实验验证分析 (experimental_validation_analysis.C)

**功能：**
- 模拟vs实验对比
- 误差源分析
- 不确定性量化
- 实验设计优化

**输出图表：**
- `accuracy_validation_analysis.png/pdf`
- `error_sources_analysis.png/pdf`
- `uncertainty_quantification.png/pdf`
- `experimental_design_optimization.png/pdf`

**精度指标：**
- RMSE (均方根误差)
- MAE (平均绝对误差)
- R² (决定系数)
- 15%容差范围内数据点比例

## 研究时间线

### 第一阶段 (2026年9月-11月)
- **目标：** 建立综合衰减系数数据库
- **任务：** 开发多元回归分析工具
- **输出：** 基础数学模型

### 第二阶段 (2026年12月-2027年2月)
- **目标：** 构建综合输运模型
- **任务：** 开发优化算法
- **输出：** SCI论文初稿

### 第三阶段 (2027年3月-8月)
- **目标：** 实验验证（±15%精度）
- **任务：** 数据对比分析
- **输出：** 论文修改与发表

## 关键研究成果

### 1. 数学模型建立
- 伽马射线衰减系数模型
- 中子吸收截面模型
- 综合屏蔽效率预测模型

### 2. 优化分析
- 帕累托前沿分析
- 多目标优化算法
- 最优组成配比确定

### 3. 验证评估
- 模型精度验证（目标：±15%）
- 误差源识别与量化
- 不确定性传播分析

## 输出文件说明

### 图表文件
- PNG格式：用于演示和报告
- PDF格式：用于论文发表

### 数据文件
- `analysis_summary.txt`：分析结果汇总
- `research_timeline.png`：研究进度规划
- `summary_report.pdf`：综合分析报告

## 使用技巧

### 1. 数据准备
确保以下文件存在：
- `build/scintillator_output.root`
- `gamma_shielding.mac`
- `neutron_shielding.mac`

### 2. 结果解读
- 关注R²值（目标 > 0.90）
- 检查15%容差范围内数据点比例
- 分析帕累托前沿找最优解

### 3. 论文撰写支持
系统生成的图表可直接用于：
- SCI论文的Results部分
- 数学模型的可视化展示
- 验证精度的统计分析

## 故障排除

### 常见问题

**Q: 运行时提示找不到数据文件**
A: 检查`build/scintillator_output.root`是否存在，如不存在请先运行Geant4仿真

**Q: 图表显示异常**
A: 确保ROOT版本 ≥ 6.0，建议使用ROOT 6.24+

**Q: 内存不足**
A: 大型数据集分析时，建议分批处理或增加系统内存

**Q: 结果精度不满足要求**
A: 检查输入参数精度，考虑增加统计样本或改进物理模型

### 技术支持

如遇到技术问题，请检查：
1. ROOT环境配置
2. 数据文件完整性
3. 系统内存和磁盘空间
4. 权限设置

## 更新日志

### v1.0 (2024年)
- 初始版本发布
- 支持基础屏蔽分析
- 数学建模功能
- 实验验证模块
- 综合报告生成

## 贡献指南

欢迎对本系统进行改进和扩展：
1. 添加新的物理模型
2. 改进优化算法
3. 增强可视化效果
4. 完善文档说明

## 许可证

本项目遵循MIT许可证，详见LICENSE文件。

---

**联系方式：**
- 项目维护者：研究团队
- 技术支持：请通过GitHub Issues反馈问题
- 学术合作：欢迎引用相关论文成果
