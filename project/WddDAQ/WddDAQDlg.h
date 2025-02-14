
// WddDAQDlg.h: 头文件
//

#pragma once
#include "WddDAQ.h"  
#include <vector>

// CWddDAQDlg 对话框
// CWddDAQDlg 对话框
class CWddDAQDlg : public CDialogEx
{
public:
    CWddDAQDlg(CWnd* pParent = nullptr); // 标准构造函数
    virtual void OnDestroy();
    std::vector<int> accumulatedData; // 用于存储累计的数据
    CStatic m_pictureControl; // 用于显示图表的静态控件

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
    CButton m_connectBtn;     // 连接按钮
    CButton m_disconnectBtn;  // 断开连接按钮
    CButton m_startAcquireBtn; // 开始采集按钮
    CButton m_stopAcquireBtn;  // 停止采集按钮
    CEdit m_statusEdit;      // 状态显示文本框
    

    afx_msg void OnBnClickedConnect();
    afx_msg void OnBnClickedDisconnect();
    afx_msg void OnBnClickedStartAcquire();
    afx_msg void OnBnClickedStopAcquire();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
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
};
