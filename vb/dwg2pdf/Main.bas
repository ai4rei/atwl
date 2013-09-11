Attribute VB_Name = "modMain"
Option Explicit

' ------------------------------------------------------------------
' AutoCAD DWG2PDF Automation
' (c) 2012-2013 Ai4rei/AN
'
' This work is licensed under a
' Creative Commons BY-SA 3.0 Unported License
' http://creativecommons.org/licenses/by-sa/3.0/
'
' ------------------------------------------------------------------

Private Declare Function KrnHlp_FindFirstChange Lib "krnhlp.dll" (ByVal lpDirectory As String) As Long
Private Declare Function KrnHlp_FindNextChange Lib "krnhlp.dll" (ByVal hChange As Long) As Long
Private Declare Function KrnHlp_FindCloseChange Lib "krnhlp.dll" (ByVal hChange As Long) As Long

Private m_Init As Boolean ' signals, that AutoCAD must be restarted on next chance
Private m_Quit As Boolean ' signals, that the application must quit

Private Sub MakePDF(oFile As Object)
    Dim myIdx As Integer
    Dim myMin As Variant
    Dim myMax As Variant
    Dim oConfig As Object 'AcadPlotConfiguration

    On Error GoTo TryAgain

    ' oFile.Application.ZoomExtents
    myMin = oFile.GetVariable("EXTMIN")
    myMax = oFile.GetVariable("EXTMAX")
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
        .PaperUnits = 1 'acMillimeters
        .PlotHidden = False
        .PlotRotation = 0 'ac0degrees
        .PlotType = 1 'acExtents
        .PlotViewportBorders = False
        .PlotViewportsFirst = True
        .PlotWithLineweights = True
        .PlotWithPlotStyles = True
        .ScaleLineweights = False
        .ShowPlotStyles = False
        .StandardScale = 0 'acScaleToFit
        .RefreshPlotDeviceInfo
        .StyleSheet = "monochrome.ctb"
        .UseStandardScale = True
        .RefreshPlotDeviceInfo
    End With

    If myMax(0) - myMin(0) > myMax(1) - myMin(1) Then
        ' Landscape
        oConfig.CanonicalMediaName = "ISO_full_bleed_A4_(297.00_x_210.00_MM)"
    Else
        ' Portrait
        oConfig.CanonicalMediaName = "ISO_full_bleed_A4_(210.00_x_297.00_MM)"
    End If

    oFile.ModelSpace.Layout.CopyFrom oConfig
    oFile.Plot.PlotToFile Left(oFile.FullName, Len(oFile.FullName) - 4) & ".pdf"
    Exit Sub
TryAgain:
    On Error GoTo RestartIt
    Resume
RestartIt:
    m_Init = True
End Sub

Private Sub DoAllAcadObjects(oAcad As Object, ByVal myWorkPath As String)
    Dim oFile As Object 'AcadDocument
    Dim myFile As String
    Dim myLExt As String

    myWorkPath = myWorkPath & "\"
    myFile = Dir(myWorkPath & "*.*", vbNormal)
    Do While myFile <> ""
        myLExt = LCase(Right(myFile, 4))

        If myFile = ".quit" Then
            m_Quit = True
            Exit Sub
        End If

        If myLExt = ".dwg" Or myLExt = ".dxf" Then
            On Error Resume Next
            Set oFile = oAcad.Documents.Open(myWorkPath & myFile, True)
            On Error GoTo 0

            If Not oFile Is Nothing Then
                MakePDF oFile
                oFile.Close False

                On Error Resume Next
                Kill myWorkPath & myFile
                On Error GoTo 0

                If m_Init Then
                    Exit Do
                End If
            End If
        End If

        DoEvents
        myFile = Dir()
    Loop
End Sub

Sub Main()
    Dim hChange As Long
    Dim oAcad As Object

    ' are we provided with correct parameter?
    If Dir$(Command$, vbDirectory) = "" Then
        MsgBox "Usage: DWG2PDFS <directory>", , App.ProductName
        Exit Sub
    End If

    ' signal (re)start of AutoCAD
    m_Init = True

    ' look for files
    hChange = KrnHlp_FindFirstChange(Command$)

    If hChange <> -1 Then
        On Error Resume Next

        Do
            If m_Init Then
                m_Init = False

                If Not oAcad Is Nothing Then
                    ' quit previous instance if any
                    oAcad.Quit
                End If

                Set oAcad = CreateObject("AutoCAD.Application")
                
                If oAcad Is Nothing Then
                    ' check if we had a successful start
                    MsgBox "Failed to initialize AutoCAD.", , App.ProductName
                    Exit Do
                End If
            End If
            
            DoAllAcadObjects oAcad, Command$
            DoEvents

            If m_Quit Then
                Exit Do
            End If
        Loop While KrnHlp_FindNextChange(hChange)

        On Error GoTo 0

        If Not oAcad Is Nothing Then
            oAcad.Quit
        End If
        KrnHlp_FindCloseChange hChange
    End If
End Sub
