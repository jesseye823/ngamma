// (1) 中子能量-透射率关系图
void neutron_energy_transmission()
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  TH1D* hInc = (TH1D*)f->Get("Neutron_Incident_E");
  TH1D* hTrans = (TH1D*)f->Get("Neutron_Transmit_E");
  if (!hInc || !hTrans) { printf("[ERROR] 缺少 Neutron_Incident_E 或 Neutron_Transmit_E 直方图\n"); return; }

  TH1D* hTr = (TH1D*)hTrans->Clone("Neutron_Transmission_Rate");
  hTr->SetTitle("Neutron Energy vs Transmission;Energy (MeV);Transmission (%)");
  hTr->Divide(hInc);
  for (int i = 1; i <= hTr->GetNbinsX(); ++i) hTr->SetBinContent(i, 100.0 * hTr->GetBinContent(i));

  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_neutron_transmission", "Neutron Energy vs Transmission", 1000, 700);
  c->SetGrid();
  gPad->SetLogx();
  hTr->SetMinimum(0);
  hTr->SetMaximum(100);
  hTr->SetLineColor(kGreen+2);
  hTr->SetLineWidth(2);
  hTr->Draw("HIST");

  c->SaveAs("neutron_energy_transmission.png");
  c->SaveAs("neutron_energy_transmission.pdf");
}


