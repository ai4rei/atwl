Attribute VB_Name = "modMain"
Option Explicit

' ------------------------------------------------------------------
' AutoCAD DWG2PDF Automation
' (c) 2012-2014 Ai4rei/AN
'
' This work is licensed under a
' Creative Commons BY-SA 3.0 Unported License
' http://creativecommons.org/licenses/by-sa/3.0/
'
' ------------------------------------------------------------------

Private Const GENERIC_WRITE = &H40000000
Private Const CREATE_ALWAYS = &H2
Private Const FILE_ATTRIBUTE_NORMAL = &H80
Private Const INVALID_HANDLE_VALUE = &HFFFFFFFF

Private Declare Function CloseHandle Lib "kernel32.dll" (ByVal hHandle As Long) As Long
Private Declare Function CreateFileW Lib "kernel32.dll" (ByVal lpszFileName As Long, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, ByVal lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long

Private Declare Sub DWG2PDF_ProcessFiles Lib "dwg2pdf.dll" (ByVal lpszDirectory As Long, ByVal lpfnCallback As Long, ByVal nContext As Long)

Enum MAKEPDFSTATUS
    MPS_SUCCESS = 0    ' No error
    MPS_GENERAL = 1    ' General unexpected error, typically solved by restarting the instance
    MPS_OPENFAILED = 2 ' File could not be opened for whatever reason
    MPS_SAVEFAILED = 3 ' PDF file could not be written for whatever reason
End Enum

Private Sub Display(sMsg As String)
    MsgBox sMsg, vbInformation, App.ProductName
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

Private Function MakePDF(oAcad As Object, sFileName As String) As MAKEPDFSTATUS
    Dim nIdx As Integer
    Dim vMin As Variant
    Dim vMax As Variant
    Dim oFile As Object   ' AcadDocument
    Dim oConfig As Object ' AcadPlotConfiguration

    On Error GoTo OpenFailed

    Set oFile = oAcad.Documents.open(sFileName, True)
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
    
    On Error Resume Next

    If oAcad Is Nothing Then Set oAcad = CreateObject("AutoCAD.Application")
    If oAcad Is Nothing Then
        Display "Failed to initialize AutoCAD COM."
        
        ' fatal failure
        ForEachFileCallback = 0
        Exit Function
    ElseIf LCase(GetBaseName(sFilePath)) = ".quit" Then
        oAcad.Quit
        Set oAcad = Nothing
        
        ' intended quit
        ForEachFileCallback = 0
        Exit Function
    End If

    Dim sLExt As String
    Let sLExt = LCase(Right(sFilePath, 4))

    If sLExt = ".dwg" Or sLExt = ".dxf" Then
        Select Case MakePDF(oAcad, sFilePath)
            Case MPS_SUCCESS
                Kill sFilePath
            Case MPS_GENERAL
                oAcad.Quit
                Set oAcad = Nothing
            Case MPS_OPENFAILED
                MakeCookie GetBasePath(sFilePath) & "\" & GetBaseName(sFilePath) & " nelze otevrit"
            Case MPS_SAVEFAILED
                Dim sName As String
                Let sName = GetBaseName(sFilePath)
                MakeCookie GetBasePath(sFilePath) & "\" & Left(sName, Len(sName) - 3) & "pdf nelze ulozit"
        End Select
        DoEvents
    End If

    ' continue
    ForEachFileCallback = 1
End Function

Sub Main()
    If Dir$(Command$, vbDirectory) <> "" Then
        DWG2PDF_ProcessFiles StrPtr(Command$), AddressOf ForEachFileCallback, 0
    Else
        Display "Usage: DWG2PDF <working directory>"
    End If
End Sub
