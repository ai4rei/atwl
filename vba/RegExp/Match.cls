VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Match"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private m_IMatch As Object

Public Sub OnInit(IMatch As Object)
    Set m_IMatch = IMatch
End Sub

Public Function SubMatches() As SubMatches
    Set SubMatches = New SubMatches

    SubMatches.OnInit m_IMatch.SubMatches
End Function

Public Property Get FirstIndex() As Long
Attribute FirstIndex.VB_Description = "Returns index of the first character of the match."
    FirstIndex = m_IMatch.FirstIndex
End Property

Public Property Get Length() As Long
    Length = m_IMatch.Length
End Property

Public Property Get Value() As String
Attribute Value.VB_UserMemId = 0
    Value = m_IMatch.Value
End Property
