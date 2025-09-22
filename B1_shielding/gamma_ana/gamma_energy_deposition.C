// (4) 伽马射线能量-能量沉积分布图
// 从 ROOT 文件中读取 Gamma_Incident_E 与 Edep，绘制二维分布或相关图。
void gamma_energy_deposition()
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  // 入射能谱（伽马）与能量沉积直方图
  TH1D* hInc = (TH1D*)f->Get("Gamma_Incident_E");
  TH1D* hEdep = (TH1D*)f->Get("Edep");
  if (!hInc || !hEdep) { printf("[ERROR] 缺少 Gamma_Incident_E 或 Edep 直方图\n"); return; }

  // 若需要二维关联，需原始逐事件数据。这里用一维对比展示：上：入射能谱；下：能量沉积分布
  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_gamma_edep", "Gamma Energy vs Energy Deposition", 1100, 800);
  c->Divide(1,2);

  c->cd(1);
  gPad->SetLogx();
  hInc->SetTitle("Gamma Incident Energy;Energy (MeV);Counts");
  hInc->SetLineColor(kBlue+1);
  hInc->SetLineWidth(2);
  hInc->Draw("HIST");

  c->cd(2);
  hEdep->SetTitle("Energy Deposition Distribution;Deposited Energy (MeV);Counts");
  hEdep->SetLineColor(kRed+1);
  hEdep->SetLineWidth(2);
  hEdep->Draw("HIST");

  c->SaveAs("gamma_energy_deposition.png");
  c->SaveAs("gamma_energy_deposition.pdf");
}
