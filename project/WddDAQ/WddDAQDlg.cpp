
// WddDAQDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "WddDAQ.h"
#include "WddDAQDlg.h"
#include "afxdialogex.h"
#include <iomanip> // 用于 std::hex 和 std::dec



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 全局变量或类成员
std::atomic<bool> stopReading(false); // 用于控制线程停止
std::thread readThread; // 全局线程变量
std::atomic<int> readCount(0);
CWnd* pDisplayText; // 指向要更新的文本框的指针


//内部调用
CString  StrFomat(wchar_t* fmt, ...)
{
	wchar_t buffer[256];
	va_list args;
	va_start(args, fmt);
	vswprintf(buffer, 256, fmt, args);
	va_end(args);
	return buffer;
}
void ComBoxAddString(CComboBox* combox, CString str)
{
	int index = 1;
	CString strbuff;
	combox->ResetContent();
	while (true) {
		index = str.Find(';');
		if (index < 0) { break; }
		strbuff = str.Left(index);
		combox->AddString(strbuff);
		str = str.Right(str.GetLength() - index - 1);
	}
	combox->SetCurSel(0);
}
double CEditGetDouble(CEdit *pEdit)
{
	CString str;
	pEdit->GetWindowText(str);
	return _ttof(str);
}



void PrintRawData(const unsigned char* read_buffer, size_t size) {
    CString rawData;

    for (size_t i = 0; i < size; ++i) {
        // 将每个字节的十六进制值添加到字符串中
        rawData.AppendFormat(_T("%02X "), static_cast<int>(read_buffer[i]));

        // 每16个字节换行
        if ((i + 1) % 8 == 0) {
            rawData.Append(_T("\n"));
        }
    }

    // 显示消息框
    AfxMessageBox(rawData);
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CWddDAQDlg::CWddDAQDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WDDDAQ_DIALOG, pParent), m_nTimerID(0) // 初始化定时器ID
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CWddDAQDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECT_BTN, m_connectBtn);      // 连接按钮
	DDX_Control(pDX, IDC_DISCONNECT_BTN, m_disconnectBtn); // 断开连接按钮
	DDX_Control(pDX, IDC_START_ACQUIRE_BTN, m_startAcquireBtn); // 开始采集按钮
	DDX_Control(pDX, IDC_STOP_ACQUIRE_BTN, m_stopAcquireBtn);   // 停止采集按钮
	DDX_Control(pDX, IDC_STATUS_EDIT, m_statusEdit);      // 状态显示文本框
    DDX_Control(pDX, IDC_DRAWING, m_ChartCtrl);
    DDX_Control(pDX, IDC_RAWDATA, m_editRawData);
}

BEGIN_MESSAGE_MAP(CWddDAQDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CONNECT_BTN, &CWddDAQDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT_BTN, &CWddDAQDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_START_ACQUIRE_BTN, &CWddDAQDlg::OnBnClickedStartAcquire)
	ON_BN_CLICKED(IDC_STOP_ACQUIRE_BTN, &CWddDAQDlg::OnBnClickedStopAcquire)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_SETT_ACQPARA, &CWddDAQDlg::OnBnClickedSettAcqpara)
	ON_BN_CLICKED(IDC_SET_FILTER_TIME, &CWddDAQDlg::OnBnClickedSetFilterTime)
	ON_BN_CLICKED(IDC_SET_TRIGER_IO, &CWddDAQDlg::OnBnClickedSetTrigerIo)
	ON_BN_CLICKED(IDC_SET_RESISTOR, &CWddDAQDlg::OnBnClickedSetResistor)
    ON_BN_CLICKED(IDC_SET_DAC_PARA, &CWddDAQDlg::OnBnClickedSetDacPara)
    ON_BN_CLICKED(IDC_OUT0, &CWddDAQDlg::OnBnClickedOut0)
    ON_BN_CLICKED(IDC_OUT1, &CWddDAQDlg::OnBnClickedOut1)
    ON_BN_CLICKED(IDC_OUT2, &CWddDAQDlg::OnBnClickedOut2)
    ON_BN_CLICKED(IDC_OUT3, &CWddDAQDlg::OnBnClickedOut3)
    ON_BN_CLICKED(IDC_DISPLAY_DATA, &CWddDAQDlg::OnBnClickedDisplayData)
    ON_BN_CLICKED(IDC_CLEAR_DATA, &CWddDAQDlg::OnBnClickedClearData)
    ON_BN_CLICKED(IDC_SET_DAC_PARA2, &CWddDAQDlg::OnBnClickedSetDacPara2)
END_MESSAGE_MAP()


// CWddDAQDlg 消息处理程序

BOOL CWddDAQDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(_T("WddDAQDemo"));// 将“关于...”菜单项添加到系统菜单中。
    m_pictureControl.SubclassDlgItem(IDC_PICTURE, this);   // 获取 IDC_PICTURE 控件的指针
	// 获取指向 IDC_READ_COUNT 文本框的指针
	pDisplayText = GetDlgItem(IDC_READ_COUNT);

    CChartAxis* pAxis = NULL;
    pAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
    pAxis->SetAutomatic(true);
    pAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
    pAxis->SetAutomatic(true);
    //添加标题
    TChartString str1;
    str1 = _T("采样数据");
    m_ChartCtrl.GetTitle()->AddString(str1);
    
    // 设置初始显示范围
    m_xMin = 0;
    m_xMax = 100;
    m_yMin = 0;
    m_yMax = 100;

    // 设置轴范围
    SetAxisRange(m_xMin, m_xMax, m_yMin, m_yMax);

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// 参数设置
	SetDlgItemText(IDC_PARAM1, _T("100"));
	SetDlgItemText(IDC_PARAM2, _T("0"));
	SetDlgItemText(IDC_FILTER_TIME, _T("0"));
	ComBoxAddString((CComboBox*)GetDlgItem(IDC_MODE), _T("连续采样;时间采样;")); 
	ComBoxAddString((CComboBox*)GetDlgItem(IDC_TRIGER_IO), _T("0;1;2;3;4;5;6;7;"));
	((CComboBox*)GetDlgItem(IDC_TRIGER_IO))->SetCurSel(8);
	ComBoxAddString((CComboBox*)GetDlgItem(IDC_RESISTOR1), _T("0;1;2;3;4;5;6;7;"));
	((CComboBox*)GetDlgItem(IDC_RESISTOR1))->SetCurSel(8);
	ComBoxAddString((CComboBox*)GetDlgItem(IDC_RESISTOR2), _T("0;1;2;3;4;5;6;7;"));
	((CComboBox*)GetDlgItem(IDC_RESISTOR2))->SetCurSel(8);
	ComBoxAddString((CComboBox*)GetDlgItem(IDC_RESISTOR3), _T("0;1;2;3;4;5;6;7;"));
	((CComboBox*)GetDlgItem(IDC_RESISTOR3))->SetCurSel(8);
	// 设置此对话框的图标,当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MAXIMIZE);

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码
	// 设置定时器
    //SetTimer(1, 100, NULL);
	//m_nTimerID = SetTimer(1, 100, nullptr); // 保存定时器ID

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
/********************************************************************************************************************************************************************
                                                                        图表缩放
********************************************************************************************************************************************************************/
void CWddDAQDlg::SetAxisRange(double xMin, double xMax, double yMin, double yMax)
{
    // 设置 X 轴范围
    CChartAxis* pXAxis = m_ChartCtrl.GetBottomAxis();
    if (pXAxis) {
        pXAxis->SetMinMax(xMin, xMax);
    }

    // 设置 Y 轴范围
    CChartAxis* pYAxis = m_ChartCtrl.GetLeftAxis();
    if (pYAxis) {
        pYAxis->SetMinMax(yMin, yMax);
    }

    // 刷新图表
    m_ChartCtrl.RefreshCtrl();
}

BOOL CWddDAQDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // 计算缩放比例
    double zoomFactor = 1.1; // 每次滚动的缩放比例
    if (zDelta > 0) {
        // 放大
        m_xMin *= zoomFactor;
        m_xMax *= zoomFactor;
        m_yMin *= zoomFactor;
        m_yMax *= zoomFactor;
    }
    else {
        // 缩小
        m_xMin /= zoomFactor;
        m_xMax /= zoomFactor;
        m_yMin /= zoomFactor;
        m_yMax /= zoomFactor;
    }

    // 设置新的显示范围
    SetAxisRange(m_xMin, m_xMax, m_yMin, m_yMax);

    // 调用基类的实现
    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CWddDAQDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWddDAQDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWddDAQDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 在销毁时清理定时器
void CWddDAQDlg::OnDestroy()
{
	// 清理定时器
	if (m_nTimerID != 0) {
		KillTimer(m_nTimerID);
		m_nTimerID = 0; // 重置定时器 ID
	}
	CDialog::OnDestroy(); // 确保调用基类的 OnDestroy
}


//文本更新
void UpdateDisplay() {
	// 更新显示文本框内容
	CString str;
	str.Format(_T("%d"), readCount.load());
	pDisplayText->SetWindowText(str); // 更新文本框
}


// 连接按钮点击事件
void CWddDAQDlg::OnBnClickedConnect()
{
	// 假设连接函数返回 0 表示成功连接
	int rtn = ConnectedDevice();
	if (rtn) { ShowMessage(rtn, _T("ConnectedDevice")); }
	if (rtn == 0)
	{
		uint32_t LibVer;
		gmc_get_lib_version(&LibVer);
		SetDlgItemText(IDC_SC_STATUS, StrFomat(_T("Lib:%02x.%02x.%02x.%02x"),VERSION_STR(LibVer)));
		m_statusEdit.SetWindowTextW(L"已连接");
	}
}

// 断开连接按钮点击事件
void CWddDAQDlg::OnBnClickedDisconnect()
{
	// 假设连接函数返回 0 表示成功连接
	int rtn = CloseDevice();
	if (rtn) { ShowMessage(rtn, _T("CloseDevice")); }
	if (rtn == 0)
	{
		uint32_t LibVer = 0;
		SetDlgItemText(IDC_SC_STATUS, StrFomat(_T("Lib:%02x.%02x.%02x.%02x"), VERSION_STR(LibVer)));
		m_statusEdit.SetWindowTextW(L"未连接");
	}
}

// 开始采集按钮点击事件
void CWddDAQDlg::OnBnClickedStartAcquire()
{
	int rtn = StartADCCollection();
	if (rtn) { ShowMessage(rtn, _T("StartADCCollection")); }
	if (rtn == 0)
	{
		readCount = 0;
		stopReading = false; // 重置停止标志

		readThread = std::thread([=]() {
			while (!stopReading.load()) {
				unsigned char read_buffer[1024]; // 假设读取1024字节
                CString str;
				int rt = TryReadADCData(read_buffer, 1024);
				if (rt == 0) {
					readCount++;// 更新计数
                    size_t bufferSize = sizeof(read_buffer) / sizeof(read_buffer[0]);     // 调用打印函数
                   // PrintRawData(read_buffer, bufferSize);
                   // 调用打印函数
                   // 生成要显示的字符串
                    CString rawData;
                    for (size_t i = 0; i < bufferSize; ++i) {
                        rawData.AppendFormat(_T("%02X "), static_cast<int>(read_buffer[i]));

                        if ((i + 1) % 32 == 0) {
                            rawData.Append(_T("\r\n")); // 使用 "\r\n" 作为换行符
                        }
                    }
                    // 将生成的字符串设置到编辑控件中
                    m_editRawData.SetWindowText(rawData);
                    // 将读取的数据转换为整数并存储
                    for (int i = 0; i < 1024; i += 2) { // 每次增加2，读取一对字节
                        // 确保不会越界
                        if (i + 1 < 1024) {
                            // 根据小端模式组合两个字节
                            int value = static_cast<int>(read_buffer[i]) | (static_cast<int>(read_buffer[i + 1]) << 8);
                            accumulatedData.push_back(value);
                        }
                    }
                    str.Format(_T("%d"), readCount.load());
                    pDisplayText->SetWindowText(str); // 更新文本框
				}
				// 避免忙等待
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});
		readThread.detach(); // detach 线程以便其独立运行
	}
}

// 停止采集按钮点击事件
void CWddDAQDlg::OnBnClickedStopAcquire()
{
	int rtn = StopADCCollection();
	if (rtn) { ShowMessage(rtn, _T("StopADCCollection")); }
	if (rtn == 0)
	{
        readCount = 0;
        CString str;
        str.Format(_T("%d"), readCount.load());
        pDisplayText->SetWindowText(str); // 更新文本框
		// 设置停止标志
		stopReading = true;
		// 等待线程结束（如果需要的话）
		if (readThread.joinable()) {
			readThread.join(); // 等待线程完成
		}
	}
}

// 定时器处理，更新UI
void CWddDAQDlg::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == 1) {
        //uint32_t ultemp;
        //for (int i = 0; i < 4; i++)
        //{
        //    int rtn = GetIoInput(i, &ultemp);
        //    if (rtn) { ShowMessage(rtn, _T("ConfigureADCParameters")); }
        //    ((CButton*)GetDlgItem(IDC_IN0 + i))->SetCheck(ultemp & 0x01);
        //}
		UpdateDisplay();
	}
	CDialogEx::OnTimer(nIDEvent); // 调用基类的OnTimer处理
}


void CWddDAQDlg::OnBnClickedSettAcqpara()
{
	// TODO: 在此添加控件通知处理程序代码

	int mode = ((CComboBox*)GetDlgItem(IDC_MODE))->GetCurSel();
	float rate = (float)CEditGetDouble((CEdit*)GetDlgItem(IDC_PARAM1))*1000;
	float time = (float)CEditGetDouble((CEdit*)GetDlgItem(IDC_PARAM2));

	int rtn = ConfigureADCParameters(mode, rate, time);
	if (rtn) { ShowMessage(rtn, _T("ConfigureADCParameters")); }
}


void CWddDAQDlg::OnBnClickedSetFilterTime()
{
	// TODO: 在此添加控件通知处理程序代码IDC_FILTER_TIME
	int time = GetDlgItemInt(IDC_FILTER_TIME);
	int rtn = SetFilterTime(time);
	if (rtn) { ShowMessage(rtn, _T("SetFilterTime")); }
}


void CWddDAQDlg::OnBnClickedSetTrigerIo()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = ((CComboBox*)GetDlgItem(IDC_TRIGER_IO))->GetCurSel();
	if (((CButton*)GetDlgItem(IDC_SET_TRIGER_IO))->GetCheck()) {
		short rtn = SetTriggerIo(index, 1);
		if (rtn) { ShowMessage(rtn, _T("SetTriggerIo")); }
	}
	else {
		short rtn = SetTriggerIo(index, 0);
		if (rtn) { ShowMessage(rtn, _T("SetTriggerIo")); }
	}
}


void CWddDAQDlg::OnBnClickedSetResistor()
{
	// TODO: 在此添加控件通知处理程序代码IDC_RESISTOR1
	uint8_t Resistor[3];
	int value1 = ((CComboBox*)GetDlgItem(IDC_RESISTOR1))->GetCurSel();
	int value2 = ((CComboBox*)GetDlgItem(IDC_RESISTOR2))->GetCurSel();
	int value3 = ((CComboBox*)GetDlgItem(IDC_RESISTOR3))->GetCurSel();
	Resistor[0] = (uint8_t)value1;
	Resistor[1] = (uint8_t)value2;
	Resistor[2] = (uint8_t)value3;
	int rtn = SetSampResistor(&Resistor[0]);
	if (rtn) { ShowMessage(rtn, _T("SetSampResistor")); }
}


void CWddDAQDlg::OnBnClickedSetDacPara()
{
    // TODO: 在此添加控件通知处理程序代码
    int DAC[4];
    DAC[0] = CEditGetDouble((CEdit*)GetDlgItem(IDC_DAC1));
    DAC[1] = CEditGetDouble((CEdit*)GetDlgItem(IDC_DAC2));
    DAC[2] = CEditGetDouble((CEdit*)GetDlgItem(IDC_DAC3));
    DAC[3] = CEditGetDouble((CEdit*)GetDlgItem(IDC_DAC4));
    int rtn = SetDACParameters(&DAC[0]);
    if (rtn) { ShowMessage(rtn, _T("SetDACParameters")); }
}


void CWddDAQDlg::OnBnClickedOut0()
{
    // TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_OUT0))->GetCheck()) {
        short rtn = SetIoOutput(0, 1);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
    else {
        short rtn = SetIoOutput(0, 0);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
}


void CWddDAQDlg::OnBnClickedOut1()
{
    // TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_OUT1))->GetCheck()) {
        short rtn = SetIoOutput(1, 1);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
    else {
        short rtn = SetIoOutput(1, 0);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
}


void CWddDAQDlg::OnBnClickedOut2()
{
    // TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_OUT2))->GetCheck()) {
        short rtn = SetIoOutput(2, 1);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
    else {
        short rtn = SetIoOutput(2, 0);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
}


void CWddDAQDlg::OnBnClickedOut3()
{
    // TODO: 在此添加控件通知处理程序代码
    if (((CButton*)GetDlgItem(IDC_OUT3))->GetCheck()) {
        short rtn = SetIoOutput(3, 1);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
    else {
        short rtn = SetIoOutput(3, 0);
        if (rtn) { ShowMessage(rtn, _T("SetIoOutput")); }
    }
}


void CWddDAQDlg::OnBnClickedDisplayData()
{
    //if (!accumulatedData.empty()) {
    //    // 请求重绘 IDC_PICTURE 控件
    //    m_pictureControl.Invalidate(); // 使控件无效以触发重绘
    //    m_pictureControl.UpdateWindow(); // 立即更新窗口
    //}
    //else {
    //    AfxMessageBox(_T("没有可显示的数据！"));
    //    return; // 退出函数
    //}

    //double X1Values[10], Y1Values[10];
    //for (int i = 0; i < 10; i++)
    //{
    //    X1Values[i] = i;
    //    Y1Values[i] = i;
    //}

    //CChartLineSerie* pLineSerie2;

    //m_ChartCtrl.SetZoomEnabled(true);
    //m_ChartCtrl.RemoveAllSeries();//先清空
    //pLineSerie2 = m_ChartCtrl.CreateLineSerie();
    //pLineSerie2->SetSeriesOrdering(poNoOrdering);//设置为无序
    //pLineSerie2->SetPoints(X1Values, Y1Values, 10);

    //for (int i = 0; i < 1024; i++) {
    //    accumulatedData.push_back(static_cast<int>(i));
    //}

    // 检查 accumulatedData 是否为空
    if (accumulatedData.empty()) {
        AfxMessageBox(_T("没有可显示的数据！"));
        return; // 退出函数
    }

    // 清空图表
    m_ChartCtrl.RemoveAllSeries();

    // 准备 X 和 Y 数据
    int dataSize = accumulatedData.size();
    std::vector<double> XValues(dataSize);
    std::vector<double> YValues(dataSize);

    for (int i = 0; i < dataSize; i++)
    {
        XValues[i] = static_cast<double>(i); // 假设 X 值为索引
        YValues[i] = accumulatedData[i]; // Y 值为 accumulatedData 中的数据
    }

    // 创建并设置数据系列
    CChartLineSerie* pLineSerie = m_ChartCtrl.CreateLineSerie();
    pLineSerie->SetSeriesOrdering(poNoOrdering); // 设置为无序
    pLineSerie->SetPoints(XValues.data(), YValues.data(), dataSize); // 设置点
}


void CWddDAQDlg::OnBnClickedClearData()
{
    // TODO: 在此添加控件通知处理程序代码
    // 清空图表
    m_ChartCtrl.RemoveAllSeries();
    accumulatedData.clear(); // 清空累计数据
}



void CWddDAQDlg::OnBnClickedSetDacPara2()
{
    // TODO: 在此添加控件通知处理程序代码
    m_editRawData.SetWindowText(_T(""));  // 设置为空字符串
}
