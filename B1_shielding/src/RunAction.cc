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
/// \file B1/src/RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4ParticleGun.hh"
#include "G4GeneralParticleSource.hh"
#include "G4SPSEneDistribution.hh"
#include "TTree.h"
#include "TFile.h"
#include <filesystem>
#include <ctime>
#include <sstream>

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction() : fTrackTree(nullptr)
{
  G4cout << "RunAction constructor called" << G4endl;
  
  // add new units for dose
  const G4double milligray = 1.e-3 * gray;
  const G4double microgray = 1.e-6 * gray;
  const G4double nanogray = 1.e-9 * gray;
  const G4double picogray = 1.e-12 * gray;

  new G4UnitDefinition("milligray", "milliGy", "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy", "Dose", microgray);
  new G4UnitDefinition("nanogray", "nanoGy", "Dose", nanogray);
  new G4UnitDefinition("picogray", "picoGy", "Dose", picogray);

  // Register accumulable to the accumulable manager
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Register(fEdep);
  accumulableManager->Register(fEdep2);
  
  // 获取分析管理器
  G4cout << "Attempting to get G4AnalysisManager instance..." << G4endl;
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (!analysisManager) {
    G4cerr << "ERROR: Failed to get G4AnalysisManager instance!" << G4endl;
    return;
  }
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);
  G4cout << "G4AnalysisManager initialized successfully" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* run)
{
  G4cout << "=== BeginOfRunAction: Starting run with " 
         << run->GetNumberOfEventToBeProcessed() << " events ===" << G4endl;
  
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();

  // 只在master线程中设置ROOT文件
  if (IsMaster()) {
    // 创建输出文件
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    // 生成包含源、事件数、时间戳的信息化输出路径，写到与build同级的data目录
    G4String particle = "unknown";
    G4String energyTag = "unknownE";
    if (auto pga = dynamic_cast<const PrimaryGeneratorAction*>(G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction())) {
      // 我们的主发射器固定是中子 + Watt谱
      particle = "neutron";
      energyTag = "Cf252_Watt";
    }
    auto now = std::time(nullptr);
    std::tm tm{};
    #ifdef _WIN32
      localtime_s(&tm, &now);
    #else
      localtime_r(&now, &tm);
    #endif
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tm);
    // 目录名顺序：粒子类型_能量_事件数_时间
    std::string folder = std::string(particle) + std::string("_") + energyTag +
                         std::string("_") + std::to_string(run->GetNumberOfEventToBeProcessed()) + std::string("ev_") + std::string(ts);
    // 相对build目录，写入到上级的data子目录
    std::filesystem::path outDir = std::filesystem::path("..") / "data" / folder;
    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);
    if (ec) {
      G4cerr << "WARNING: Failed to create output directory: " << outDir.string() << G4endl;
    }
    std::filesystem::path outFile = outDir / "scintillator_output.root";
    G4String fileName = outFile.string();
    G4cout << "Creating ROOT file: " << fileName << G4endl;
    
    try {
      analysisManager->OpenFile(fileName);
      
      // 创建直方图
      analysisManager->CreateH1("Edep", "Energy Deposition", 100, 0., 10.*MeV);
      analysisManager->CreateH1("DPA", "Displacements Per Atom", 100, 0., 1.0);
      analysisManager->CreateH1("NIEL", "Non-Ionizing Energy Loss", 100, 0., 1.*MeV);
      // 透射与俘获诊断
      analysisManager->CreateH1("Gamma_Transmit_E", "Gamma Transmission Energy", 200, 0., 10.*MeV);
      analysisManager->CreateH1("Neutron_Transmit_E", "Neutron Transmission Energy", 200, 0., 20.*MeV);
      analysisManager->CreateH1("Neutron_Capture_E", "Neutron Capture Energy (neutron pre-capture)", 200, 0., 20.*MeV);
      analysisManager->CreateH1("Capture_Gamma_E", "Capture Gamma Energy", 400, 0., 10.*MeV);
      analysisManager->CreateH1("Gamma_Incident_E", "Gamma Incident Energy", 200, 0., 10.*MeV);
      analysisManager->CreateH1("Neutron_Incident_E", "Neutron Incident Energy", 200, 0., 20.*MeV);
      analysisManager->CreateH1("Capture_Count", "Neutron Capture Count (per run)", 10, 0., 10.);
 
      // 创建Ntuple
      analysisManager->CreateNtuple("PhysicsData", "Physics Quantities");
      analysisManager->CreateNtupleIColumn("EventID");
      analysisManager->CreateNtupleDColumn("Edep");
      analysisManager->CreateNtupleDColumn("X");
      analysisManager->CreateNtupleDColumn("Y");
      analysisManager->CreateNtupleDColumn("Z");
      analysisManager->FinishNtuple();
      // ActivationProducts（简表）
      analysisManager->CreateNtuple("ActivationProducts", "Capture simplified table");
      analysisManager->CreateNtupleDColumn("PreNeutronE");
      analysisManager->CreateNtupleDColumn("CaptureGammaE");
      analysisManager->CreateNtupleDColumn("X");
      analysisManager->CreateNtupleDColumn("Y");
      analysisManager->CreateNtupleDColumn("Z");
      analysisManager->FinishNtuple();
      // Damage类量（与光学无关）：DPA/NIEL
      analysisManager->CreateNtuple("Damage", "Damage quantities (non-optical): DPA, NIEL");
      analysisManager->CreateNtupleIColumn("EventID");
      analysisManager->CreateNtupleDColumn("DPA");
      analysisManager->CreateNtupleDColumn("NIEL");
      analysisManager->FinishNtuple();
      
      // 通过G4AnalysisManager创建轨迹数据的Ntuple
      analysisManager->CreateNtuple("TrackData", "Particle Track Information");
      analysisManager->CreateNtupleIColumn("TrackID");
      analysisManager->CreateNtupleIColumn("ParentID");
      analysisManager->CreateNtupleIColumn("PDGCode");
      analysisManager->CreateNtupleDColumn("X");
      analysisManager->CreateNtupleDColumn("Y");
      analysisManager->CreateNtupleDColumn("Z");
      analysisManager->CreateNtupleDColumn("KineticEnergy");
      analysisManager->CreateNtupleDColumn("Time");
      analysisManager->CreateNtupleIColumn("StepNumber");
      analysisManager->FinishNtuple();
      
      // 不再需要手动创建TTree
      fTrackTree = nullptr;
      
      G4cout << "Analysis setup completed (including TrackData TTree)" << G4endl;
    } catch (...) {
      G4cerr << "ERROR: Exception during ROOT analysis setup!" << G4endl;
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4cout << "=== EndOfRunAction: Processing run results ===" << G4endl;
  
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Compute dose = total energy deposit in a run and its variance
  G4double edep = fEdep.GetValue();
  G4double edep2 = fEdep2.GetValue();

  G4double rms = edep2 - edep * edep / nofEvents;
  if (rms > 0.) rms = std::sqrt(rms);
  else rms = 0.;

  const DetectorConstruction* detConstruction
    = static_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4double mass = detConstruction->GetScoringVolume()->GetMass();
  G4double dose = edep / mass;
  G4double rmsDose = rms / mass;

  // Run conditions
  G4String runCondition;  // 可按需填充（GPS场景不强制打印粒子信息）

  // Print
  if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------" << G4endl
     << " The run consists of " << nofEvents << " " << runCondition << G4endl
     << " Cumulated dose per run, in scoring volume : "
     << G4BestUnit(dose, "Dose") << " rms = " << G4BestUnit(rmsDose, "Dose")
     << G4endl
     << "------------------------------------------------------------" << G4endl
     << G4endl;
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------" << G4endl
     << " The run consists of " << nofEvents << " " << runCondition << G4endl
     << " Cumulated dose per run, in scoring volume : "
     << G4BestUnit(dose, "Dose") << " rms = " << G4BestUnit(rmsDose, "Dose")
     << G4endl
     << "------------------------------------------------------------" << G4endl
     << G4endl;
  }
  
  // 只在master线程写入文件
  if (IsMaster()) {
    try {
      G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
      G4cout << "Writing ROOT file..." << G4endl;
      
      analysisManager->Write();
      
      G4String closed = analysisManager->GetFileName();
      analysisManager->CloseFile();
      G4cout << "Analysis results written to " << closed << G4endl;
    } catch (...) {
      G4cerr << "ERROR: Exception during ROOT file writing!" << G4endl;
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::AddEdep(G4double edep)
{
  fEdep += edep;
  fEdep2 += edep * edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::FillTrackData(G4int trackID, G4int parentID, G4int pdgCode, 
                              G4double x, G4double y, G4double z, 
                              G4double kineticEnergy, G4double time, G4int stepNumber)
{
  // 使用G4AnalysisManager填充轨迹数据（Ntuple ID = 3）
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager) {
    analysisManager->FillNtupleIColumn(3, 0, trackID);
    analysisManager->FillNtupleIColumn(3, 1, parentID);
    analysisManager->FillNtupleIColumn(3, 2, pdgCode);
    analysisManager->FillNtupleDColumn(3, 3, x);
    analysisManager->FillNtupleDColumn(3, 4, y);
    analysisManager->FillNtupleDColumn(3, 5, z);
    analysisManager->FillNtupleDColumn(3, 6, kineticEnergy);
    analysisManager->FillNtupleDColumn(3, 7, time);
    analysisManager->FillNtupleIColumn(3, 8, stepNumber);
    analysisManager->AddNtupleRow(3);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1