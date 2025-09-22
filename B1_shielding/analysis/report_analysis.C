// 报告数据分析脚本 - 生成漂亮的图表
void report_analysis() {
    std::cout << "\n==========================================" << std::endl;
    std::cout << "    报告数据分析 - 中子和伽马射线" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // 打开数据文件：优先使用 data 下最新的输出
    TString latest = gSystem->GetFromPipe("ls -dt data/*/scintillator_output.root 2>/dev/null | head -1");
    TString filepath = latest.Length() ? latest : "build/scintillator_output.root";
    TFile* f = new TFile(filepath);
    if (!f || f->IsZombie()) {
        std::cout << "错误：无法打开数据文件: " << filepath << std::endl;
        return;
    }
    
    // 获取直方图
    TH1D* hEdep = (TH1D*)f->Get("Edep");
    TH1D* hDPA = (TH1D*)f->Get("DPA");
    TH1D* hNIEL = (TH1D*)f->Get("NIEL");
    TH1D* hNeutronTransmit = (TH1D*)f->Get("Neutron_Transmit_E");
    
    if (!hEdep || !hDPA || !hNIEL) {
        std::cout << "错误：无法找到直方图" << std::endl;
        return;
    }
    
    // 设置MATLAB风格样式
    gStyle->SetOptStat(0);
    gStyle->SetPadGridX(true);
    gStyle->SetPadGridY(true);
    gStyle->SetGridColor(kGray);
    gStyle->SetGridStyle(3);  // 虚线网格
    gStyle->SetGridWidth(1);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetFrameLineColor(kBlack);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetPadColor(kWhite);
    gStyle->SetCanvasColor(kWhite);
    
    // 设置字体
    gStyle->SetTitleFont(132, "XYZ");
    gStyle->SetLabelFont(132, "XYZ");
    gStyle->SetTextFont(132);
    
    // 创建画布
    TCanvas* c1 = new TCanvas("c1", "Report Analysis", 2000, 1400);
    c1->Divide(2, 2);
    
    // 设置画布边距，确保标题完整显示
    c1->SetMargin(0.1, 0.1, 0.1, 0.1);
    
    // 设置MATLAB风格颜色
    hEdep->SetLineColor(kBlue);
    hEdep->SetLineWidth(2);
    hEdep->SetFillColor(kBlue);
    hEdep->SetFillStyle(3004);
    
    hDPA->SetLineColor(kRed);
    hDPA->SetLineWidth(2);
    hDPA->SetFillColor(kRed);
    hDPA->SetFillStyle(3005);
    
    hNIEL->SetLineColor(kGreen);
    hNIEL->SetLineWidth(2);
    hNIEL->SetFillColor(kGreen);
    hNIEL->SetFillStyle(3006);
    
    // 绘制能量沉积
    c1->cd(1);
    hEdep->SetTitle("Energy Deposition Distribution;Energy (MeV);Counts");
    hEdep->GetXaxis()->SetTitleSize(0.05);
    hEdep->GetYaxis()->SetTitleSize(0.05);
    hEdep->GetXaxis()->SetLabelSize(0.04);
    hEdep->GetYaxis()->SetLabelSize(0.04);
    hEdep->GetXaxis()->SetTitleOffset(1.5);
    hEdep->GetYaxis()->SetTitleOffset(1.5);
    hEdep->Draw("HIST");
    
    // 绘制DPA三维图 - DPA vs Energy vs Counts
    c1->cd(2);
    
    // 创建三维直方图
    TH2D* hDPA2D = new TH2D("hDPA2D", "DPA vs Energy Distribution;Energy (MeV);DPA", 
                           50, 0, 10, 50, 0, 0.01);
    
    // 从原始数据填充二维直方图
    TTree* tree = (TTree*)f->Get("PhysicsData");
    if (tree) {
        Double_t edep, x, y, z;
        tree->SetBranchAddress("Edep", &edep);
        tree->SetBranchAddress("X", &x);
        tree->SetBranchAddress("Y", &y);
        tree->SetBranchAddress("Z", &z);
        
        Long64_t nentries = tree->GetEntries();
        for (Long64_t i = 0; i < nentries; i++) {
            tree->GetEntry(i);
            // 使用Edep作为能量，计算简化的DPA值
            Double_t dpa_value = edep * 0.001; // 简化的DPA计算
            hDPA2D->Fill(edep, dpa_value);
        }
    }
    
    hDPA2D->SetTitle("DPA vs Energy Distribution;Energy (MeV);DPA");
    hDPA2D->GetXaxis()->SetTitleSize(0.05);
    hDPA2D->GetYaxis()->SetTitleSize(0.05);
    hDPA2D->GetXaxis()->SetLabelSize(0.04);
    hDPA2D->GetYaxis()->SetLabelSize(0.04);
    hDPA2D->GetXaxis()->SetTitleOffset(1.5);
    hDPA2D->GetYaxis()->SetTitleOffset(1.5);
    
    // 设置颜色方案
    hDPA2D->SetContour(50);
    hDPA2D->Draw("COLZ");
    
    // 设置对数坐标 - Y轴对数
    c1->cd(2)->SetLogy();
    
    // 绘制NIEL - 使用对数坐标显示，聚焦有数据的区域
    c1->cd(3);
    hNIEL->SetTitle("Non-Ionizing Energy Loss (NIEL);NIEL (MeV);Counts");
    hNIEL->GetXaxis()->SetRangeUser(0, 0.2);  // 聚焦在0到0.2 MeV的范围，显示主要数据
    hNIEL->Rebin(1);  // 不合并bin，保持原始分辨率
    hNIEL->GetXaxis()->SetTitleSize(0.05);
    hNIEL->GetYaxis()->SetTitleSize(0.05);
    hNIEL->GetXaxis()->SetLabelSize(0.04);
    hNIEL->GetYaxis()->SetLabelSize(0.04);
    hNIEL->GetXaxis()->SetTitleOffset(1.5);
    hNIEL->GetYaxis()->SetTitleOffset(1.5);
    hNIEL->Draw("HIST");
    
    // 设置对数坐标 - Y轴对数
    c1->cd(3)->SetLogy();
    
    // 绘制屏蔽效率
    c1->cd(4);
    TH1D* hShielding = new TH1D("hShielding", "Shielding Efficiency Analysis", 2, 0, 2);
    hShielding->SetBinContent(1, hEdep->GetEntries());
    hShielding->SetBinContent(2, hNeutronTransmit ? hNeutronTransmit->GetEntries() : 0);
    hShielding->SetLineColor(kOrange);
    hShielding->SetLineWidth(2);
    hShielding->SetFillColor(kOrange);
    hShielding->SetFillStyle(3007);
    hShielding->SetTitle("Shielding Efficiency;Type;Counts");
    hShielding->GetXaxis()->SetTitleSize(0.05);
    hShielding->GetYaxis()->SetTitleSize(0.05);
    hShielding->GetXaxis()->SetLabelSize(0.04);
    hShielding->GetYaxis()->SetLabelSize(0.04);
    hShielding->GetXaxis()->SetTitleOffset(1.5);
    hShielding->GetYaxis()->SetTitleOffset(1.5);
    hShielding->GetXaxis()->SetBinLabel(1, "Shielded");
    hShielding->GetXaxis()->SetBinLabel(2, "Transmitted");
    hShielding->Draw("HIST");
    
    // 保存图像
    c1->SaveAs("report_analysis.png");
    c1->SaveAs("report_analysis.pdf");
    
    // 创建统计信息
    std::cout << "\n--- 统计信息 ---" << std::endl;
    std::cout << "能量沉积事件数: " << hEdep->GetEntries() << std::endl;
    std::cout << "DPA事件数: " << hDPA->GetEntries() << std::endl;
    std::cout << "NIEL事件数: " << hNIEL->GetEntries() << std::endl;
    if (hNeutronTransmit) {
        std::cout << "穿透中子数: " << hNeutronTransmit->GetEntries() << std::endl;
    }
    
    // 计算效率
    double totalEvents = hEdep->GetEntries() + (hNeutronTransmit ? hNeutronTransmit->GetEntries() : 0);
    if (totalEvents > 0) {
        double shieldingEfficiency = (double)hEdep->GetEntries() / totalEvents * 100.0;
        std::cout << "屏蔽效率: " << shieldingEfficiency << "%" << std::endl;
    }
    
    std::cout << "\n图像已保存为:" << std::endl;
    std::cout << "  - report_analysis.png" << std::endl;
    std::cout << "  - report_analysis.pdf" << std::endl;
    
    std::cout << "\n==========================================" << std::endl;
}