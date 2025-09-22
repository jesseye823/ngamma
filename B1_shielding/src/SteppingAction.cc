//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/src/SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "DPAModelConfig.hh"
#include "G4AnalysisManager.hh"
#include "G4VProcess.hh"
#include <map>
#include <fstream>
#include <sstream>

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace {
  // 懒加载 SRIM Ed 数据表：文本格式 每行: <ElementNameOrSymbol> <Ed_eV>
  std::map<G4String, G4double>& SRIM_Ed_Map()
  {
    static std::map<G4String, G4double> table;
    static G4bool loaded = false;
    if (!loaded) {
      const char* candidates[] = {
        "/home/jesse/ngamma/B1_shielding/SRIM_Ed.dat",
        "../SRIM_Ed.dat",
        "SRIM_Ed.dat"
      };
      for (const char* path : candidates) {
        std::ifstream fin(path);
        if (!fin.good()) continue;
        std::string line;
        while (std::getline(fin, line)) {
          if (line.empty() || line[0] == '#') continue;
          std::istringstream iss(line);
          std::string name; double ed_eV;
          if (!(iss >> name >> ed_eV)) continue;
          table[G4String(name)] = ed_eV * eV;
        }
        break; // 读取到第一个可用文件即停止
      }
      loaded = true;
    }
    return table;
  }

  // 查表获取元素Ed（若未配置则返回负数表示未命中）
  G4double SRIM_Ed_Lookup(const G4String& elementName)
  {
    auto& tbl = SRIM_Ed_Map();
    auto it = tbl.find(elementName);
    if (it != tbl.end()) return it->second;
    return -1.0; // 未命中
  }
}

SteppingAction::SteppingAction(EventAction* eventAction)
  : fEventAction(eventAction)
{
  G4cout << "SteppingAction constructor called" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!fScoringVolume) {
    const DetectorConstruction* detConstruction
      = static_cast<const DetectorConstruction*>
        (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    fScoringVolume = detConstruction->GetScoringVolume();
  }

  // get volume of the current step
  G4LogicalVolume* volume
    = step->GetPreStepPoint()->GetTouchableHandle()
      ->GetVolume()->GetLogicalVolume();

  // check if we are in scoring volume
  if (volume != fScoringVolume) return;

  // collect energy deposited in this step
  G4double edepStep = step->GetTotalEnergyDeposit();
  fEventAction->AddEdep(edepStep);
  
  // 计算DPA（根据配置选择模型）
  G4double dpa = CalculateDPA(step);
  fEventAction->AddDPA(dpa);

  // 计算NIEL（完整版）：带电粒子核阻止 + 中子PKA经Lindhard分配
  G4double niel = CalculateNIEL(step);
  fEventAction->AddNIEL(niel);

  // 记录轨迹信息（限制记录数量以避免文件过大）
  static G4int totalSteps = 0;
  totalSteps++;
  if (totalSteps % 100 == 0) {  // 每100步记录一次轨迹
    const G4Track* track = step->GetTrack();
    G4ThreeVector position = step->GetPreStepPoint()->GetPosition();
    fEventAction->FillTrack(
      track->GetTrackID(),
      track->GetParentID(), 
      track->GetDefinition()->GetPDGEncoding(),
      position.x()/cm, position.y()/cm, position.z()/cm,  // 位置转换为cm
      track->GetKineticEnergy()/MeV,
      track->GetGlobalTime()/ns,
      track->GetCurrentStepNumber()
    );
  }

  // 透射与俘获诊断
  auto analysis = G4AnalysisManager::Instance();
  if (analysis) {
    const G4Track* trk = step->GetTrack();
    const G4ParticleDefinition* pd = trk->GetDefinition();
    G4int pdg = pd->GetPDGEncoding();
    G4double Epre = step->GetPreStepPoint()->GetKineticEnergy();
    G4double Ek = step->GetPostStepPoint()->GetKineticEnergy();

    // 入射能谱：当进入计分体的第一步
    if (step->GetPreStepPoint()->GetStepStatus() == fGeomBoundary && volume == fScoringVolume) {
      if (pdg == 22) analysis->FillH1(7, Epre);       // Gamma_Incident_E -> H1 index 7
      if (pdg == 2112) analysis->FillH1(8, Epre);     // Neutron_Incident_E -> H1 index 8
    }

    // 透射：离开计分体
    auto postPhys = step->GetPostStepPoint()->GetPhysicalVolume();
    if (volume == fScoringVolume && (!postPhys || postPhys->GetLogicalVolume() != fScoringVolume)) {
      if (pdg == 22) analysis->FillH1(3, Ek);
      if (pdg == 2112) analysis->FillH1(4, Ek);
    }

    // 俘获过程
    const G4VProcess* proc = step->GetPostStepPoint()->GetProcessDefinedStep();
    if (proc) {
      G4String pname = proc->GetProcessName();
      if (pname == "nCapture" && pdg == 2112) {
        analysis->FillH1(5, Epre);              // Neutron_Capture_E
        analysis->FillH1(9, 1.0);               // Capture_Count（累加）
        // 遍历本步产生的次级，记录俘获γ
        const auto* secs = step->GetSecondaryInCurrentStep();
        if (secs) {
          for (const auto* s : *secs) {
            if (s && s->GetDefinition()->GetPDGEncoding() == 22) {
              G4double Eg = s->GetKineticEnergy();
              analysis->FillH1(6, Eg);          // Capture_Gamma_E
              auto pos = step->GetPostStepPoint()->GetPosition();
              analysis->FillNtupleDColumn(1, 0, Epre);   // ActivationProducts.PreNeutronE
              analysis->FillNtupleDColumn(1, 1, Eg);     // CaptureGammaE
              analysis->FillNtupleDColumn(1, 2, pos.x());
              analysis->FillNtupleDColumn(1, 3, pos.y());
              analysis->FillNtupleDColumn(1, 4, pos.z());
              analysis->AddNtupleRow(1);
            }
          }
        }
      }
      // γ能谱统计（综合）
      if (pdg == 22) {
        analysis->FillH1(6, Ek);
      }
    }
  }
}

// 主DPA计算函数（根据配置选择模型）
G4double SteppingAction::CalculateDPA(const G4Step* step)
{
  DPAModelType currentModel = DPAModelConfig::GetCurrentModel();
  
  switch (currentModel) {
    case DPAModelType::NRT:
      return CalculateNRT_DPA(step);
    case DPAModelType::SRIM:
      return CalculateSRIM_DPA(step);
    default:
      return CalculateNRT_DPA(step);  // 默认使用NRT模型
  }
}

// NRT (Norgett-Robinson-Torrens) DPA模型实现
G4double SteppingAction::CalculateNRT_DPA(const G4Step* step)
{
  G4double edep = step->GetTotalEnergyDeposit();
  G4double stepLength = step->GetStepLength();
  
  if (edep <= 0. || stepLength <= 0.) return 0.;
  
  // 获取粒子信息
  const G4Track* track = step->GetTrack();
  const G4ParticleDefinition* particle = track->GetDefinition();
  G4int pdgCode = particle->GetPDGEncoding();
  G4double kineticEnergy = step->GetPreStepPoint()->GetKineticEnergy();
  
  // 获取材料信息
  const G4Material* material = step->GetPreStepPoint()->GetMaterial();
  G4double density = material->GetDensity();
  
  // 计算材料平均原子量
  G4double atomicWeight = 0.;
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* fractions = material->GetFractionVector();
  G4int nElements = material->GetNumberOfElements();
  
  for (G4int i = 0; i < nElements; i++) {
    atomicWeight += (*elements)[i]->GetA() * fractions[i];
  }
  
  // NRT模型参数（基于闪烁体玻璃材料）
  G4double Ed = GetDisplacementThreshold(material);  // 材料相关的位移阈值
  G4double Td = 0.8 * Ed;  // 有效位移阈值
  
  // 计算反冲能量（基于粒子类型）
  G4double T = CalculateRecoilEnergy(kineticEnergy, pdgCode, atomicWeight);
  
  // NRT公式：ν(T) = 0.8 × T / (2 × Ed)
  G4double nu = 0.8 * T / (2.0 * Ed);
  
  // 原子数密度
  G4double N = density * Avogadro / atomicWeight;
  
  // 体积
  G4double V = stepLength * 1.*cm2;
  
  // NRT DPA计算
  G4double dpa = nu * edep / (2.0 * Ed * N * V);
  
  return dpa;
}

// 获取材料相关的位移阈值能量
G4double SteppingAction::GetDisplacementThreshold(const G4Material* material)
{
  G4String materialName = material->GetName();
  
  // 基于闪烁体玻璃组分的位移阈值
  if (materialName.find("Glass") != G4String::npos || materialName.find("Scintillator") != G4String::npos) {
    // 若SRIM表存在元素条目，则按元素权重平均，否则回退到默认典型值
    const G4ElementVector* elements = material->GetElementVector();
    const G4double* fractions = material->GetFractionVector();
    G4int nElements = material->GetNumberOfElements();
    G4double sumEd = 0., sumW = 0.;
    for (G4int i = 0; i < nElements; i++) {
      G4String ename = (*elements)[i]->GetName();
      G4double ed = SRIM_Ed_Lookup(ename);
      if (ed > 0.) { sumEd += ed * fractions[i]; sumW += fractions[i]; }
    }
    if (sumW > 0.) return sumEd; // 使用SRIM权重平均
    return 30.*eV;  // 回退：玻璃典型值
  }
  
  // 元素特定的位移阈值
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* fractions = material->GetFractionVector();
  G4int nElements = material->GetNumberOfElements();
  
  G4double weightedEd = 0.;
  for (G4int i = 0; i < nElements; i++) {
    G4String elementName = (*elements)[i]->GetName();
    G4double elementEd = 25.*eV;  // 默认值
    if (G4double edTab = SRIM_Ed_Lookup(elementName); edTab > 0.) {
      elementEd = edTab;
    }
    
    if (elementName == "Si") elementEd = 25.*eV;
    else if (elementName == "O") elementEd = 20.*eV;
    else if (elementName == "B") elementEd = 15.*eV;
    else if (elementName == "Li") elementEd = 10.*eV;
    else if (elementName == "Mg") elementEd = 25.*eV; // 典型金属位移阈值
    else if (elementName == "Al") elementEd = 25.*eV; // 常用NRT默认值
    else if (elementName == "Ce") elementEd = 40.*eV; // 稀土元素较高阈值
    else if (elementName == "Gd") elementEd = 40.*eV; // 稀土元素较高阈值
    else if (elementName == "Na") elementEd = 18.*eV;
    else if (elementName == "K") elementEd = 22.*eV;
    else if (elementName == "Ba") elementEd = 35.*eV;
    else if (elementName == "Pb") elementEd = 40.*eV;
    
    weightedEd += elementEd * fractions[i];
  }
  
  return weightedEd;
}

// 计算反冲能量
G4double SteppingAction::CalculateRecoilEnergy(G4double kineticEnergy, G4int pdgCode, G4double atomicWeight)
{
  G4double T = 0.;
  
  if (pdgCode == 2112) {  // 中子
    // 中子-核弹性散射的最大能量传递
    T = 4.0 * kineticEnergy * atomicWeight / 
        ((1.0 + atomicWeight) * (1.0 + atomicWeight));
  } else if (pdgCode == 2212) {  // 质子
    // 质子-核弹性散射
    T = 4.0 * kineticEnergy * atomicWeight / 
        ((1.0 + atomicWeight) * (1.0 + atomicWeight));
  } else if (pdgCode == 22) {  // γ射线
    // γ射线通过光电效应和康普顿散射
    T = kineticEnergy * 0.1;  // 约10%能量传递给反冲电子
  } else {
    // 其他粒子
    T = kineticEnergy * 0.5;
  }
  
  return T;
}

// SRIM (Stopping and Range of Ions in Matter) DPA模型实现
G4double SteppingAction::CalculateSRIM_DPA(const G4Step* step)
{
  G4double edep = step->GetTotalEnergyDeposit();
  G4double stepLength = step->GetStepLength();
  
  if (edep <= 0. || stepLength <= 0.) return 0.;
  
  // 获取粒子信息
  const G4Track* track = step->GetTrack();
  const G4ParticleDefinition* particle = track->GetDefinition();
  G4int pdgCode = particle->GetPDGEncoding();
  G4double kineticEnergy = step->GetPreStepPoint()->GetKineticEnergy();
  
  // 获取材料信息
  const G4Material* material = step->GetPreStepPoint()->GetMaterial();
  G4double density = material->GetDensity();
  
  // 计算材料平均原子量
  G4double atomicWeight = 0.;
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* fractions = material->GetFractionVector();
  G4int nElements = material->GetNumberOfElements();
  
  for (G4int i = 0; i < nElements; i++) {
    atomicWeight += (*elements)[i]->GetA() * fractions[i];
  }
  
  // SRIM模型参数
  G4double Ed = GetSRIMDisplacementThreshold(material);  // SRIM位移阈值
  
  // 计算核阻止本领（对位移损伤贡献最大）
  G4double nuclearStoppingPower = CalculateNuclearStoppingPower(kineticEnergy, pdgCode, material);
  
  // 计算电子阻止本领（对位移损伤贡献较小）
  G4double electronicStoppingPower = CalculateElectronicStoppingPower(kineticEnergy, pdgCode, material);
  
  // SRIM DPA计算：DPA = (dE/dx)_nuclear / (2 * Ed * N)
  G4double N = density * Avogadro / atomicWeight;  // 原子数密度
  
  // 主要贡献来自核阻止本领
  G4double dpa = nuclearStoppingPower * stepLength / (2.0 * Ed * N);
  
  // 添加电子阻止本领的贡献（较小）
  dpa += electronicStoppingPower * stepLength * 0.1 / (2.0 * Ed * N);
  
  return dpa;
}

// 计算核阻止本领
G4double SteppingAction::CalculateNuclearStoppingPower(G4double energy, G4int pdgCode, const G4Material* material)
{
  G4double stoppingPower = 0.;
  
  if (pdgCode == 2112) {  // 中子
    // 中子核阻止本领（基于能量）
    if (energy < 1.*keV) {
      stoppingPower = 1.0e-3 * MeV / (g/cm2);  // 热中子
    } else if (energy < 1.*MeV) {
      stoppingPower = 1.0e-2 * MeV / (g/cm2);  // 快中子
    } else {
      stoppingPower = 1.0e-1 * MeV / (g/cm2);  // 高能中子
    }
  } else if (pdgCode == 2212) {  // 质子
    // 质子核阻止本领（基于Bethe-Bloch公式简化）
    stoppingPower = 0.1 * MeV / (g/cm2) * std::log(energy / (1.*MeV));
  } else if (pdgCode == 22) {  // γ射线
    // γ射线通过次级电子产生核阻止
    stoppingPower = 1.0e-4 * MeV / (g/cm2);
  } else {
    // 其他粒子
    stoppingPower = 1.0e-2 * MeV / (g/cm2);
  }
  
  return stoppingPower;
}

// 计算电子阻止本领
G4double SteppingAction::CalculateElectronicStoppingPower(G4double energy, G4int pdgCode, const G4Material* material)
{
  G4double stoppingPower = 0.;
  
  if (pdgCode == 2112) {  // 中子
    // 中子电子阻止本领（较小）
    stoppingPower = 1.0e-4 * MeV / (g/cm2);
  } else if (pdgCode == 2212) {  // 质子
    // 质子电子阻止本领（基于Bethe-Bloch公式）
    stoppingPower = 1.0 * MeV / (g/cm2) * std::log(energy / (1.*MeV));
  } else if (pdgCode == 22) {  // γ射线
    // γ射线电子阻止本领
    stoppingPower = 1.0e-2 * MeV / (g/cm2);
  } else {
    // 其他粒子
    stoppingPower = 1.0e-1 * MeV / (g/cm2);
  }
  
  return stoppingPower;
}

// 获取SRIM位移阈值
G4double SteppingAction::GetSRIMDisplacementThreshold(const G4Material* material)
{
  G4String materialName = material->GetName();
  
  // 基于闪烁体玻璃组分的SRIM位移阈值
  if (materialName.find("Glass") != G4String::npos || materialName.find("Scintillator") != G4String::npos) {
    return 25.*eV;  // SRIM推荐的玻璃材料值
  }
  
  // 元素特定的SRIM位移阈值
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* fractions = material->GetFractionVector();
  G4int nElements = material->GetNumberOfElements();
  
  G4double weightedEd = 0.;
  for (G4int i = 0; i < nElements; i++) {
    G4String elementName = (*elements)[i]->GetName();
    G4double elementEd = 25.*eV;  // SRIM默认值
    if (G4double edTab = SRIM_Ed_Lookup(elementName); edTab > 0.) {
      elementEd = edTab;
    }
    
    if (elementName == "Si") elementEd = 25.*eV;      // SRIM推荐值
    else if (elementName == "O") elementEd = 20.*eV;  // SRIM推荐值
    else if (elementName == "B") elementEd = 15.*eV;  // SRIM推荐值
    else if (elementName == "Li") elementEd = 10.*eV; // SRIM推荐值
    else if (elementName == "Mg") elementEd = 25.*eV; // 参考典型金属
    else if (elementName == "Al") elementEd = 25.*eV; // 文献常用
    else if (elementName == "Ce") elementEd = 35.*eV; // 稀土较高
    else if (elementName == "Gd") elementEd = 35.*eV; // 稀土较高
    else if (elementName == "Na") elementEd = 18.*eV; // SRIM推荐值
    else if (elementName == "K") elementEd = 22.*eV;  // SRIM推荐值
    else if (elementName == "Ba") elementEd = 30.*eV; // SRIM推荐值
    else if (elementName == "Pb") elementEd = 35.*eV; // SRIM推荐值
    
    weightedEd += elementEd * fractions[i];
  }
  
  return weightedEd;
}

// NIEL（非电离能量损失）完整版计算
G4double SteppingAction::CalculateNIEL(const G4Step* step)
{
  const G4Track* track = step->GetTrack();
  const G4ParticleDefinition* particle = track->GetDefinition();
  G4int pdg = particle->GetPDGEncoding();
  const G4Material* material = step->GetPreStepPoint()->GetMaterial();
  G4double energy = step->GetPreStepPoint()->GetKineticEnergy();
  G4double dx = step->GetStepLength();
  if (dx <= 0.) return 0.;

  // 平均原子序Z与质量数A（加权）
  G4double Zbar = 10., Abar = 20.;
  ComputeAverageZA(material, Zbar, Abar);

  // 带电粒子：使用核阻止本领近似（SRIM/ZBL风格）
  if (pdg != 2112 && pdg != 22) {
    G4double Sn = CalculateNuclearStoppingPower(energy, pdg, material); // MeV/(g/cm2)
    G4double rho = material->GetDensity(); // g/cm3
    G4double dEnonion = Sn * rho * dx;     // MeV
    return dEnonion;
  }

  // 中子：通过一次碰撞近似的PKA能量并用Lindhard分配
  if (pdg == 2112) {
    // 取等效反冲能量（与DPA中同一近似保持一致）
    // 平均A用于近似计算
    G4double Trec = CalculateRecoilEnergy(energy, pdg, Abar);
    if (Trec <= 0.) return 0.;
    G4double f = LindhardFraction(Trec, Zbar, Abar); // 转为非电离的能量份额
    return f * Trec;
  }

  // γ：通过次级电子引入的非电离通常较小，这里给极小近似
  if (pdg == 22) {
    return 1.0e-4 * MeV * (dx / (1.*mm));
  }
  return 0.;
}

void SteppingAction::ComputeAverageZA(const G4Material* material, G4double& Zbar, G4double& Abar) const
{
  const G4ElementVector* elements = material->GetElementVector();
  const G4double* fractions = material->GetFractionVector();
  G4int n = material->GetNumberOfElements();
  G4double Zsum = 0., Asum = 0.;
  for (G4int i = 0; i < n; ++i) {
    Zsum += (*elements)[i]->GetZ() * fractions[i];
    Asum += (*elements)[i]->GetA() * fractions[i];
  }
  if (Zsum > 0.) Zbar = Zsum; else Zbar = 10.;
  if (Asum > 0.) Abar = Asum; else Abar = 20.;
}

// 简化的Lindhard分配函数（常用近似：k*g(e)形式，这里用单调近似）
G4double SteppingAction::LindhardFraction(G4double T, G4double Zbar, G4double Abar) const
{
  // 归一化能量尺度和经验系数（为保持稳定性取常见近似）
  // f_L(T) ~ c * T^(m) / (1 + b*T^(m))，保证0..1范围
  const G4double c = 0.3;
  const G4double b = 0.1 / MeV;
  const G4double m = 0.5; // 次方根形状
  G4double x = std::pow(std::max(T, 0.*MeV)/MeV, m);
  G4double f = (c * x) / (1.0 + b * x);
  if (f < 0.) f = 0.;
  if (f > 1.) f = 1.;
  return f;
}

}  // namespace B1