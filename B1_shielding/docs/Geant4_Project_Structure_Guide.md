# Geant4 项目结构详解

## 概述

本文档详细介绍Geant4项目中每个源文件和头文件的作用，帮助初学者理解Geant4应用程序的基本架构和工作流程。

## 项目结构总览

```
B1_shielding/
├── exampleB1.cc                    # 主程序入口
├── include/                         # 头文件目录
│   ├── ActionInitialization.hh     # 动作初始化类声明
│   ├── CustomPhysicsList.hh        # 自定义物理列表声明
│   ├── DetectorConstruction.hh     # 探测器构建类声明
│   ├── DPAModelConfig.hh           # DPA模型配置声明
│   ├── EventAction.hh              # 事件动作类声明
│   ├── PrimaryGeneratorAction.hh   # 源初粒子产生器声明
│   ├── RunAction.hh                # 运行动作类声明
│   └── SteppingAction.hh           # 步进动作类声明
├── src/                            # 源文件目录
│   ├── ActionInitialization.cc     # 动作初始化类实现
│   ├── CustomPhysicsList.cc        # 自定义物理列表实现
│   ├── DetectorConstruction.cc     # 探测器构建类实现
│   ├── EventAction.cc              # 事件动作类实现
│   ├── PrimaryGeneratorAction.cc   # 源初粒子产生器实现
│   ├── RunAction.cc                # 运行动作类实现
│   └── SteppingAction.cc           # 步进动作类实现
├── CMakeLists.txt                  # CMake构建配置
├── run.mac                         # Geant4宏文件
└── vis.mac                         # 可视化宏文件
```

---

## 主程序文件

### `exampleB1.cc` - 程序入口

**作用**：程序的主入口点，负责初始化 Geant4 内核并启动模拟。

**主要功能**：
- 初始化 Geant4 运行管理器
- 注册必要的用户类（探测器、物理、操作等）
- 处理命令行参数和宏文件
- 启动 UI 界面或批处理模式

**典型代码结构**：
```cpp
int main(int argc, char** argv)
{
    // 构建默认运行管理器
    auto* runManager = G4RunManagerFactory::CreateRunManager();
    
    // 设置必要的初始化类
    runManager->SetUserInitialization(new B1::DetectorConstruction());
    runManager->SetUserInitialization(new CustomPhysicsList());
    runManager->SetUserInitialization(new B1::ActionInitialization());
    
    // 初始化G4内核
    runManager->Initialize();
    
    // 处理宏文件或启动UI
    if (argc == 1) {
        // 交互模式
        ui->SessionStart();
    } else {
        // 批处理模式
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    
    delete runManager;
    return 0;
}
```

**学习要点**：
- 这是学习 Geant4 的起点，理解程序启动流程
- 掌握批处理模式和交互模式的区别
- 了解用户类的注册顺序和重要性

---

## 头文件详解 (`include/` 目录)

### `DetectorConstruction.hh` - 探测器构建类

**作用**：定义模拟中的几何结构、材料和探测器配置。

**核心内容**：
```cpp
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();
    
    virtual G4VPhysicalVolume* Construct();  // 构建几何的纯虚函数
    virtual void ConstructSDandField();      // 构建敏感探测器和场
    
    G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

private:
    G4LogicalVolume* fScoringVolume;         // 计分体积
    G4Material* DefineShieldingGlass();      // 定义屏蔽玻璃材料
};
```

**学习要点**：
- 必须继承 `G4VUserDetectorConstruction`
- `Construct()` 是核心方法，返回世界体积
- 理解材料定义、几何构建、逻辑体积概念

### `CustomPhysicsList.hh` - 物理过程列表

**作用**：定义模拟中使用的物理过程和模型。

**核心内容**：
```cpp
class CustomPhysicsList : public G4VModularPhysicsList
{
public:
    CustomPhysicsList();
    virtual ~CustomPhysicsList();
    
    virtual void SetCuts();                  // 设置产生阈值
    void SetEMPhysicsOption(G4int option);   // 选择电磁物理模型

private:
    G4int fEMPhysicsOption;                  // 电磁物理选项
    G4double defaultCutValue;                // 默认截断值
    G4double cutForGamma, cutForElectron, cutForPositron;
};
```

**学习要点**：
- 控制哪些物理过程参与模拟
- 理解 production cuts 的重要性
- 掌握不同物理模型的适用场景

### `PrimaryGeneratorAction.hh` - 初级粒子发生器

**作用**：定义初级粒子的类型、能量、位置和方向。

**核心内容**：
```cpp
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();
    
    virtual void GeneratePrimaries(G4Event*);  // 生成初级粒子
    
    const G4ParticleGun* GetParticleGun() const { return fParticleGun; }

private:
    G4ParticleGun* fParticleGun;              // 粒子枪
};
```

**学习要点**：
- 每个事件开始时调用 `GeneratePrimaries()`
- 可以定义单个粒子或粒子束
- 理解粒子枪的配置参数

### `ActionInitialization.hh` - 动作初始化

**作用**：统一管理和初始化所有用户动作类。

**核心内容**：
```cpp
class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization();
    virtual ~ActionInitialization();
    
    virtual void BuildForMaster() const;     // 主线程初始化
    virtual void Build() const;              // 工作线程初始化
};
```

**学习要点**：
- 多线程模式下的动作管理
- 区分主线程和工作线程的初始化
- 理解动作类的生命周期

### `RunAction.hh` - 运行级动作

**作用**：处理整个运行（Run）开始和结束时的操作。

**核心内容**：
```cpp
class RunAction : public G4UserRunAction
{
public:
    RunAction();
    virtual ~RunAction();
    
    virtual void BeginOfRunAction(const G4Run*);  // 运行开始
    virtual void EndOfRunAction(const G4Run*);    // 运行结束
    
    void AddEdep(G4double edep) { fEdep += edep; }

private:
    G4double fEdep, fEdep2;                       // 能量沉积累积
};
```

**学习要点**：
- 一个 Run 包含多个 Event
- 用于统计分析和数据输出
- 理解数据累积和平均的概念

### `EventAction.hh` - 事件级动作

**作用**：处理单个事件开始和结束时的操作。

**核心内容**：
```cpp
class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction* runAction);
    virtual ~EventAction();
    
    virtual void BeginOfEventAction(const G4Event*);  // 事件开始
    virtual void EndOfEventAction(const G4Event*);    // 事件结束
    
    void AddEdep(G4double edep) { fEdep += edep; }

private:
    RunAction* fRunAction;                            // 运行动作指针
    G4double fEdep;                                   // 事件能量沉积
};
```

**学习要点**：
- 一个 Event 对应一个初级粒子及其次级粒子
- 事件级数据收集和处理
- 与 RunAction 的数据传递

### `SteppingAction.hh` - 步进级动作

**作用**：处理粒子传输过程中每一步的详细信息。

**核心内容**：
```cpp
class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(EventAction* eventAction);
    virtual ~SteppingAction();
    
    virtual void UserSteppingAction(const G4Step*);   // 每步调用

private:
    EventAction* fEventAction;                        // 事件动作指针
};
```

**学习要点**：
- 最细粒度的信息收集
- 可以访问位置、能量、时间等详细信息
- 计算密集，需要谨慎使用

### `DPAModelConfig.hh` - DPA模型配置

**作用**：配置位移损伤（DPA）计算模型的参数。

**核心内容**：
```cpp
class DPAModelConfig
{
public:
    enum DPAModel {
        NRT_MODEL,      // NRT模型
        SRIM_MODEL      // SRIM模型
    };
    
    static void SetDPAModel(DPAModel model);
    static DPAModel GetDPAModel();
    
private:
    static DPAModel fCurrentModel;
};
```

**学习要点**：
- 专门用于辐射损伤计算
- 理解不同DPA模型的区别
- 配置类的设计模式

---

## 源文件详解 (`src/` 目录)

### `DetectorConstruction.cc` - 几何构建实现

**主要功能**：
1. **材料定义**：创建各种材料（玻璃、空气等）
2. **几何构建**：定义世界体积、目标体积等
3. **体积放置**：将逻辑体积放置到物理空间
4. **用户限制**：设置步长限制等参数

**关键代码段**：
```cpp
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // 1. 定义材料
    G4Material* shieldingGlass = DefineShieldingGlass();
    G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
    
    // 2. 创建几何形状
    auto solidWorld = new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
    auto solidGlass = new G4Box("ShieldingGlass", 0.5*glass_sizeXY, 0.5*glass_sizeXY, 0.5*glass_sizeZ);
    
    // 3. 创建逻辑体积
    auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
    auto logicGlass = new G4LogicalVolume(solidGlass, shieldingGlass, "ShieldingGlass");
    
    // 4. 物理放置
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "World", nullptr, false, 0);
    new G4PVPlacement(nullptr, G4ThreeVector(), logicGlass, "ShieldingGlass", logicWorld, false, 0);
    
    return physWorld;
}
```

**学习要点**：
- Solid（形状）→ Logical（材料+形状）→ Physical（位置+方向）
- 世界体积必须包含所有其他体积
- 材料属性直接影响物理过程

### `CustomPhysicsList.cc` - 物理过程实现

**主要功能**：
1. **物理包注册**：选择电磁、强子等物理模型
2. **精度设置**：配置步长、能量表格等参数
3. **截断值设置**：定义粒子产生阈值

**关键代码段**：
```cpp
CustomPhysicsList::CustomPhysicsList()
{
    // 自动步长设置
    G4double glassThickness = 75.*mm;
    G4double finalRange = std::max(5.*um, std::min(glassThickness/1000., 50.*um));
    
    G4EmParameters* em = G4EmParameters::Instance();
    em->SetStepFunctionMuHad(0.03, finalRange);  // 3%相对步长
    em->SetNumberOfBinsPerDecade(20);            // 表格精度
    
    // 注册物理过程
    RegisterPhysics(new G4EmStandardPhysics_option4());  // 高精度EM
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());     // 强子物理
}
```

**学习要点**：
- 物理模型的选择影响计算精度和速度
- 步长设置是精度控制的关键
- 不同能量范围需要不同的物理模型

### `PrimaryGeneratorAction.cc` - 粒子产生器实现

**主要功能**：
1. **粒子类型设置**：定义粒子种类（质子、电子等）
2. **能量设置**：设置初始动能
3. **位置和方向**：定义发射位置和方向

**关键代码段**：
```cpp
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // 设置粒子类型
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("proton");
    fParticleGun->SetParticleDefinition(particle);
    
    // 设置能量
    fParticleGun->SetParticleEnergy(6*MeV);
    
    // 设置位置和方向
    fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., -4*cm));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
    
    // 生成粒子
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
```

**学习要点**：
- 每个事件都会调用这个文件爱你
- 可以生成不同类型的粒子束
- 位置和方向的坐标系统

### `ActionInitialization.cc` - 动作管理实现

**主要功能**：
1. **创建用户动作**：实例化各种动作类
2. **多线程管理**：区分主线程和工作线程
3. **动作关联**：建立动作类之间的联系

**关键代码段**：
```cpp
void ActionInitialization::Build() const
{
    // 创建粒子发生器
    SetUserAction(new PrimaryGeneratorAction);
    
    // 创建运行动作
    RunAction* runAction = new RunAction;
    SetUserAction(runAction);
    
    // 创建事件动作（需要运行动作指针）
    EventAction* eventAction = new EventAction(runAction);
    SetUserAction(eventAction);
    
    // 创建步进动作（需要事件动作指针）
    SetUserAction(new SteppingAction(eventAction));
}
```

**学习要点**：
- 动作类之间的依赖关系
- 数据在不同动作类间的传递
- 多线程模式下的对象管理

### `RunAction.cc` - 运行管理实现

**主要功能**：
1. **数据初始化**：运行开始时重置累积量
2. **文件管理**：创建和关闭输出文件
3. **统计计算**：计算平均值、标准差等

**关键代码段**：
```cpp
void RunAction::BeginOfRunAction(const G4Run*)
{
    // 重置累积量
    fEdep = fEdep2 = 0.;
    
    // 创建分析管理器
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile("B1");
    
    // 创建直方图
    analysisManager->CreateH1("Edep","Edep in absorber", 100, 0., 800*MeV);
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;
    
    // 计算统计量
    G4double edep  = fEdep/nofEvents;
    G4double edep2 = fEdep2/nofEvents;
    G4double rms = edep2 - edep*edep;
    
    // 输出结果
    G4cout << "Mean energy deposit: " << edep/MeV << " MeV" << G4endl;
}
```

**学习要点**：
- 运行级数据的生命周期
- 统计分析的基本方法
- 文件I/O的管理

### `EventAction.cc` - 事件处理实现

**主要功能**：
1. **事件数据重置**：每个事件开始时清零
2. **数据传递**：将事件数据传递给运行动作
3. **事件级分析**：对单个事件进行处理

**关键代码段**：
```cpp
void EventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;  // 重置事件能量沉积
}

void EventAction::EndOfEventAction(const G4Event*)
{
    // 将事件数据累积到运行数据
    fRunAction->AddEdep(fEdep);
    
    // 填充直方图
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillH1(0, fEdep);
}
```

**学习要点**：
- 事件级数据的处理流程
- 与运行动作的数据交换
- 直方图填充的时机

### `SteppingAction.cc` - 步进处理实现

**主要功能**：
1. **步长信息获取**：获取位置、能量等信息
2. **体积判断**：判断粒子在哪个体积中
3. **详细计算**：进行DPA、能量沉积等计算

**关键代码段**：
```cpp
void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // 获取体积信息
    G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()
                                   ->GetVolume()->GetLogicalVolume();
    
    // 检查是否在计分体积中
    const DetectorConstruction* detectorConstruction
        = static_cast<const DetectorConstruction*>
          (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4LogicalVolume* fScoringVolume = detectorConstruction->GetScoringVolume();
    
    if (volume != fScoringVolume) return;
    
    // 获取能量沉积
    G4double edepStep = step->GetTotalEnergyDeposit();
    
    // 累积到事件动作
    fEventAction->AddEdep(edepStep);
}
```

**学习要点**：
- Step 包含丰富的物理信息
- 体积过滤的重要性
- 能量沉积的逐步累积

---

## Geant4 数据流程图

```
程序启动 (main)
    ↓
探测器构建 (DetectorConstruction)
    ↓
物理过程注册 (PhysicsList)
    ↓
动作初始化 (ActionInitialization)
    ↓
═══════════════════════════════════
运行开始 (RunAction::BeginOfRunAction)
    ↓
    ┌─→ 事件开始 (EventAction::BeginOfEventAction)
    │       ↓
    │   生成初级粒子 (PrimaryGeneratorAction)
    │       ↓
    │   ┌─→ 粒子步进 (SteppingAction::UserSteppingAction)
    │   │       ↓
    │   │   [物理过程计算]
    │   │       ↓
    │   └─── [继续下一步] ←─┐
    │           ↓          │
    │       [粒子死亡?] ────┘
    │           ↓ Yes
    │   事件结束 (EventAction::EndOfEventAction)
    │       ↓
    └─── [更多事件?] ←─────┘
            ↓ No
运行结束 (RunAction::EndOfRunAction)
    ↓
程序结束
```

---

## 学习建议

### 初学者路径：
1. **从 `exampleB1.cc` 开始**：理解程序启动流程
2. **学习 `DetectorConstruction`**：掌握几何和材料定义
3. **理解 `PrimaryGeneratorAction`**：学会设置初级粒子
4. **掌握动作类层次**：Run → Event → Step 的数据流

### 进阶学习：
1. **深入 `PhysicsList`**：理解物理模型选择
2. **优化 `SteppingAction`**：进行复杂的物理计算
3. **扩展数据分析**：添加更多输出和统计

### 实践建议：
1. **修改参数**：改变粒子类型、能量、几何尺寸
2. **添加功能**：增加新的计分量、输出格式
3. **性能优化**：理解不同设置对计算速度的影响

---

## 常见问题

### Q: 为什么需要这么多类？
**A**: Geant4 采用面向对象设计，将不同功能分离，便于维护和扩展。每个类负责特定的任务，符合单一职责原则。

### Q: 数据如何在类之间传递？
**A**: 通过指针传递和累积机制：Step → Event → Run，每个级别收集相应的数据。

### Q: 如何选择合适的物理模型？
**A**: 根据粒子类型、能量范围和精度要求选择。高能用 Standard，低能用 Livermore，极低能用 DNA 物理。

### Q: 什么时候需要修改 SteppingAction？
**A**: 当需要详细的步长信息、复杂的物理计算或特殊的数据收集时。但要注意性能影响。

---

*本指南适用于 Geant4.11.x 版本，帮助学生快速理解 Geant4 应用程序的基本结构和工作原理。*