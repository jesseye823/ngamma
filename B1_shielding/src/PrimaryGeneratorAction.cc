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
/// \file B1/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B1::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "G4GeneralParticleSource.hh"
#include "G4GenericMessenger.hh"
#include <algorithm>
#include <cmath>

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  G4cout << "PrimaryGeneratorAction constructor called" << G4endl;

  // 默认模式：CF252（可由UI切换为GPS）
  fMode = SourceMode::CF252;

  // 生成器
  fParticleGun = new G4ParticleGun(1);
  fGPS = new G4GeneralParticleSource();

  // 粒子默认中子
  auto particle = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
  fParticleGun->SetParticleDefinition(particle);

  // 源面矩形参数（单位：cm），位置在z = -10 cm
  fHalfX = 10.0;   // 20 cm 宽的一半
  fHalfY = 10.0;   // 20 cm 高的一半
  fSourceZ = -10.0;

  // Watt谱参数（MeV, 1/MeV）并初始化CDF查找表
  fWattA_MeV = 1.025;
  fWattB_perMeV = 2.926;
  initializeCf252Spectrum();

  // UI: /source/mode cf252|gps
  fMessenger = new G4GenericMessenger(this, "/source/", "Primary source control");
  fMessenger->DeclareMethod("mode", &PrimaryGeneratorAction::SetMode)
            .SetGuidance("Set source mode: cf252 or gps");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  if (fMode == SourceMode::GPS) {
    // 直接交给GPS（由宏配置）
    fGPS->GeneratePrimaryVertex(anEvent);
    return;
  }

  // 1) 位置：矩形面上均匀采样 (单位转换到 mm)
  const G4double x_cm = (2.0*G4UniformRand()-1.0) * fHalfX;
  const G4double y_cm = (2.0*G4UniformRand()-1.0) * fHalfY;
  const G4double z_cm = fSourceZ;
  fParticleGun->SetParticlePosition(G4ThreeVector(x_cm*cm, y_cm*cm, z_cm*cm));

  // 2) 方向：+Z 半空间各向同性
  const G4double u = G4UniformRand();        // cos(theta) in [0,1]
  const G4double cosTheta = u;
  const G4double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta*cosTheta));
  const G4double phi = 2.0*M_PI*G4UniformRand();
  const G4double dx = sinTheta*std::cos(phi);
  const G4double dy = sinTheta*std::sin(phi);
  const G4double dz = cosTheta;               // 指向 +Z 半球
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(dx, dy, dz));

  // 3) 能量：按Watt分布抽样（MeV）
  const G4double eMeV = sampleCf252EnergyMeV();
  fParticleGun->SetParticleEnergy(eMeV*MeV);

  // 发射
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

void PrimaryGeneratorAction::initializeCf252Spectrum()
{
  // 构建能量网格与未归一化PDF：f(E) = exp(-E/a)*sinh(sqrt(b E))
  // 取 0.0 到 12.0 MeV，步长 0.05 MeV，避免0点奇异从1e-6开始
  const G4double eMin = 1e-6;  // MeV
  const G4double eMax = 12.0;  // MeV
  const G4double de   = 0.05;  // MeV

  fEgridMeV.clear();
  fCdf.clear();

  G4double accum = 0.0;
  for (G4double E=eMin; E<=eMax+0.5*de; E+=de) {
    const G4double val = std::exp(-E/fWattA_MeV) * std::sinh(std::sqrt(std::max(0.0, fWattB_perMeV*E)));
    fEgridMeV.push_back(E);
    accum += val;
    fCdf.push_back(accum);
  }
  // 归一化CDF到[0,1]
  if (!fCdf.empty() && fCdf.back()>0) {
    const G4double norm = fCdf.back();
    for (auto &c : fCdf) c /= norm;
  }
}

G4double PrimaryGeneratorAction::sampleCf252EnergyMeV() const
{
  if (fEgridMeV.empty() || fCdf.empty()) return 2.1; // 后备
  const G4double r = G4UniformRand();
  // 在CDF上二分查找
  auto it = std::lower_bound(fCdf.begin(), fCdf.end(), r);
  if (it == fCdf.begin()) return fEgridMeV.front();
  if (it == fCdf.end()) return fEgridMeV.back();
  size_t idx = std::distance(fCdf.begin(), it);
  // 线性内插CDF反演
  const G4double c1 = fCdf[idx-1];
  const G4double c2 = fCdf[idx];
  const G4double e1 = fEgridMeV[idx-1];
  const G4double e2 = fEgridMeV[idx];
  const G4double t = (c2>c1) ? ( (r-c1)/(c2-c1) ) : 0.0;
  return e1 + t*(e2-e1);
}

void PrimaryGeneratorAction::SetMode(const G4String& mode)
{
  if (mode == "gps" || mode == "GPS") {
    fMode = SourceMode::GPS;
    G4cout << "[source] mode = gps (macro-controlled)" << G4endl;
  } else {
    fMode = SourceMode::CF252;
    G4cout << "[source] mode = cf252 (built-in Watt + surface)" << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1