#ifndef CustomPhysicsList_h
#define CustomPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class CustomPhysicsList: public G4VModularPhysicsList
{
public:
  CustomPhysicsList();
  virtual ~CustomPhysicsList();

public:
  virtual void SetCuts();
  
  // 设置电磁物理选项
  void SetEMPhysicsOption(G4int option);
  
private:
  G4int fEMPhysicsOption; // 0=Standard_option4, 1=Livermore, 2=LowEP
  
  // 截断值
  G4double cutForGamma;
  G4double cutForElectron;
  G4double cutForPositron;
  G4double cutForProton;
  G4double cutForAlpha;
  G4double cutForGenericIon;
};

#endif