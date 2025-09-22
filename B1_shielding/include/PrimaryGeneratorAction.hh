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
/// \file B1/include/PrimaryGeneratorAction.hh
/// \brief Definition of the B1::PrimaryGeneratorAction class

#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include <vector>
#include "G4Types.hh"
#include "G4String.hh"

class G4ParticleGun;
class G4GeneralParticleSource;
class G4GenericMessenger;
class G4Event;
class G4Box;

namespace B1
{

/// Primary generator with built-in Cf-252 Watt spectrum rectangular surface source.

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    // method from the base class
    void GeneratePrimaries(G4Event*) override;

    // UI setter
    void SetMode(const G4String& mode);

  private:
    // Internal helpers
    void initializeCf252Spectrum();
    G4double sampleCf252EnergyMeV() const;

    // Mode switch: cf252 (built-in) or gps (macro-controlled)
    enum class SourceMode { CF252, GPS };
    SourceMode fMode;

    // Generators
    G4ParticleGun* fParticleGun = nullptr;  // simple particle gun
    G4GeneralParticleSource* fGPS = nullptr; // optional GPS

    // UI
    G4GenericMessenger* fMessenger = nullptr;

    // Cf-252 Watt spectrum parameters and lookup tables
    G4double fWattA_MeV;         // a parameter in MeV
    G4double fWattB_perMeV;      // b parameter in 1/MeV
    std::vector<G4double> fEgridMeV;   // energy grid (MeV)
    std::vector<G4double> fCdf;        // normalized CDF on grid

    // Rectangular surface source geometry
    G4double fHalfX;             // half width (cm)
    G4double fHalfY;             // half height (cm)
    G4double fSourceZ;           // Z position (cm)
};

}  // namespace B1

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
