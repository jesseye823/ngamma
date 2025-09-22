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
/// \file B1/include/SteppingAction.hh
/// \brief Definition of the B1::SteppingAction class

#ifndef B1SteppingAction_h
#define B1SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"  // for G4double/G4int

class G4Material;
class G4LogicalVolume;
class G4Step;

namespace B1
{

class EventAction;

/// Stepping action class

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(EventAction* eventAction);
    ~SteppingAction() override = default;

    // method from the base class
    void UserSteppingAction(const G4Step*) override;

  private:
    EventAction* fEventAction = nullptr;
    G4LogicalVolume* fScoringVolume = nullptr;
    
    // DPA计算函数
    G4double CalculateDPA(const G4Step* step);  // 主函数，根据配置选择模型
    G4double CalculateNRT_DPA(const G4Step* step);  // NRT模型（默认）
    G4double CalculateSRIM_DPA(const G4Step* step);  // SRIM模型（高精度）
    G4double GetDisplacementThreshold(const G4Material* material);
    G4double CalculateRecoilEnergy(G4double kineticEnergy, G4int pdgCode, G4double atomicWeight);
    
    // SRIM模型辅助函数
    G4double CalculateNuclearStoppingPower(G4double energy, G4int pdgCode, const G4Material* material);
    G4double CalculateElectronicStoppingPower(G4double energy, G4int pdgCode, const G4Material* material);
    G4double GetSRIMDisplacementThreshold(const G4Material* material);

    // NIEL（非电离能量损失）完整版
    G4double CalculateNIEL(const G4Step* step);
    void ComputeAverageZA(const G4Material* material, G4double& Zbar, G4double& Abar) const;
    G4double LindhardFraction(G4double recoilEnergy, G4double Zbar, G4double Abar) const;
};

}  // namespace B1

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
