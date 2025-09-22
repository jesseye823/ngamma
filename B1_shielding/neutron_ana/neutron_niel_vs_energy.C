// (4) NIEL-中子能量关系图
// 需要逐能量的NIEL，当前ROOT中提供的是总NIEL直方图（累计）。
// 这里以 Neutron_Incident_E 为X轴，用 NIEL 总量归一到入射计数，给出“平均NIEL”随能量的近似：
// 注意：这是近似占位，如需精确需在模拟中按步或按能量分bin记录NIEL。
void neutron_niel_vs_energy()
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  TH1D* hInc = (TH1D*)f->Get("Neutron_Incident_E");
  TH1D* hNIEL = (TH1D*)f->Get("NIEL");
  if (!hInc || !hNIEL) { printf("[ERROR] 缺少 Neutron_Incident_E 或 NIEL 直方图\n"); return; }

  // 近似：每个能量bin的平均NIEL = (总NIEL/入射总数) 常值线；仅作占位显示。
  double avgNIEL = (hInc->GetEntries() > 0) ? (hNIEL->GetMean()) : 0.0;

  // 用入射能量轴构造常值曲线
  TGraph* gr = new TGraph();
  for (int i = 1; i <= hInc->GetNbinsX(); ++i) {
    double E = hInc->GetXaxis()->GetBinCenter(i);
    gr->SetPoint(i-1, E, avgNIEL);
  }

  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_neutron_niel", "NIEL vs Neutron Energy (approx)", 1000, 700);
  c->SetGrid();
  gPad->SetLogx();
  gr->SetTitle("NIEL vs Neutron Energy (approx);Energy (MeV);NIEL (MeV·cm^{2}/g)");
  gr->SetLineColor(kOrange+2);
  gr->SetLineWidth(2);
  gr->Draw("AL");

  c->SaveAs("neutron_niel_vs_energy.png");
  c->SaveAs("neutron_niel_vs_energy.pdf");
}


