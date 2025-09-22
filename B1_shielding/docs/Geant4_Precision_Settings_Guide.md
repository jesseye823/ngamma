# Geant4 精度设置完全指南

## 概述

本指南详细介绍了影响 Geant4 模拟精度的关键设置，按重要性分类并提供实用的代码示例。这些设置直接影响模拟结果与真实物理情况的一致性。

## 目录

1. [能量相关精度设置](#1-能量相关精度设置)
2. [多重散射精度设置](#2-多重散射精度设置)
3. [物理过程选择与精度](#3-物理过程选择与精度)
4. [几何与材料精度](#4-几何与材料精度)
5. [随机数与统计精度](#5-随机数与统计精度)
6. [边界与传输精度](#6-边界与传输精度)
7. [输出与记录精度](#7-输出与记录精度)
8. [关键的"隐藏"设置](#8-关键的隐藏设置)
9. [自动步长设置实现](#9-自动步长设置实现)
10. [实用建议](#10-实用建议)

---

## 1. 能量相关精度设置

### Production Cuts（生产阈值）

Production cuts 控制次级粒子的产生阈值，直接影响能量沉积精度。

```cpp
// 在 PhysicsList::SetCuts() 中设置
void CustomPhysicsList::SetCuts()
{
    // 设置全局截断值
    SetCutValue(10*um, "gamma");     // γ射线range cut
    SetCutValue(10*um, "e-");        // 电子range cut  
    SetCutValue(10*um, "e+");        // 正电子range cut
    SetCutValue(1*um, "proton");     // 质子cut
    
    // 为特定区域设置不同的cut
    G4Region* targetRegion = G4RegionStore::GetInstance()->GetRegion("TargetRegion");
    G4ProductionCuts* cuts = new G4ProductionCuts();
    cuts->SetProductionCut(1*um, "gamma");
    cuts->SetProductionCut(1*um, "e-");
    cuts->SetProductionCut(1*um, "e+");
    targetRegion->SetProductionCuts(cuts);
    
    if (verboseLevel > 0) DumpCutValuesTable();
}
```

### 能量阈值与表格精度

```cpp
// 在 PhysicsList 构造函数中设置
CustomPhysicsList::CustomPhysicsList()
{
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 能量范围设置
    em->SetMinEnergy(100*eV);        // 最低跟踪能量
    em->SetMaxEnergy(100*TeV);       // 最高能量
    
    // 表格精度设置
    em->SetNumberOfBinsPerDecade(20); // 能量表格精度（默认7，越高越精确）
    em->SetSpline(true);             // 启用样条插值提高精度
    
    // 能量损失计算精度
    em->SetDEDXBinning(84);          // dE/dx表格bins数
    em->SetLambdaBinning(84);        // 截面表格bins数
}
```

### 能量损失涨落模型

```cpp
// 控制能量损失的统计涨落
G4EmParameters* em = G4EmParameters::Instance();

em->SetLossFluctuations(true);    // 启用能量损失涨落（Landau/Vavilov）
em->SetBuildCSDARange(true);      // 构建CSDA射程表
em->SetLinearLossLimit(0.01);     // 线性损失限制（影响薄层近似）
em->SetUseCutAsFinalRange(false); // 不使用cut作为最终射程
em->SetApplyCuts(true);           // 应用production cuts
```

---

## 2. 多重散射精度设置

### 散射模型选择

```cpp
// 在 PhysicsList 构造函数中设置多重散射参数
G4EmParameters* em = G4EmParameters::Instance();

// 散射算法控制
em->SetMscStepLimitType(fUseDistanceToBoundary); // 步长限制算法
em->SetMscLateralDisplacement(true);  // 启用横向位移
em->SetMscMuHadLateralDisplacement(true); // μ子/强子横向位移

// 散射精度参数
em->SetMscRangeFactor(0.04);          // 射程因子（影响步长）
em->SetMscGeomFactor(2.5);            // 几何因子
em->SetMscSafetyFactor(0.6);          // 安全因子
em->SetMscLambdaLimit(1*mm);          // Lambda限制
```

### 散射角阈值

```cpp
// 单散射与多重散射的分界设置
em->SetMscThetaLimit(0.1*rad);        // 单散射/多散射分界角
em->SetScreeningFactor(1.0);          // 屏蔽因子
em->SetFactorForAngleLimit(1.0);      // 角度限制因子

// 不同粒子的散射设置
em->SetMscEnergyLimit(100*MeV);       // 多重散射能量上限
```

---

## 3. 物理过程选择与精度

### 电磁物理模型精度

```cpp
// 在 PhysicsList 构造函数中选择合适的物理模型

// 标准精度（快速）
RegisterPhysics(new G4EmStandardPhysics());

// 高精度选项
RegisterPhysics(new G4EmStandardPhysics_option3()); // 高精度
RegisterPhysics(new G4EmStandardPhysics_option4()); // 最高精度

// 低能精确模型
RegisterPhysics(new G4EmLivermorePhysics());        // Livermore模型
RegisterPhysics(new G4EmPenelopePhysics());         // Penelope模型
RegisterPhysics(new G4EmLowEPPhysics());            // 低能物理

// DNA物理（极低能）
RegisterPhysics(new G4EmDNAPhysics());
```

### 强子物理精度

```cpp
// 强子物理模型选择
RegisterPhysics(new G4HadronPhysicsFTFP_BERT());    // 标准
RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP()); // 高精度（含热中子）
RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());  // 最高精度
RegisterPhysics(new G4HadronPhysicsINCLXX());       // INCL++模型

// 中子物理
RegisterPhysics(new G4NeutronTrackingCut());        // 中子跟踪截断
RegisterPhysics(new G4HadronElasticPhysicsHP());    // 高精度弹性散射
```

---

## 4. 几何与材料精度

### 材料定义精度

```cpp
// 在 DetectorConstruction 中精确定义材料
G4Material* DefineShieldingGlass()
{
    G4NistManager* nist = G4NistManager::Instance();
    
    // 创建材料
    G4double density = 2.5*g/cm3;
    G4Material* glass = new G4Material("ShieldingGlass", density, nComponents);
    
    // 设置材料的电离参数（影响阻止本领）
    glass->GetIonisation()->SetMeanExcitationEnergy(68*eV); // 平均激发能
    glass->GetIonisation()->SetBirksConstant(0.1*mm/MeV);   // Birks饱和常数
    glass->GetIonisation()->SetFermiEnergy(200*eV);         // 费米能级
    
    // 设置材料密度效应参数
    G4IonisParamMat* ionisation = glass->GetIonisation();
    ionisation->SetDensityEffectParameters(2.0, 3.0, 0.1, 2.8, 0.14);
    
    return glass;
}
```

### 几何精度设置

```cpp
// 几何管理器设置
G4GeometryManager* geoManager = G4GeometryManager::GetInstance();
geoManager->SetWorldMaximumExtent(200*cm);
geoManager->OpenGeometry();  // 重建几何

// 体积重叠检查
G4bool checkOverlaps = true;
new G4PVPlacement(nullptr, G4ThreeVector(), logicVolume, 
                  "PhysicalVolume", motherVolume, false, 0, checkOverlaps);
```

---

## 5. 随机数与统计精度

### 随机数引擎

```cpp
// 在 main() 或 ActionInitialization 中设置
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/MTwistEngine.h"

// 设置高质量随机数引擎
CLHEP::HepRandom::setTheEngine(new CLHEP::MTwistEngine); // Mersenne Twister
// 或者
CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine); // RANECU

// 设置随机数种子
CLHEP::HepRandom::setTheSeed(12345);

// 保存随机数状态（用于重现）
CLHEP::HepRandom::saveEngineStatus("random.state");
```

### 统计权重

```cpp
// 在 SteppingAction 或其他地方使用权重
void SteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    
    // 获取当前权重
    G4double weight = track->GetWeight();
    
    // 设置新权重（用于方差缩减）
    G4double newWeight = weight * biasFunction();
    track->SetWeight(newWeight);
}
```

---

## 6. 边界与传输精度

### 边界精度

```cpp
// 在 DetectorConstruction 或 PhysicsList 中设置
G4TransportationManager* transportMgr = G4TransportationManager::GetTransportationManager();
G4Navigator* navigator = transportMgr->GetNavigatorForTracking();

navigator->SetPushVerbosity(1);        // 边界推进详细程度
navigator->CheckMode(true);            // 启用边界检查
navigator->SetWorldVolume(physWorld);  // 设置世界体积
```

### 磁场精度

```cpp
// 磁场管理器设置（如果使用磁场）
G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()
    ->GetFieldManager();

// 设置积分精度
fieldMgr->SetDeltaChord(0.1*mm);        // 弦长精度
fieldMgr->SetDeltaOneStep(0.01*mm);     // 单步精度
fieldMgr->SetMinimumEpsilonStep(1.e-5); // 最小相对步长
fieldMgr->SetMaximumEpsilonStep(1.e-3); // 最大相对步长

// 设置积分器
G4ClassicalRK4* stepper = new G4ClassicalRK4(equation);
G4ChordFinder* chordFinder = new G4ChordFinder(magneticField, 1.0*mm, stepper);
fieldMgr->SetChordFinder(chordFinder);
```

---

## 7. 输出与记录精度

### 记录精度设置

```cpp
// 在 RunAction 中设置分析管理器
void RunAction::BeginOfRunAction(const G4Run*)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    
    // 基本设置
    man->SetActivation(true);
    man->SetVerboseLevel(1);
    man->SetFirstHistoId(1);
    
    // 设置输出文件
    man->OpenFile("simulation_results.root");
    
    // 创建直方图时设置精度
    man->CreateH1("energy_deposition", "Energy Deposition", 1000, 0, 10*MeV);
    man->CreateH2("position_energy", "Position vs Energy", 
                  100, -5*cm, 5*cm,    // x轴
                  100, 0, 10*MeV);     // y轴
}
```

---

## 8. 关键的"隐藏"设置

### 内存与缓存

```cpp
// 在 PhysicsList 中设置表格管理
G4ProductionCutsTable* cutsTable = G4ProductionCutsTable::GetProductionCutsTable();
cutsTable->SetEnergyRange(100*eV, 100*GeV);
cutsTable->SetVerboseLevel(1);

G4LossTableManager* lossManager = G4LossTableManager::Instance();
lossManager->SetVerbose(1);
lossManager->SetBuildCSDARange(true);
lossManager->SetLPMFlag(true);          // LPM效应
lossManager->SetSplineFlag(true);       // 样条插值
```

### 过程激活控制

```cpp
// 精确控制哪些过程参与模拟
void CustomPhysicsList::ConstructProcess()
{
    // 调用基类方法
    G4VModularPhysicsList::ConstructProcess();
    
    // 获取过程表
    G4ProcessTable* processTable = G4ProcessTable::GetProcessTable();
    
    // 激活/关闭特定过程
    processTable->SetProcessActivation("msc", true);      // 多重散射
    processTable->SetProcessActivation("hIoni", true);    // 强子电离
    processTable->SetProcessActivation("eBrem", true);    // 电子轫致辐射
    
    // 设置过程排序（影响物理准确性）
    G4ProcessManager* pmanager = G4Proton::Proton()->GetProcessManager();
    pmanager->SetProcessOrdering(ionizationProcess, idxAlongStep, 1);
    pmanager->SetProcessOrdering(multipleScatteringProcess, idxAlongStep, 2);
}
```

---

## 9. 自动步长设置实现

### 通用自动步长公式

基于几何厚度自动计算最优步长，确保物理精度与计算效率的平衡：

```cpp
// 在 PhysicsList 构造函数中实现自动步长
CustomPhysicsList::CustomPhysicsList()
{
    // 获取目标厚度（从几何或配置获取）
    G4double targetThickness = 75.*mm;  // 例如：75mm厚屏蔽玻璃
    
    // 自动计算finalRange：clamp(thickness/1000, 5μm, 50μm)
    G4double finalRange = std::max(5.*um, std::min(targetThickness/1000., 50.*um));
    
    // 设置EM参数
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 自适应步长设置
    em->SetStepFunctionMuHad(0.03, finalRange);      // 质子/介子/强子
    em->SetStepFunctionLightIons(0.03, finalRange);  // 轻离子
    em->SetStepFunctionIons(0.03, finalRange);       // 重离子
    em->SetStepFunction(0.2, 0.1*mm);                // e±保持默认
    
    // 表格精度
    em->SetNumberOfBinsPerDecade(20);
    em->SetMinEnergy(100*eV);
    
    G4cout << "Auto step settings: dRoverRange=0.03, finalRange=" 
           << finalRange/um << " μm for thickness=" << targetThickness/mm << " mm" << G4endl;
}
```

### 深度谱解析度设置

```cpp
// 在 DetectorConstruction 中为特定体积设置UserLimits
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // ... 几何构建代码 ...
    
    // 为需要高深度分辨率的体积设置UserLimits
    if (enableDepthResolution) {
        G4double binWidth = 100.*um;    // 深度直方图bin宽度
        G4double maxStep = 0.5 * binWidth;
        
        logicTargetVolume->SetUserLimits(new G4UserLimits(maxStep));
        
        G4cout << "Depth resolution mode: maxStep=" << maxStep/um 
               << " μm for binWidth=" << binWidth/um << " μm" << G4endl;
    }
    
    return physWorld;
}
```

### 物理一致性验证

```cpp
// 验证设置是否满足物理一致性要求
void ValidatePhysicsSettings()
{
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 检查相对能量损失约束：每步ΔE/E ≤ 1-2%
    G4double dRoverRange = 0.03;  // 3%相对步长
    if (dRoverRange > 0.05) {
        G4cout << "Warning: dRoverRange=" << dRoverRange 
               << " may be too large for accurate energy deposition" << G4endl;
    }
    
    // 检查末端步长是否合理
    G4double finalRange = em->MuHadStepFunctionP2();
    if (finalRange > 100.*um) {
        G4cout << "Warning: finalRange=" << finalRange/um 
               << " μm may be too large for Bragg peak resolution" << G4endl;
    }
    
    // 检查production cuts
    G4ProductionCutsTable* cutsTable = G4ProductionCutsTable::GetProductionCutsTable();
    G4double gammaCut = cutsTable->GetLowEdgeEnergy(0, 0);  // gamma cut
    if (gammaCut > 10.*keV) {
        G4cout << "Warning: gamma cut=" << gammaCut/keV 
               << " keV may affect energy deposition accuracy" << G4endl;
    }
}
```

---

## 10. 实用建议

### 按影响大小排序的设置优先级

#### **高影响（必须设对）：**

1. **Production Cuts**
```cpp
// 直接影响能量沉积精度
SetCutValue(1-10*um, "gamma");
SetCutValue(1-10*um, "e-");
SetCutValue(1-10*um, "e+");
```

2. **步长设置**
```cpp
// 影响空间分辨率和Bragg峰形状
em->SetStepFunctionMuHad(0.02-0.05, 5-50*um);
```

3. **物理模型选择**
```cpp
// 决定基础物理精度
RegisterPhysics(new G4EmStandardPhysics_option4()); // 高精度
```

4. **能量范围与表格精度**
```cpp
// 影响dE/dx和截面插值精度
em->SetNumberOfBinsPerDecade(20);
em->SetMinEnergy(100*eV);
```

#### **中等影响（建议优化）：**

1. **多重散射参数**：影响角分布和轨迹
2. **材料参数**：影响阻止本领计算
3. **涨落模型**：影响统计精度

#### **低影响（可用默认）：**

1. **随机数引擎**：影响统计质量
2. **边界精度**：几何复杂时重要
3. **磁场设置**：有磁场时关键

### 快速配置模板

```cpp
// 高精度配置（推荐用于精确计算）
void SetHighPrecisionMode()
{
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 步长设置
    em->SetStepFunctionMuHad(0.02, 5*um);
    em->SetStepFunction(0.1, 0.01*mm);
    
    // 表格精度
    em->SetNumberOfBinsPerDecade(20);
    em->SetSpline(true);
    
    // 物理过程
    RegisterPhysics(new G4EmStandardPhysics_option4());
    
    // Production cuts
    SetCutValue(1*um, "gamma");
    SetCutValue(1*um, "e-");
    SetCutValue(1*um, "e+");
}

// 平衡配置（推荐用于一般模拟）
void SetBalancedMode()
{
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 自动步长
    G4double thickness = GetTargetThickness();
    G4double finalRange = std::max(5.*um, std::min(thickness/1000., 50.*um));
    em->SetStepFunctionMuHad(0.03, finalRange);
    
    // 标准精度
    em->SetNumberOfBinsPerDecade(15);
    RegisterPhysics(new G4EmStandardPhysics_option3());
    
    // 合理的cuts
    SetCutValue(10*um, "gamma");
    SetCutValue(10*um, "e-");
    SetCutValue(10*um, "e+");
}

// 快速配置（用于参数扫描）
void SetFastMode()
{
    G4EmParameters* em = G4EmParameters::Instance();
    
    // 较大步长
    em->SetStepFunctionMuHad(0.1, 50*um);
    
    // 标准物理
    RegisterPhysics(new G4EmStandardPhysics());
    
    // 较大cuts
    SetCutValue(100*um, "gamma");
    SetCutValue(100*um, "e-");
    SetCutValue(100*um, "e+");
}
```

### 验证设置的方法

```cpp
// 与外部基准对比验证
void ValidateAgainstSRIM()
{
    // 1. 比较射程：模拟 vs SRIM/NIST
    // 要求：投影射程偏差 ≤ 3-5%
    
    // 2. 比较Bragg峰：峰位、峰高、FWHM
    // 要求：峰位偏差 ≤ 1个bin，峰高偏差 ≤ 5%
    
    // 3. 比较总沉积能：能量守恒检查
    // 要求：|入射能量 - 沉积能量 - 逃逸能量| ≤ 1%
}
```

---

## 总结

**核心原则**：Production cuts + 步长 + 物理模型 + 能量表格精度，这四个设置控制了90%的Geant4模拟精度。

**自动化策略**：使用基于几何厚度的自动步长公式，配合合理的production cuts，可以在不同应用中实现"一次设置，处处适用"的效果。

**验证方法**：通过与SRIM/NIST等外部基准对比，确保模拟结果的物理一致性在工程可接受范围内（通常3-5%）。

---

*本指南基于Geant4.11.x版本编写，适用于辐射物理、医学物理、高能物理等领域的精确模拟需求。*