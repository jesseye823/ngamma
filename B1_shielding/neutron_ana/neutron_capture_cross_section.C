// (2) 中子能量-俘获截面关系图（占位：从ROOT中无法直接给出截面，
// 这里用俘获计数与入射计数按能量归一得到相对量，或对接外部截面库后替换）
void neutron_capture_cross_section()
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  TH1D* hInc = (TH1D*)f->Get("Neutron_Incident_E");
  TH1D* hCapE = (TH1D*)f->Get("Neutron_Capture_E"); // 俘获前中子能量分布
  if (!hInc || !hCapE) { printf("[ERROR] 缺少 Neutron_Incident_E 或 Neutron_Capture_E 直方图\n"); return; }

  // 相对“俘获概率” ~ hCapE / hInc；若需物理截面(barn)，应接入外部核数据表。
  TH1D* hRelXS = (TH1D*)hCapE->Clone("Relative_Capture_Cross_Section");
  hRelXS->SetTitle("Neutron Energy vs (Relative) Capture Cross Section;Energy (MeV);Relative Capture (arb.)");
  hRelXS->Divide(hInc);

  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_neutron_capture_xs", "Neutron Energy vs Capture XS (relative)", 1000, 700);
  c->SetGrid();
  gPad->SetLogx();
  gPad->SetLogy();
  hRelXS->SetLineColor(kMagenta+1);
  hRelXS->SetLineWidth(2);
  hRelXS->Draw("HIST");

  c->SaveAs("neutron_capture_cross_section.png");
  c->SaveAs("neutron_capture_cross_section.pdf");
}


