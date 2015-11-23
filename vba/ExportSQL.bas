Attribute VB_Name = "ExportSQL"
Option Compare Database
Option Explicit

' ------------------------------------------------------------------
' Export MS Access Tables and Relations to a MySQL-compatible Dump
' (c) 2013 Ai4rei/AN
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

Private Declare Function CloseHandle Lib "kernel32.dll" (ByVal hHandle As Long) As Boolean
Private Declare Function WriteFile Lib "kernel32.dll" (ByVal hFile As Long, ByVal lpBuffer As String, ByVal dwWrite As Long, ByRef dwWritten As Long, ByVal lpTemplate As Long) As Boolean
Private Declare Function CreateFile Lib "kernel32.dll" Alias "CreateFileA" (ByVal lpFileName As String, ByVal dwDesiredAccess As Long, ByVal dwShareMode As Long, ByVal lpSecurityAttributes As Long, ByVal dwCreationDisposition As Long, ByVal dwFlagsAndAttributes As Long, ByVal hTemplateFile As Long) As Long

Private Function ExportSQL_P_GetDateStrFromDate(myDate As Date) As String
    ExportSQL_P_GetDateStrFromDate = IIf(Year(myDate) > 999, "", "0") & Year(myDate) & "-" & IIf(Month(myDate) > 9, "", "0") & Month(myDate) & "-" & IIf(Day(myDate) > 9, "", "0") & Day(myDate)
End Function

Private Function ExportSQL_P_GetTimeStrFromDate(myDate As Date) As String
    ExportSQL_P_GetTimeStrFromDate = IIf(Hour(myDate) > 9, "", "0") & Hour(myDate) & ":" & IIf(Minute(myDate) > 9, "", "0") & Minute(myDate) & ":" & IIf(Second(myDate) > 9, "", "0") & Second(myDate)
End Function

Private Sub ExportSQL_P_Write(hFile As Long, sString As String)
    Dim myWritten As Long

    If hFile <> INVALID_HANDLE_VALUE Then
        Debug.Assert WriteFile(hFile, sString & vbCrLf, Len(sString & vbCrLf), myWritten, 0) And Len(sString & vbCrLf) = myWritten
    Else
        Debug.Print sString
    End If
End Sub

Private Sub ExportSQL_P_TableForeignKeys(hFile As Long)
    Dim oRelation As Relation
    Dim oField As Field
    Dim aFrKeys() As String
    Dim aFields() As String
    Dim nFieldIdx As Integer

    ExportSQL_P_Write hFile, "--"
    ExportSQL_P_Write hFile, "-- Foreign Keys"
    ExportSQL_P_Write hFile, "--"

    For Each oRelation In CurrentDb.Relations
        ExportSQL_P_Write hFile, "ALTER TABLE `" & oRelation.ForeignTable & "` ADD CONSTRAINT `" & oRelation.Name & "` FOREIGN KEY"

        If oRelation.Attributes And Not (dbRelationDeleteCascade Or dbRelationUpdateCascade Or dbRelationUnique) Then
            Debug.Assert 0
        End If

        With oRelation
            ReDim aFrKeys(0 To .Fields.Count - 1)
            ReDim aFields(0 To .Fields.Count - 1)
    
            For nFieldIdx = 0 To .Fields.Count - 1
                With .Fields(nFieldIdx)
                    aFrKeys(nFieldIdx) = "`" & .ForeignName & "`"
                    aFields(nFieldIdx) = "`" & .Name & "`"
                End With
            Next
        End With

        ExportSQL_P_Write hFile, Chr(9) & "(" & Join(aFrKeys, ",") & ") REFERENCES `" & oRelation.Table & "` (" & Join(aFields, ",") & ")"
        ExportSQL_P_Write hFile, Chr(9) & "ON DELETE " & IIf(oRelation.Attributes And dbRelationDeleteCascade, "CASCADE", "RESTRICT")
        ExportSQL_P_Write hFile, Chr(9) & "ON UPDATE " & IIf(oRelation.Attributes And dbRelationUpdateCascade, "CASCADE", "RESTRICT")
        ExportSQL_P_Write hFile, Chr(9) & ";"
        ExportSQL_P_Write hFile, ""
    Next
End Sub

Private Sub ExportSQL_P_TableDump(hFile As Long, sTableName As String)
    Dim myRes As DAO.Recordset
    Dim myDT As Date
    Dim sDTFmt As String
    Dim sInsertDef As String
    Dim sInsertPak As String
    Dim nData As Long

    Set myRes = CurrentDb.OpenRecordset("SELECT * FROM [" & sTableName & "];", dbOpenSnapshot)

    If myRes.RecordCount > 0 Then
        ExportSQL_P_Write hFile, "--"
        ExportSQL_P_Write hFile, "-- Table `" & sTableName & "` Data"
        ExportSQL_P_Write hFile, "--"
    
        sInsertDef = "INSERT INTO `" & sTableName & "` ("
        
        Dim aItems() As String
        Dim nItems As Integer
        ReDim aItems(0 To myRes.Fields.Count - 1)
    
        For nItems = 0 To myRes.Fields.Count - 1
            aItems(nItems) = "`" & myRes.Fields(nItems).Name & "`"
        Next
    
        sInsertDef = sInsertDef & Join(aItems, ",") & ") VALUES"
    
        Dim aValues() As String
        Dim nValues As Integer
        ReDim aValues(0 To 4999)
        
        While Not myRes.EOF
            For nItems = 0 To myRes.Fields.Count - 1
                If IsNull(myRes.Fields(nItems).Value) Then
                    aItems(nItems) = "NULL"
                Else
                    Select Case myRes.Fields(nItems).Type
                        Case 1
                            aItems(nItems) = IIf(myRes.Fields(nItems).Value, "TRUE", "FALSE")
                        Case 5, 6, 7
                            aItems(nItems) = "'" & Replace(myRes.Fields(nItems).Value, ",", ".") & "'"
                        Case 8
                            myDT = myRes.Fields(nItems).Value
        
                            On Error Resume Next ' no format selected defaults to datetime
                            sDTFmt = myRes.Fields(nItems).Properties("Format")
                            On Error GoTo 0
                            
                            Select Case sDTFmt
                                Case "Long Date", "Medium Date", "Short Date"
                                    aItems(nItems) = "'" & ExportSQL_P_GetDateStrFromDate(myDT) & "'"
                                Case "Long Time", "Medium Time", "Short Time"
                                    aItems(nItems) = "'" & ExportSQL_P_GetTimeStrFromDate(myDT) & "'"
                                Case Else
                                    aItems(nItems) = "'" & ExportSQL_P_GetDateStrFromDate(myDT) & " " & ExportSQL_P_GetTimeStrFromDate(myDT) & "'"
                            End Select
                        Case Else
                            aItems(nItems) = "'" & Replace(Replace(Replace(Replace(Replace(myRes.Fields(nItems).Value, "\", "\\"), "'", "\'"), Chr(34), "\" & Chr(34)) & "'", Chr(10), "\n"), Chr(13), "\r")
                    End Select
                End If
            Next
    
            aValues(nValues) = Chr(9) & "(" & Join(aItems, ",") & ")"
            nValues = nValues + 1
    
            If nValues >= 5000 Then
                ' flush
                ExportSQL_P_Write hFile, sInsertDef & vbCrLf & Join(aValues, "," & vbCrLf) & vbCrLf & Chr(9) & ";"
                nValues = 0
                DoEvents
            End If
    
            myRes.MoveNext
        Wend
    
        If nValues > 0 Then
            ReDim Preserve aValues(0 To nValues - 1)
            ExportSQL_P_Write hFile, sInsertDef & vbCrLf & Join(aValues, "," & vbCrLf) & vbCrLf & Chr(9) & ";"
        End If
        
        ExportSQL_P_Write hFile, ""
    End If

    myRes.Close
End Sub

Private Sub ExportSQL_P_TableWrite(hFile As Long, oTable As TableDef)
    Dim oField As Field
    Dim sFieldDef As String
    Dim oIndex As Index
    Dim sIndexDef As String
    Dim sDTFmt As String
    Dim sAutoInc As String

    Dim aIndexItems() As String
    Dim nIndexItems As Integer

    Dim aTableItems() As String
    Dim nTableItems As Integer
    ReDim aTableItems(0 To (oTable.Fields.Count + oTable.Indexes.Count) - 1)

    ExportSQL_P_Write hFile, "--"
    ExportSQL_P_Write hFile, "-- Table `" & oTable.Name & "` Definition"
    ExportSQL_P_Write hFile, "--"
    ExportSQL_P_Write hFile, "CREATE TABLE `" & oTable.Name & "` ("

    For Each oField In oTable.Fields
        sFieldDef = Chr(9) & "`" & oField.Name & "` "

        Select Case oField.Type
            Case 1 ' Yes/No (Bit)
                sFieldDef = sFieldDef & "BOOLEAN"
            Case 2 ' Number (Byte)
                sFieldDef = sFieldDef & "TINYINT(3) UNSIGNED"
            Case 3 ' Number (Integer)
                sFieldDef = sFieldDef & "SMALLINT(6)"
            Case 4 ' Number (Long)
                sFieldDef = sFieldDef & "INT(11)"
            Case 5 ' Currency
                sFieldDef = sFieldDef & "DECIMAL(15,4)"
            Case 6 ' Single
                sFieldDef = sFieldDef & "FLOAT"
            Case 7 ' Double
                sFieldDef = sFieldDef & "DOUBLE"
            Case 8 ' Date/Time
                On Error Resume Next ' no format selected defaults to datetime
                sDTFmt = oField.Properties("Format")
                On Error GoTo 0
                
                Select Case sDTFmt
                    Case "Long Date", "Medium Date", "Short Date"
                        sFieldDef = sFieldDef & "DATE"
                    Case "Long Time", "Medium Time", "Short Time"
                        sFieldDef = sFieldDef & "TIME"
                    Case Else
                        sFieldDef = sFieldDef & "DATETIME"
                End Select
            Case 9 ' Binary (hidden)
                sFieldDef = sFieldDef & "BINARY"
                Debug.Assert 0
            Case 10 ' Text
                sFieldDef = sFieldDef & "VARCHAR(" & oField.Size & ")"
            Case 11 ' OLE
                Debug.Assert 0
            Case 12 ' Memo/Hyperlink
                sFieldDef = sFieldDef & "TEXT"
            Case Else
                Debug.Assert 0
        End Select

        If oField.Required Then
            sFieldDef = sFieldDef & " NOT NULL"
        End If

        If oField.Attributes And dbAutoIncrField Then
            sFieldDef = sFieldDef & " AUTO_INCREMENT"
            sAutoInc = oField.Name
        End If

        aTableItems(nTableItems) = sFieldDef
        nTableItems = nTableItems + 1
    Next
    
    For Each oIndex In oTable.Indexes
        sIndexDef = Chr(9)
        
        If oIndex.Primary Then
            sIndexDef = sIndexDef & "PRIMARY "
        ElseIf oIndex.Unique Then
            sIndexDef = sIndexDef & "UNIQUE "
        End If

        sIndexDef = sIndexDef & "KEY `" & oIndex.Name & "` ("

        ReDim aIndexItems(0 To oIndex.Fields.Count - 1)
        nIndexItems = 0
        For Each oField In oIndex.Fields
            aIndexItems(nIndexItems) = "`" & oField.Name & "`"
            nIndexItems = nIndexItems + 1
        Next

        sIndexDef = sIndexDef & Join(aIndexItems, ",") & ")"

        aTableItems(nTableItems) = sIndexDef
        nTableItems = nTableItems + 1
    Next

    If sAutoInc <> "" Then
        Dim myRes As DAO.Recordset

        Set myRes = CurrentDb.OpenRecordset("SELECT MAX([" & sAutoInc & "])+1 AS [__AIV] FROM [" & oTable.Name & "];", dbOpenSnapshot)

        If myRes.RecordCount = 1 Then
            sAutoInc = " AUTO_INCREMENT=" & CLng(myRes![__AIV])
        Else
            sAutoInc = ""
        End If

        myRes.Close
    End If

    ExportSQL_P_Write hFile, Join(aTableItems, "," & vbCrLf)
    ExportSQL_P_Write hFile, ") ENGINE=InnoDB" & sAutoInc & " DEFAULT CHARSET=utf8;"
    ExportSQL_P_Write hFile, ""

    ExportSQL_P_TableDump hFile, oTable.Name
End Sub

' ExportSQL "", "dump.sql"
Public Sub ExportSQL(Optional sTable As String = "", Optional sFileName As String = "")
    Dim hFile As Long
    Dim nCount As Long
    Dim oTable As TableDef

    hFile = INVALID_HANDLE_VALUE

    If sFileName <> "" Then
        hFile = CreateFile(sFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)
    End If

    If sTable <> "" Then
        ExportSQL_P_TableWrite hFile, CurrentDb.TableDefs(sTable)
    Else
        ' tables
        For Each oTable In CurrentDb.TableDefs
            If LCase(Left(oTable.Name, 4)) <> "msys" Then ' all but system tables
                If sFileName <> "" Then Debug.Print oTable.Name
                ExportSQL_P_TableWrite hFile, oTable
                If sFileName <> "" Then Debug.Print "   OK"
                DoEvents
            End If
        Next
        ' relations
        If sFileName <> "" Then Debug.Print "<Relations>"
        ExportSQL_P_TableForeignKeys hFile
        If sFileName <> "" Then Debug.Print "   OK"
    End If

    If hFile <> INVALID_HANDLE_VALUE Then
        CloseHandle hFile
        hFile = INVALID_HANDLE_VALUE
    End If
End Sub
