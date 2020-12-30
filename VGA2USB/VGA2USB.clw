; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "VGA2USB.h"
LastPage=0

ClassCount=4
Class1=CVGA2USBApp
Class3=CMainFrame
Class4=CAboutDlg

ResourceCount=5
Resource1=IDD_CONFIG_DIALOG
Class2=CChildView
Resource2=IDD_NETCFG_DIALOG
Resource3=IDR_MAINFRAME
Resource4=IDD_ABOUTBOX
Resource5=IDD_DISPLAY_DIALOG

[CLS:CVGA2USBApp]
Type=0
HeaderFile=VGA2USB.h
ImplementationFile=VGA2USB.cpp
Filter=N

[CLS:CChildView]
Type=0
HeaderFile=ChildView.h
ImplementationFile=ChildView.cpp
Filter=N

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=VGA2USB.cpp
ImplementationFile=VGA2USB.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=8
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_MAILTO,static,1342308352
Control6=IDC_BUILD,static,1342308352
Control7=IDC_ADDRESS,static,1342308352
Control8=IDC_BMP,static,1342177294

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_EDIT_COPY
Command3=ID_VIEW_TOOLBAR
Command4=ID_VIEW_STATUS_BAR
Command5=ID_FULL_SCREEN
Command6=ID_DEVICE_CONFIG
Command7=ID_NET_CONFIG
Command8=ID_DISPLAY_CONFIG
Command9=ID_APP_ABOUT
CommandCount=9

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_AUDIO_ENABLE
Command2=ID_EDIT_COPY
Command3=ID_VIDEO_PAUSE
Command4=ID_VIDEO_REC
Command5=ID_SNAPSHOT_SAVE
Command6=ID_NEXT_PANE
Command7=ID_PREV_PANE
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_EDIT_CUT
Command11=ID_EDIT_UNDO
CommandCount=11

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_SNAPSHOT_SAVE
Command2=ID_SNAPSHOT_PRINT
Command3=ID_SNAPSHOT_COPY
Command4=ID_VIDEO_REC
Command5=ID_VIDEO_PAUSE
Command6=ID_AUDIO_ENABLE
Command7=ID_NETSERVICE_ENABLE
Command8=ID_CONNECT_NETDEVICE
CommandCount=8

[DLG:IDD_CONFIG_DIALOG]
Type=1
Class=?
ControlCount=25
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_GAIN_RED,msctls_trackbar32,1342177281
Control4=IDC_GAIN_GREEN,msctls_trackbar32,1342177281
Control5=IDC_GAIN_BLUE,msctls_trackbar32,1342177281
Control6=IDC_STATIC_RED,static,1342177280
Control7=IDC_STATIC_GREEN,static,1342177280
Control8=IDC_STATIC_BLUE,static,1342177280
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,button,1342177287
Control11=IDC_OFFSET_RED,msctls_trackbar32,1342242817
Control12=IDC_OFFSET_GREEN,msctls_trackbar32,1342242817
Control13=IDC_OFFSET_BLUE,msctls_trackbar32,1342242817
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,static,1342308352
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_STATIC,static,1342308352

[DLG:IDD_NETCFG_DIALOG]
Type=1
Class=?
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_IPADDRESS,SysIPAddress32,1342242816
Control4=IDC_EDIT_PORT,edit,1350639744
Control5=IDC_STATIC_PORT,static,1342308352
Control6=IDC_STATIC_IP,static,1342308352

[DLG:IDD_DISPLAY_DIALOG]
Type=1
Class=?
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_CHECK_SCALING,button,1342242819

