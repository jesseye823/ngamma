#ifndef B1_DetectorMessenger_h
#define B1_DetectorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIcmdWithAString;

namespace B1 {

class DetectorConstruction;

class DetectorMessenger : public G4UImessenger {
public:
  explicit DetectorMessenger(DetectorConstruction* det);
  ~DetectorMessenger() override;

  void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
  DetectorConstruction* fDetector;
  G4UIdirectory* fDetDir;
  G4UIdirectory* fGlassDir;
  G4UIcmdWithAString* fCompositionFileCmd; // /det/glass/compositionFile <path>
};

} // namespace B1

#endif


