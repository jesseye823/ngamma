// 测试不同步长设置的收敛性
void test_step_convergence() {
    // 比较不同maxStep设置下的结果
    vector<double> stepSizes = {10, 25, 50, 100, 200}; // μm
    
    cout << "步长收敛性测试建议：" << endl;
    cout << "1. 分别设置maxStep为: ";
    for(auto step : stepSizes) {
        cout << step << "μm ";
    }
    cout << endl;
    
    cout << "2. 运行相同的模拟（如1000个10MeV质子）" << endl;
    cout << "3. 比较总能量沉积的差异" << endl;
    cout << "4. 如果相邻两个设置的差异 < 1%，则较大的步长可接受" << endl;
    
    cout << "\n当前设置分析：" << endl;
    cout << "Production Cut: 10μm" << endl;
    cout << "Current MaxStep: 50μm (5x production cut)" << endl;
    cout << "探测器厚度: 75mm" << endl;
    cout << "预估步数: ~1500步" << endl;
    
    cout << "\n推荐的测试序列：" << endl;
    cout << "1. 先用200μm测试（快速）" << endl;
    cout << "2. 再用50μm验证（当前）" << endl;
    cout << "3. 如果差异<2%，可以用200μm提高效率" << endl;
}
