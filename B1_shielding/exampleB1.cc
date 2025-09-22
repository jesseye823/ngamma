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
/// \file exampleB1.cc
/// \brief Main program of the B1 example

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "CustomPhysicsList.hh"
#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
// #include "Randomize.hh"

using namespace B1;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  // Optionally: choose a different Random engine...
  // G4Random::setTheEngine(new CLHEP::MTwistEngine);

  // use G4SteppingVerboseWithUnits
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  // Construct the run manager (force single thread for ROOT compatibility)
  //
  auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

  // Set mandatory initialization classes
  //
  // Detector construction
  runManager->SetUserInitialization(new DetectorConstruction());

  // Physics list
  {
    // 读取环境变量 EM_PHYSICS_OPTION；未设置则默认使用 0 (Standard_option4)
    const char* envEMPhys = std::getenv("EM_PHYSICS_OPTION");
    G4int emPhysicsOption = envEMPhys ? std::atoi(envEMPhys) : 0;
    
    auto customPhysList = new CustomPhysicsList();
    customPhysList->SetEMPhysicsOption(emPhysicsOption);
    customPhysList->SetVerboseLevel(1);
    runManager->SetUserInitialization(customPhysList);
    
    G4cout << "=== Physics List Configuration ===" << G4endl;
    G4cout << "EM Physics Option: " << emPhysicsOption << G4endl;
    G4cout << "0 = G4EmStandardPhysics_option4 (default)" << G4endl;
    G4cout << "1 = G4EmLivermorePhysics" << G4endl;
    G4cout << "2 = G4EmLowEPPhysics (recommended for low energy studies)" << G4endl;
    G4cout << "=================================" << G4endl;
  }

  // User action initialization
  runManager->SetUserInitialization(new ActionInitialization());

  // Initialize visualization with the default graphics system
  auto visManager = new G4VisExecutive(argc, argv);
  // Constructors can also take optional arguments:
  // - a graphics system of choice, eg. "OGL"
  // - and a verbosity argument - see /vis/verbose guidance.
  // auto visManager = new G4VisExecutive(argc, argv, "OGL", "Quiet");
  // auto visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  auto UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //
  if (!ui) {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }
  else {
    // interactive mode
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
