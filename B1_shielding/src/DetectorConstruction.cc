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
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4UserLimits.hh"
#include "G4UImanager.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4cout << "DetectorConstruction::Construct() called" << G4endl;
  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // 使用Geant4内置的玻璃材料，避免自定义材料的range转换问题
  G4Material* shieldingGlass = nist->FindOrBuildMaterial("G4_GLASS_PLATE");

  // 世界体积参数（缩小以减少真空体积）
  G4double world_sizeXY = 50 * cm;
  G4double world_sizeZ = 50 * cm;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_Galactic");  // 使用真空

  // 75mm厚度屏蔽玻璃体积参数
  G4double glass_sizeXY = 20 * cm;
  G4double glass_sizeZ = 7.5 * cm;  // 75mm厚度

  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;

  //
  // World
  //
  auto solidWorld = new G4Box("World",  // its name
                              0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);  // its size

  auto logicWorld = new G4LogicalVolume(solidWorld,  // its solid
                                       world_mat,  // its material
                                       "World");  // its name

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
                                     G4ThreeVector(),  // at (0,0,0)
                                     logicWorld,  // its logical volume
                                     "World",  // its name
                                     nullptr,  // its mother  volume
                                     false,  // no boolean operation
                                     0,  // copy number
                                     checkOverlaps);  // overlaps checking

  //
  // 75mm厚度高性能屏蔽玻璃
  //
  auto solidGlass = new G4Box("ShieldingGlass",  // its name
                              0.5 * glass_sizeXY, 0.5 * glass_sizeXY, 0.5 * glass_sizeZ);  // its size

  auto logicGlass = new G4LogicalVolume(solidGlass,  // its solid
                                        shieldingGlass,  // its material
                                        "ShieldingGlass");  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    G4ThreeVector(),  // at (0,0,0)
                    logicGlass,  // its logical volume
                    "ShieldingGlass",  // its name
                    logicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking

  // 可选：为深度谱分析设置UserLimits
  // 默认启用，可根据需要注释掉
  G4double binWidth = 100.*um;  // 深度谱bin宽度
  G4double maxStep = 0.5 * binWidth;
  logicGlass->SetUserLimits(new G4UserLimits(maxStep));
  G4cout << "Depth resolution mode: maxStep=" << maxStep/um << " µm for binWidth=" << binWidth/um << " µm" << G4endl;

  // 为玻璃定义区域级ProductionCuts（在玻璃内维持0.01 mm的高分辨率cut）
  // 这样在真空世界中仍使用较大的全局cut，避免初始化时的能量-程程转换异常
  {
    G4Region* glassRegion = new G4Region("GlassRegion");
    logicGlass->SetRegion(glassRegion);
    glassRegion->AddRootLogicalVolume(logicGlass);

    auto glassCuts = new G4ProductionCuts();
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("gamma"));
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("e-"));
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("e+"));
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("proton"));
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("alpha"));
    glassCuts->SetProductionCut(0.01*mm, G4ProductionCuts::GetIndex("GenericIon"));
    glassRegion->SetProductionCuts(glassCuts);

    G4cout << "GlassRegion production cuts set to 0.01 mm for gamma/e-/e+/proton/alpha/genericIon" << G4endl;
  }

  // Set Glass as scoring volume
  fScoringVolume = logicGlass;

  G4cout << "75mm厚度高性能屏蔽玻璃几何结构构建成功" << G4endl;

  //
  // always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Material* DetectorConstruction::DefineShieldingGlass()
{
  G4NistManager* nist = G4NistManager::Instance();

  // 定义元素
  G4String name, symbol;
  G4double a, z;

  // 基础元素定义
  a = 16.00*g/mole;
  G4Element* elO  = new G4Element(name="Oxygen"  ,symbol="O" , z= 8., a);
  a = 28.09*g/mole;
  G4Element* elSi = new G4Element(name="Silicon", symbol="Si", z=14., a);
  a = 22.99*g/mole;
  G4Element* elNa = new G4Element(name="Sodium", symbol="Na", z=11., a);
  a = 30.10*g/mole;
  G4Element* elK = new G4Element(name="Potassium", symbol="K", z=19., a);
  a = 6.941*g/mole;
  G4Element* elLi = new G4Element(name="Lithium", symbol="Li", z=3., a);
  a = 65.39*g/mole;
  G4Element* elZn = new G4Element(name="Zinc", symbol="Zn", z=30., a);
  a = 26.98*g/mole;
  G4Element* elAl = new G4Element(name="Aluminum", symbol="Al", z=13., a);
  a = 140.12*g/mole;
  G4Element* elCe = new G4Element(name="Cerium", symbol="Ce", z=58., a);
  a = 10.81*g/mole;
  G4Element* elB = new G4Element(name="Boron", symbol="B", z=5., a);
  a = 207.20*g/mole;
  G4Element* elPb = new G4Element(name="Lead",symbol="Pb", z=82., a);
  a = 157.25*g/mole;
  G4Element* elGd = new G4Element(name="Gadolinium",symbol="Gd", z=64., a);
  a = 24.31*g/mole;
  G4Element* elMg = new G4Element(name="Magnesium",symbol="Mg", z=12., a);

  // 定义氧化物材料
  G4double density;
  
  density = 2.200*g/cm3;
  G4Material* SiO2 = new G4Material(name="quartz", density, 2);
  SiO2->AddElement(elSi, 1);
  SiO2->AddElement(elO , 2);
  
  density = 2.270*g/cm3;
  G4Material* Na2O = new G4Material(name="na2o", density, 2);
  Na2O->AddElement(elNa, 2);
  Na2O->AddElement(elO , 1);
  
  density = 2.3*g/cm3;
  G4Material* K2O = new G4Material(name="k2o", density, 2);
  K2O->AddElement(elK, 2);
  K2O->AddElement(elO , 1);
  
  density = 5.6*g/cm3;
  G4Material* ZnO = new G4Material(name="zno", density, 2);
  ZnO->AddElement(elZn, 1);
  ZnO->AddElement(elO , 1);
  
  density = 7.407*g/cm3;
  G4Material* Gd2O3 = new G4Material(name="Gd2O3", density, 2);
  Gd2O3->AddElement(elGd, 2);
  Gd2O3->AddElement(elO , 3);
  
  density = 3.970*g/cm3;
  G4Material* Al2O3 = new G4Material(name="al2o3", density, 2);
  Al2O3->AddElement(elAl, 2);
  Al2O3->AddElement(elO , 3);
  
  density = 2.013*g/cm3;
  G4Material* Li2O = new G4Material(name="li2o", density, 2);
  Li2O->AddElement(elLi, 2);
  Li2O->AddElement(elO , 1);
  
  density = 7.200*g/cm3;
  G4Material* CeO2 = new G4Material(name="ceO2", density, 2);
  CeO2->AddElement(elCe, 1);
  CeO2->AddElement(elO , 2);
  
  density = 1.840*g/cm3;
  G4Material* B2O3 = new G4Material(name="b2O3", density, 2);
  B2O3->AddElement(elB, 2);
  B2O3->AddElement(elO , 3);
  
  density = 9.530*g/cm3;
  G4Material* PbO = new G4Material(name="pbO", density, 2);
  PbO->AddElement(elPb, 1);
  PbO->AddElement(elO , 1);

  density = 3.58*g/cm3;
  G4Material* MgO = new G4Material(name="mgO", density, 2);
  MgO->AddElement(elMg, 1);
  MgO->AddElement(elO , 1);


  // 创建高性能屏蔽玻璃材料
  density = 2.460*g/cm3;  // 优化密度用于屏蔽
  G4Material* glassMaterial = new G4Material(name="ShieldingGlass", density, 8);
  
  // 材料配比
  glassMaterial->AddMaterial(B2O3, 60*perCent);
  //glassMaterial->AddMaterial(SiO2, 60*perCent);      // 基础玻璃基质
  glassMaterial->AddMaterial(MgO, 4*perCent);    
  glassMaterial->AddMaterial(Al2O3, 8*perCent);  
  glassMaterial->AddMaterial(CeO2, 5*perCent);  
  glassMaterial->AddMaterial(B2O3, 60*perCent);     
  glassMaterial->AddMaterial(Gd2O3, 5*perCent);    
  //glassMaterial->AddMaterial(ZnO, 6*perCent);       
  glassMaterial->AddMaterial(Li2O, 18*perCent);      
  //glassMaterial->AddMaterial(PbO, 2*perCent);      

  G4cout << "Using Geant4 built-in G4_GLASS_PLATE material" << G4endl;

  return glassMaterial;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1