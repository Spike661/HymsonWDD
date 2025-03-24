
// WddDAQDlg.h: 头文件
//

#pragma once
#include "WddDAQ.h"  
#include <vector>
#include ".\ChartCtrl\ChartCtrl.h"
#include ".\ChartCtrl\ChartTitle.h"
#include ".\ChartCtrl\ChartLineSerie.h"  //画线头文件
// CWddDAQDlg 对话框
// CWddDAQDlg 对话框
class CWddDAQDlg : public CDialogEx
{
public:
    CWddDAQDlg(CWnd* pParent = nullptr); // 标准构造函数
    virtual void OnDestroy();
    std::vector<int> accumulatedData; // 用于存储累计的数据
    CStatic m_pictureControl; // 用于显示图表的静态控件
    CEdit m_editRawData;

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WDDDAQ_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 支持
    virtual BOOL OnInitDialog();
    virtual void OnTimer(UINT_PTR nIDEvent);
    
    // 定义成员变量
    UINT_PTR m_nTimerID; // 定义一个定时器ID

protected:
    HICON m_hIcon;

    // 控件变量
    CButton m_connectBtn;      // 连接按钮
    CButton m_disconnectBtn;   // 断开连接按钮
    CButton m_startAcquireBtn; // 开始采集按钮
    CButton m_stopAcquireBtn;  // 停止采集按钮
    CEdit m_statusEdit;        // 状态显示文本框
 
    CChartCtrl m_ChartCtrl;

    afx_msg void OnBnClickedConnect();
    afx_msg void OnBnClickedDisconnect();
    afx_msg void OnBnClickedStartAcquire();
    afx_msg void OnBnClickedStopAcquire();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void SetAxisRange(double xMin, double xMax, double yMin, double yMax);
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt); // 声明消息处理函数
    DECLARE_MESSAGE_MAP()

private:
    double m_xMin, m_xMax; // X 轴范围
    double m_yMin, m_yMax; // Y 轴范围

public:
    afx_msg void OnBnClickedSettAcqpara();
    afx_msg void OnBnClickedSetFilterTime();
    afx_msg void OnBnClickedSetTrigerIo();
    afx_msg void OnBnClickedSetResistor();

    afx_msg void OnBnClickedSetDacPara();
    afx_msg void OnBnClickedOut0();
    afx_msg void OnBnClickedOut1();
    afx_msg void OnBnClickedOut2();
    afx_msg void OnBnClickedOut3();

    afx_msg void OnBnClickedDisplayData();
    afx_msg void OnBnClickedClearData();
    afx_msg void OnBnClickedSetDacPara2();
};
