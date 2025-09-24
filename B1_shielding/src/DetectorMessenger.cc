#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

namespace B1 {

DetectorMessenger::DetectorMessenger(DetectorConstruction* det)
  : fDetector(det)
{
  fDetDir = new G4UIdirectory("/det/");
  fDetDir->SetGuidance("Detector control commands");

  fGlassDir = new G4UIdirectory("/det/glass/");
  fGlassDir->SetGuidance("Glass composition controls");

  fCompositionFileCmd = new G4UIcmdWithAString("/det/glass/compositionFile", this);
  fCompositionFileCmd->SetGuidance("Set glass composition file path (format: <MaterialName> <percent>) per line");
  fCompositionFileCmd->SetParameterName("filepath", false);
}

DetectorMessenger::~DetectorMessenger()
{
  delete fCompositionFileCmd;
  delete fGlassDir;
  delete fDetDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command == fCompositionFileCmd && fDetector) {
    fDetector->SetGlassCompositionFile(newValue);
  }
}

} // namespace B1


