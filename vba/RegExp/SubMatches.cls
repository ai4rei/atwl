VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "SubMatches"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private m_ISubMatches As Object

Public Sub OnInit(ISubMatches As Object)
    Set m_ISubMatches = ISubMatches
End Sub

Public Function Count() As Long
    Count = m_ISubMatches.Count
End Function

Public Function Item(Idx As Variant) As String
Attribute Item.VB_UserMemId = 0
    Item = m_ISubMatches.Item(Idx)
End Function

Public Function NewEnum() As IUnknown
Attribute NewEnum.VB_UserMemId = -4
Attribute NewEnum.VB_MemberFlags = "40"
    Set NewEnum = m_ISubMatches.[_NewEnum]
End Function
