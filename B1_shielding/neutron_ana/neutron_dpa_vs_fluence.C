// (3) DPA-中子注量关系图
// 需要中子注量 (n/cm^2) 估计：注量 ~ 入射计数 / 面积。此处以计数作相对注量，或由用户提供有效面积后换算。
void neutron_dpa_vs_fluence(double effective_area_cm2 = 1.0)
{
  TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
  TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
  TFile* f = TFile::Open(filepath);
  if (!f || f->IsZombie()) { printf("[ERROR] 打不开ROOT文件: %s\n", filepath.Data()); return; }

  TH1D* hDPA = (TH1D*)f->Get("DPA");
  TH1D* hInc = (TH1D*)f->Get("Neutron_Incident_E");
  if (!hDPA || !hInc) { printf("[ERROR] 缺少 DPA 或 Neutron_Incident_E 直方图\n"); return; }

  double incident_counts = hInc->GetEntries();
  double fluence = (effective_area_cm2 > 0) ? incident_counts / effective_area_cm2 : incident_counts; // n/cm2（相对）
  double dpa_mean = hDPA->GetMean();

  // 画点：本次运行的 (fluence, DPA)
  gStyle->SetOptStat(0);
  TCanvas* c = new TCanvas("c_neutron_dpa_fluence", "DPA vs Neutron Fluence", 900, 650);
  c->SetGrid();
  TGraph* gr = new TGraph(1);
  gr->SetTitle("DPA vs Neutron Fluence;Neutron Fluence (n/cm^{2});DPA (dpa)");
  gr->SetPoint(0, fluence, dpa_mean);
  gr->SetMarkerStyle(20);
  gr->SetMarkerColor(kBlue+2);
  gr->SetMarkerSize(1.2);
  gr->Draw("AP");

  c->SaveAs("neutron_dpa_vs_fluence.png");
  c->SaveAs("neutron_dpa_vs_fluence.pdf");
}


