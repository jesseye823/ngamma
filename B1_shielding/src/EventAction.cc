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
// * use.  Please see the license in the file  LICENSE and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree to acknowledge its *
// * use  in  resulting  scientific  publications, and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/src/EventAction.cc
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"

#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep = 0.;
  fNIEL = 0.;
  fDPA = 0.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{
  // accumulate statistics in run action
  fRunAction->AddEdep(fEdep);
  
  // 写入ROOT树
  auto analysis = G4AnalysisManager::Instance();
  if (analysis) {
    // 写入PhysicsData树
    analysis->FillNtupleIColumn(0, 0, event->GetEventID());
    analysis->FillNtupleDColumn(0, 1, fEdep);
    analysis->FillNtupleDColumn(0, 2, 0.0);  // X位置（暂时设为0）
    analysis->FillNtupleDColumn(0, 3, 0.0);  // Y位置（暂时设为0）
    analysis->FillNtupleDColumn(0, 4, 0.0);  // Z位置（暂时设为0）
    analysis->AddNtupleRow(0);
    
    // 写入Optical树（DPA和NIEL数据）
    analysis->FillNtupleIColumn(2, 0, event->GetEventID());
    analysis->FillNtupleDColumn(2, 1, fDPA);
    analysis->FillNtupleDColumn(2, 2, fNIEL);
    analysis->AddNtupleRow(2);
    
    // 写入直方图
    analysis->FillH1(0, fEdep);  // Edep直方图
    analysis->FillH1(1, fDPA);   // DPA直方图
    analysis->FillH1(2, fNIEL);  // NIEL直方图
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::FillTrack(G4int trackID, G4int parentID, G4int pdgCode, 
                             G4double x, G4double y, G4double z, 
                             G4double kineticEnergy, G4double time, G4int stepNumber)
{
  if (fRunAction) {
    fRunAction->FillTrackData(trackID, parentID, pdgCode, x, y, z, kineticEnergy, time, stepNumber);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
