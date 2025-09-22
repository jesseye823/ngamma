// (3) 元素成分-线性衰减系数关系图
// 说明：该图需要成分百分比与对应的线性衰减系数μ。这里从 docs/ 或外部CSV载入成分，
// 若无外部数据，则提供一个示例数组，可后续替换为真实拟合/数据库结果。
void gamma_composition_mu()
{
  gStyle->SetOptStat(0);

  // 示例：元素质量百分比（可改为读取CSV）
  std::vector<TString> elements = {"SiO2","Al2O3","CeO2","B2O3","Gd2O3","ZnO","Li2O","PbO"};
  std::vector<double> percent   = {45, 15, 10, 8, 8, 6, 4, 2};

  // 示例μ（cm^-1）：可由实际拟合或数据库计算得到。此处给出占位示例。
  // 建议后续根据你的模拟/测量，对每种成分比例在某一参考能量（如 662 keV）下拟合 μ。
  std::vector<double> mu = {0.08, 0.06, 0.12, 0.05, 0.15, 0.07, 0.04, 0.20};

  TCanvas* c = new TCanvas("c_gamma_comp_mu", "Composition vs Linear Attenuation Coefficient", 1100, 750);
  c->SetGrid();

  // 画条形图
  TH1D* h = new TH1D("h_comp_mu", "Composition vs Linear Attenuation Coefficient;Component (% mass);#mu (cm^{-1})", elements.size(), 0, elements.size());
  for (int i = 0; i < (int)elements.size(); ++i) {
    h->SetBinContent(i+1, mu[i]);
    h->GetXaxis()->SetBinLabel(i+1, Form("%s(%.1f%%)", elements[i].Data(), percent[i]));
  }
  h->SetFillColor(kAzure-9);
  h->SetLineColor(kAzure+2);
  h->SetLineWidth(2);
  h->SetStats(0);
  h->Draw("BAR2");
  gPad->SetBottomMargin(0.18);
  h->LabelsOption("v");

  c->SaveAs("gamma_composition_mu.png");
  c->SaveAs("gamma_composition_mu.pdf");
}
