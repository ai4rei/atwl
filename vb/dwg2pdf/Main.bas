Attribute VB_Name = "modMain"
Option Explicit

' ------------------------------------------------------------------
' AutoCAD DWG2PDF Automation
' (c) 2012-2016 Ai4rei/AN
'
' This work is licensed under a
' Creative Commons BY-SA 3.0 Unported License
' http://creativecommons.org/licenses/by-sa/3.0/
'
' ------------------------------------------------------------------

' Windows API declarations
Private Const GENERIC_WRITE = &H40000000
Private Const CREATE_ALWAYS = &H2
Private Const FILE_ATTRIBUTE_NORMAL = &H80
Private Const INVALID_HANDLE_VALUE = &HFFFFFFFF
Private Const STD_OUTPUT_HANDLE = -11

Private Declare Function CloseHandle Lib "kernel32.dll" (ByVal hHandle As Long) As Long
Private Declare Function CreateFileW Lib "kernel32.dll" (ByVal lpszFileName As Long, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, ByVal lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long
Private Declare Function AllocConsole Lib "kernel32.dll" () As Long
Private Declare Function FreeConsole Lib "kernel32.dll" () As Long
Private Declare Function GetStdHandle Lib "kernel32.dll" (ByVal nStdHandle As Long) As Long
Private Declare Function WriteConsoleW Lib "kernel32.dll" (ByVal hHandle As Long, ByVal lpBuffer As Long, ByVal nNumberOfCharsToWrite As Long, ByRef lpNumberOfCharsWritten As Long, ByVal lpReserved As Long) As Long
Private Declare Function SetConsoleCtrlHandler Lib "kernel32.dll" (ByVal lpfnHandler As Long, ByVal bAdd As Long) As Long

' AutoCAD declarations
Const acExtents = 1
Const acMillimeters = 1
Const ac0degrees = 0
Const acScaleToFit = 0
Const acActiveViewport = 0

' other
Private Declare Sub DWG2PDF_ProcessFiles Lib "dwg2pdf.dll" (ByVal lpszDirectory As Long, ByVal lpfnCallback As Long, ByVal nContext As Long)

Enum MAKEPDFSTATUS
    MPS_SUCCESS = 0    ' No error
    MPS_GENERAL = 1    ' General unexpected error, typically solved by restarting the instance
    MPS_OPENFAILED = 2 ' File could not be opened for whatever reason
    MPS_SAVEFAILED = 3 ' PDF file could not be written for whatever reason
    MPS_OPENFAILED2 = 4 ' File could not be opened, but did not throw exception
End Enum

Private Sub Display(sMsg As String)
    MsgBox sMsg, vbInformation, App.ProductName
End Sub

Private Sub ShowMessage(sMsg As String)
    Dim nDummy As Long

    WriteConsoleW GetStdHandle(STD_OUTPUT_HANDLE), StrPtr(sMsg), Len(sMsg), nDummy, 0
End Sub

Private Sub MakeCookie(sName As String)
    Dim hFile As Long
    Let hFile = CreateFileW(StrPtr(sName & "." & Chr(160)), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)

    If hFile <> INVALID_HANDLE_VALUE Then
        CloseHandle hFile
    End If
End Sub

Private Function GetBasePath(sFilePath As String)
    Dim nIdx As Long
    Let nIdx = InStrRev(sFilePath, "\")

    If nIdx > 0 Then
        GetBasePath = Left(sFilePath, nIdx - 1)
    Else
        GetBasePath = "."
    End If
End Function

Private Function GetBaseName(sFilePath As String)
    Dim nIdx As Long
    Let nIdx = InStrRev(sFilePath, "\")

    If nIdx > 0 Then
        GetBaseName = Mid(sFilePath, nIdx + 1)
    Else
        GetBaseName = sFilePath
    End If
End Function

Private Function MakePDF2(oAcad As Object, sFileName As String) As MAKEPDFSTATUS
    Dim oDoc As Object ' AcadDocument

    On Error GoTo L_LoadFailed
    Set oDoc = oAcad.Documents.Open(sFileName, True)
    On Error GoTo 0

    If oDoc Is Nothing Then
        MakePDF2 = MPS_OPENFAILED2
        Exit Function
    End If

    Dim vMin As Variant
    Dim vMax As Variant
    Dim oLay As Object ' AcadLayout
    oDoc.SetVariable "BACKGROUNDPLOT", CInt(0)

    For Each oLay In oDoc.Layouts
        If oLay.Block.Count = 0 Then GoTo L_Continue

        oDoc.ActiveLayout = oLay
        oLay.ConfigName = "DWG To PDF.pc3"

        oLay.PlotType = acExtents
        oLay.CenterPlot = True ' will not work with PlotType == acLayout
        oLay.PaperUnits = acMillimeters
        oLay.PlotHidden = False
        oLay.PlotRotation = ac0degrees
        oLay.PlotViewportBorders = False
        oLay.PlotViewportsFirst = True
        oLay.PlotWithLineweights = True
        oLay.PlotWithPlotStyles = True
        oLay.ScaleLineweights = False
        oLay.ShowPlotStyles = False
        oLay.StandardScale = acScaleToFit
        oLay.RefreshPlotDeviceInfo
        oLay.StyleSheet = "monochrome.ctb"
        oLay.UseStandardScale = True
        oLay.RefreshPlotDeviceInfo

        vMin = oDoc.GetVariable("EXTMIN")
        vMax = oDoc.GetVariable("EXTMAX")

        If vMax(0) - vMin(0) > vMax(1) - vMin(1) Then
            ' landscape
            oLay.CanonicalMediaName = "ISO_full_bleed_A4_(297.00_x_210.00_MM)"
        Else
            ' portrait
            oLay.CanonicalMediaName = "ISO_full_bleed_A4_(210.00_x_297.00_MM)"
        End If

        oLay.RefreshPlotDeviceInfo
        oDoc.Regen acActiveViewport

        On Error GoTo L_SaveFailed
        oDoc.Plot.PlotToFile Left(oDoc.FullName, Len(oDoc.FullName) - 4) & "-" & oLay.Name & ".pdf" ' <source name>-<layout name>
        On Error GoTo 0
L_Continue:
    Next

L_FastExit:
    oDoc.Close False
    MakePDF2 = MPS_SUCCESS
    Exit Function

L_LoadFailed:
    MakePDF2 = MPS_OPENFAILED
    On Error GoTo 0
    Resume L_FastExit

L_SaveFailed:
    MakePDF2 = MPS_SAVEFAILED
    On Error GoTo 0
    Resume L_FastExit
End Function

Private Function MakePDF(oAcad As Object, sFileName As String) As MAKEPDFSTATUS
    Dim nIdx As Integer
    Dim vMin As Variant
    Dim vMax As Variant
    Dim oFile As Object   ' AcadDocument
    Dim oConfig As Object ' AcadPlotConfiguration

    On Error GoTo OpenFailed

    Set oFile = oAcad.Documents.Open(sFileName, True)
    If oFile Is Nothing Then GoTo OpenFailed

    On Error GoTo TryAgain

    ' oFile.Application.ZoomExtents
    vMin = oFile.GetVariable("EXTMIN")
    vMax = oFile.GetVariable("EXTMAX")
    oFile.SetVariable "BACKGROUNDPLOT", CInt(0)

    While oFile.PlotConfigurations.Count > 0
        oFile.PlotConfigurations(0).Delete
    Wend
    Set oConfig = oFile.PlotConfigurations.Add("Batch", True)

    With oConfig
        .CenterPlot = True
        .ConfigName = "DWG To PDF.pc3"
        ' .ModelType = True
        ' .Name = "Batch"
        .PaperUnits = 1                  ' acMillimeters
        .PlotHidden = False
        .PlotRotation = 0                ' ac0degrees
        .PlotType = 1                    ' acExtents
        .PlotViewportBorders = False
        .PlotViewportsFirst = True
        .PlotWithLineweights = True
        .PlotWithPlotStyles = True
        .ScaleLineweights = False
        .ShowPlotStyles = False
        .StandardScale = 0               ' acScaleToFit
        .RefreshPlotDeviceInfo
        .StyleSheet = "monochrome.ctb"
        .UseStandardScale = True
        .RefreshPlotDeviceInfo
    End With

    If vMax(0) - vMin(0) > vMax(1) - vMin(1) Then
        ' Landscape
        oConfig.CanonicalMediaName = "ISO_full_bleed_A4_(297.00_x_210.00_MM)"
    Else
        ' Portrait
        oConfig.CanonicalMediaName = "ISO_full_bleed_A4_(210.00_x_297.00_MM)"
    End If

    oFile.ModelSpace.Layout.CopyFrom oConfig

    On Error GoTo SaveFailed

    If Not oFile.Plot.PlotToFile(Left(oFile.FullName, Len(oFile.FullName) - 3) & "pdf") Then GoTo SaveFailed

    On Error GoTo 0

    MakePDF = MPS_SUCCESS

DoExit:
    If Not oFile Is Nothing Then
        oFile.Close False
    End If
    Exit Function

OpenFailed:
    MakePDF = MPS_OPENFAILED
    Resume DoExit

SaveFailed:
    MakePDF = MPS_SAVEFAILED
    Resume DoExit

TryAgain:
    On Error GoTo ResetInstance
    Resume

ResetInstance:
    MakePDF = MPS_GENERAL
    Resume DoExit
End Function

Public Function ForEachFileCallback(ByVal sFilePath As String, ByVal nContext As Long) As Long
    Static oAcad As Object

    If oAcad Is Nothing Then
        Set oAcad = CreateObject("AutoCAD.Application")
    
        If oAcad Is Nothing Then
            Display "Failed to initialize AutoCAD server."

            ' fatal failure
            ForEachFileCallback = 0
            Exit Function
        End If
    End If
    
    If LCase(GetBaseName(sFilePath)) = ".quit" Then
        oAcad.Quit
        Set oAcad = Nothing

        ' intended quit
        ForEachFileCallback = 0
        Kill sFilePath
        Exit Function
    End If

    Dim sLExt As String
    Let sLExt = LCase(Right(sFilePath, 4))

    Dim nTries As Long

    If sLExt = ".dwg" Or sLExt = ".dxf" Then
        ShowMessage sFilePath

        Select Case MakePDF2(oAcad, sFilePath)
            Case MPS_SUCCESS
                On Error GoTo L_RetryDelete
                Kill sFilePath
L_DoNotDelete:
                On Error GoTo 0
                ShowMessage " OK" & vbCrLf
            Case MPS_GENERAL
                oAcad.Quit
                Set oAcad = Nothing
            Case MPS_OPENFAILED
                MakeCookie GetBasePath(sFilePath) & "\" & GetBaseName(sFilePath) & " otevreni selhalo"
                ShowMessage " OFail" & vbCrLf
            Case MPS_OPENFAILED2
                MakeCookie GetBasePath(sFilePath) & "\" & GetBaseName(sFilePath) & " nelze otevrit"
                ShowMessage " OFail2" & vbCrLf
            Case MPS_SAVEFAILED
                Dim sName As String
                Let sName = GetBaseName(sFilePath)
                MakeCookie GetBasePath(sFilePath) & "\" & Left(sName, Len(sName) - 3) & "pdf nelze ulozit"
                ShowMessage " SFail" & vbCrLf
        End Select
        DoEvents
    End If

    ' continue
    ForEachFileCallback = 1
    Exit Function
    
L_RetryDelete:
    If nTries > 120 Then
        ShowMessage " *"
        Resume L_DoNotDelete
    End If

    DoEvents
    nTries = nTries + 1
    Resume
End Function

Sub Main()
    AllocConsole

    If Dir$(Command$, vbDirectory) <> "" Then
        DWG2PDF_ProcessFiles StrPtr(Command$), AddressOf ForEachFileCallback, 0
    Else
        Display "Usage: DWG2PDF <working directory>"
    End If

    FreeConsole
End Sub
