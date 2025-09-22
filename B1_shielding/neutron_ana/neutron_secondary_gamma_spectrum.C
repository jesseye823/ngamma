// (5) 次级伽马射线能谱分布图（由中子俘获产生）
void neutron_secondary_gamma_spectrum()
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  TH1D* hCapGamma = (TH1D*)f->Get("Capture_Gamma_E");
  if (!hCapGamma) { printf("[ERROR] 缺少 Capture_Gamma_E 直方图\n"); return; }

  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_secondary_gamma", "Secondary Gamma Spectrum from Neutron Capture", 1000, 700);
  c->SetGrid();
  gPad->SetLogx();
  hCapGamma->SetTitle("Secondary Gamma Spectrum (Capture);Gamma Energy (MeV);Counts/MeV");
  hCapGamma->SetLineColor(kCyan+2);
  hCapGamma->SetLineWidth(2);
  hCapGamma->Draw("HIST");

  c->SaveAs("neutron_secondary_gamma_spectrum.png");
  c->SaveAs("neutron_secondary_gamma_spectrum.pdf");
}


