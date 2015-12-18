VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "RegExp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private m_RegExp As Object

Private Sub Class_Initialize()
    Set m_RegExp = CreateObject("VBScript.RegExp")
End Sub

Private Sub Class_Terminate()
    Set m_RegExp = Nothing
End Sub

Public Function Execute(sSubject As String) As Matches
    Set Execute = New Matches
    
    Execute.OnInit m_RegExp.Execute(sSubject)
End Function

Public Function Replace(sSubject As String, sReplacement As String) As String
    Let Replace = m_RegExp.Replace(sSubject, sReplacement)
End Function

Public Function Test(sSubject As String) As Boolean
    Let Test = m_RegExp.Test(sSubject)
End Function

Public Property Get IsGlobal() As Boolean
    IsGlobal = m_RegExp.Global
End Property

Public Property Let IsGlobal(bValue As Boolean)
    m_RegExp.Global = bValue
End Property

Public Property Get IgnoreCase() As Boolean
    IgnoreCase = m_RegExp.IgnoreCase
End Property

Public Property Let IgnoreCase(bValue As Boolean)
    m_RegExp.IgnoreCase = bValue
End Property

Public Property Get Pattern() As String
    Pattern = m_RegExp.Pattern
End Property

Public Property Let Pattern(sValue As String)
    m_RegExp.Pattern = sValue
End Property

Public Property Get FLOAT_NUMBER() As String
    FLOAT_NUMBER = "([\-\+]?(?:\d+(?:[\.,]\d*)?|\d*[\.,]\d+))"
End Property

Public Property Get UNSIGNED_FLOAT_NUMBER() As String
    UNSIGNED_FLOAT_NUMBER = "(\d+(?:[\.,]\d*)?|\d*[\.,]\d+)"
End Property

