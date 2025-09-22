// (2) 玻璃厚度-屏蔽效率关系图
// 屏蔽效率 = (1 - 透射计数/入射计数) × 100%
// 需要在不同厚度条件下生成的数据（可通过多个输出文件或在一文件中不同标签）。
// 这里示例支持批量读取 data/* 目录中包含 thicknessXXcm 关键字的文件名进行扫描。
void gamma_thickness_efficiency()
{
  gStyle->SetOptStat(0);
  TGraph* gr = new TGraph();
  gr->SetTitle("Gamma Shielding Efficiency vs Thickness;Glass Thickness (cm);Shielding Efficiency (%)");

  // 扫描 data 子目录
  TSystemDirectory dataDir("data", "data");
  TList* dirs = dataDir.GetListOfFiles();
  if (!dirs) { printf("[WARN] 未找到 data 目录或为空\n"); }
  int idx = 0;
  if (dirs) {
    TIter it(dirs);
    TObject* obj;
    while ((obj = it())) {
      TString name = obj->GetName();
      if (!obj->IsA()->InheritsFrom("TSystemFile")) continue;
      if (name == "." || name == "..") continue;
      if (!name.Contains("thickness")) continue; // 目录名需包含 thicknessX.Xcm

      // 解析厚度
      double thickness_cm = 0.0;
      Ssiz_t p = name.Index("thickness");
      if (p != kNPOS) {
        TString sub = name(p + 9, name.Length() - (p + 9));
        sub.ReplaceAll("cm", "");
        sub.ReplaceAll("_", "."); // 容错
        thickness_cm = sub.Atof();
      }

      TString filepath = TString::Format("data/%s/scintillator_output.root", name.Data());
      TFile* f = TFile::Open(filepath);
      if (!f || f->IsZombie()) continue;
      TH1D* hInc = (TH1D*)f->Get("Gamma_Incident_E");
      TH1D* hTrans = (TH1D*)f->Get("Gamma_Transmit_E");
      if (!hInc || !hTrans) { f->Close(); continue; }
      double inc = hInc->GetEntries();
      double trans = hTrans->GetEntries();
      if (inc > 0) {
        double eff = (1.0 - trans / inc) * 100.0;
        gr->SetPoint(idx++, thickness_cm, eff);
      }
      f->Close();
    }
  }

  TCanvas* c = new TCanvas("c_gamma_thickness", "Gamma Shielding vs Thickness", 1000, 700);
  c->SetGrid();
  gr->SetMarkerStyle(20);
  gr->SetMarkerColor(kRed+1);
  gr->SetLineColor(kRed+1);
  gr->SetLineWidth(2);
  gr->Draw("APL");

  c->SaveAs("gamma_thickness_efficiency.png");
  c->SaveAs("gamma_thickness_efficiency.pdf");
}


