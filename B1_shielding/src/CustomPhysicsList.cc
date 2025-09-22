#include "CustomPhysicsList.hh"

#include "G4SystemOfUnits.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLowEPPhysics.hh"

#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"

#include "G4HadronElasticPhysics.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4IonBinaryCascadePhysics.hh"
#include "G4NeutronTrackingCut.hh"

#include "G4LossTableManager.hh"
#include "G4UnitsTable.hh"
#include "G4ProcessManager.hh"
#include "G4IonFluctuations.hh"
#include "G4IonParametrisedLossModel.hh"
#include "G4EmProcessSubType.hh"

#include "G4EmExtraPhysics.hh"
#include "G4EmParameters.hh"
#include "G4StoppingPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4IonElasticPhysics.hh"

#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4HadronPhysicsQGSP_BIC.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4HadronPhysicsINCLXX.hh"

#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

CustomPhysicsList::CustomPhysicsList()
  : G4VModularPhysicsList(),
    fEMPhysicsOption(0)
{
  G4LossTableManager::Instance();
  
  // 全局默认cut设置为0.01 mm（10微米）
  defaultCutValue = 0.01*mm;
  cutForGamma     = defaultCutValue;
  cutForElectron  = defaultCutValue;
  cutForPositron  = defaultCutValue;
  cutForProton    = defaultCutValue;
  cutForAlpha     = defaultCutValue;
  cutForGenericIon = defaultCutValue;
  
  G4cout << "Global production cuts set to " << defaultCutValue/mm << " mm" << G4endl;

  // 自动步长设置：基于几何厚度计算finalRange
  G4double glassThickness = 75.*mm;  // 从DetectorConstruction获取的厚度
  G4double finalRange = std::max(5.*um, std::min(glassThickness/1000., 50.*um));
  
  G4EmParameters* em = G4EmParameters::Instance();
  em->SetStepFunctionMuHad(0.03, finalRange);  // 质子/介子/强子：相对步长3%，末端钳制
  em->SetStepFunctionLightIons(0.03, finalRange);  // 轻离子
  em->SetStepFunctionIons(0.03, finalRange);  // 重离子
  em->SetStepFunction(0.2, 0.1*mm);  // e±保持默认
  em->SetNumberOfBinsPerDecade(20);
  em->SetMinEnergy(100*eV);
  
  G4cout << "Auto step settings: dRoverRange=0.03, finalRange=" 
         << finalRange/um << " µm for thickness=" << glassThickness/mm << " mm" << G4endl;

  SetVerboseLevel(1);

  // 默认使用Standard_option4
  RegisterPhysics(new G4EmStandardPhysics_option4());
  
  // 其他物理过程
  RegisterPhysics(new G4EmExtraPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  // 强子物理（启用HP以支持亚MeV中子）
  // 采用QGSP_BIC_HP联用弹性HP，确保热/慢中子截面库生效
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4HadronPhysicsQGSP_BIC_HP());
  RegisterPhysics(new G4IonBinaryCascadePhysics());
  RegisterPhysics(new G4NeutronTrackingCut());

  // 停止和离子物理
  RegisterPhysics(new G4StoppingPhysics());
  RegisterPhysics(new G4IonPhysics());
  RegisterPhysics(new G4IonElasticPhysics());
}

CustomPhysicsList::~CustomPhysicsList()
{}

void CustomPhysicsList::SetEMPhysicsOption(G4int option)
{
  fEMPhysicsOption = option;
  
  // 移除现有的电磁物理
  RemovePhysics("G4EmStandardPhysics_option4");
  RemovePhysics("G4EmLivermorePhysics");
  RemovePhysics("G4EmLowEPPhysics");
  
  // 根据选项添加新的电磁物理
  switch(option) {
    case 0:
      RegisterPhysics(new G4EmStandardPhysics_option4());
      G4cout << "Using G4EmStandardPhysics_option4" << G4endl;
      break;
    case 1:
      RegisterPhysics(new G4EmLivermorePhysics());
      G4cout << "Using G4EmLivermorePhysics" << G4endl;
      break;
    case 2:
      RegisterPhysics(new G4EmLowEPPhysics());
      G4cout << "Using G4EmLowEPPhysics (Low Energy Physics)" << G4endl;
      break;
    default:
      RegisterPhysics(new G4EmStandardPhysics_option4());
      G4cout << "Invalid option, using G4EmStandardPhysics_option4" << G4endl;
      break;
  }
}

void CustomPhysicsList::SetCuts()
{
  if (verboseLevel >0) {
    G4cout << "CustomPhysicsList::SetCuts:";
    G4cout << "CutLength : " << G4BestUnit(defaultCutValue,"Length") << G4endl;
    G4cout << "Enhanced cuts for secondary particle generation:" << G4endl;
    G4cout << "  Gamma cut: " << G4BestUnit(cutForGamma,"Length") << G4endl;
    G4cout << "  Electron cut: " << G4BestUnit(cutForElectron,"Length") << G4endl;
    G4cout << "  Positron cut: " << G4BestUnit(cutForPositron,"Length") << G4endl;
    G4cout << "  Proton cut: " << G4BestUnit(cutForProton,"Length") << G4endl;
  }

  // 设置全局截断值
  SetCutValue(cutForGamma, "gamma");
  SetCutValue(cutForElectron, "e-");
  SetCutValue(cutForPositron, "e+");
  SetCutValue(cutForProton, "proton");

  if (verboseLevel>0) DumpCutValuesTable();
}