VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Matches"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private m_IMatches As Object

Public Sub OnInit(IMatches As Object)
    Set m_IMatches = IMatches
End Sub

Public Function Count() As Long
    Count = m_IMatches.Count
End Function

Public Function Item(Idx As Variant) As Match
Attribute Item.VB_UserMemId = 0
    Set Item = New Match

    Item.OnInit m_IMatches(Idx)
End Function

Public Function NewEnum() As IUnknown
Attribute NewEnum.VB_UserMemId = -4
Attribute NewEnum.VB_MemberFlags = "40"
    Set NewEnum = m_IMatches.[_NewEnum]
End Function
