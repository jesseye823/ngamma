// 几何结构绘图脚本
void geometry_plot() {
    // 设置ROOT样式
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(true);
    gStyle->SetPadGridY(true);
    gStyle->SetGridColor(kGray);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetTitleFont(132, "XYZ");
    gStyle->SetLabelFont(132, "XYZ");
    gStyle->SetTextFont(132);
    
    // 创建画布
    TCanvas* c1 = new TCanvas("c1", "Shielding Glass Geometry", 1200, 900);
    c1->Divide(2, 2);
    
    // 前视图 (X-Z)
    c1->cd(1);
    TBox* glass_front = new TBox(-10, -3.75, 10, 3.75);
    glass_front->SetFillColor(kBlue);
    glass_front->SetLineWidth(2);
    glass_front->Draw();
    
    TEllipse* source_front = new TEllipse(0, -30, 2, 2);
    source_front->SetFillColor(kRed);
    source_front->SetLineWidth(2);
    source_front->Draw();
    
    gPad->SetGrid();
    gPad->SetTitle("Front View (X-Z)");
    
    // 侧视图 (Y-Z)
    c1->cd(2);
    TBox* glass_side = new TBox(-10, -3.75, 10, 3.75);
    glass_side->SetFillColor(kBlue);
    glass_side->SetLineWidth(2);
    glass_side->Draw();
    
    TEllipse* source_side = new TEllipse(0, -30, 2, 2);
    source_side->SetFillColor(kRed);
    source_side->SetLineWidth(2);
    source_side->Draw();
    
    gPad->SetGrid();
    gPad->SetTitle("Side View (Y-Z)");
    
    // 顶视图 (X-Y)
    c1->cd(3);
    TBox* glass_top = new TBox(-10, -10, 10, 10);
    glass_top->SetFillColor(kBlue);
    glass_top->SetLineWidth(2);
    glass_top->Draw();
    
    TEllipse* source_top = new TEllipse(0, 0, 2, 2);
    source_top->SetFillColor(kRed);
    source_top->SetLineWidth(2);
    source_top->Draw();
    
    gPad->SetGrid();
    gPad->SetTitle("Top View (X-Y)");
    
    // 材料成分图
    c1->cd(4);
    TH1D* hComposition = new TH1D("hComposition", "Material Composition;Element;Percentage (%)", 8, 0, 8);
    hComposition->SetBinContent(1, 45); // SiO2
    hComposition->SetBinContent(2, 15); // Al2O3
    hComposition->SetBinContent(3, 10); // CeO2
    hComposition->SetBinContent(4, 8);  // B2O3
    hComposition->SetBinContent(5, 8);  // Gd2O3
    hComposition->SetBinContent(6, 6);  // ZnO
    hComposition->SetBinContent(7, 4);  // Li2O
    hComposition->SetBinContent(8, 2);  // PbO
    
    hComposition->GetXaxis()->SetBinLabel(1, "SiO2");
    hComposition->GetXaxis()->SetBinLabel(2, "Al2O3");
    hComposition->GetXaxis()->SetBinLabel(3, "CeO2");
    hComposition->GetXaxis()->SetBinLabel(4, "B2O3");
    hComposition->GetXaxis()->SetBinLabel(5, "Gd2O3");
    hComposition->GetXaxis()->SetBinLabel(6, "ZnO");
    hComposition->GetXaxis()->SetBinLabel(7, "Li2O");
    hComposition->GetXaxis()->SetBinLabel(8, "PbO");
    
    hComposition->SetFillColor(kBlue);
    hComposition->SetLineWidth(2);
    hComposition->Draw("BAR");
    
    gPad->SetGrid();
    
    // 保存图像
    c1->Print("report_images/geometry_plot.png");
    c1->Print("report_images/geometry_plot.pdf");
    
    cout << "几何结构图已保存到 report_images/geometry_plot.png/pdf" << endl;
}