Attribute VB_Name = "libmonetra_vb6"
Option Explicit

' M_ReturnStatus returns
Public Const M_SUCCESS As Long = 1
Public Const M_FAIL As Long = 0
Public Const M_ERROR As Long = -1
' M_CheckStatus returns
Public Const M_DONE As Long = 2
Public Const M_PENDING As Long = 1

Declare Function M_InitEngine Lib "libmonetra_stdcall" (ByVal cafile As String) As Long
Declare Sub M_DestroyEngine Lib "libmonetra_stdcall" ()
Declare Sub M_InitConn Lib "libmonetra_stdcall" (ByRef conn As Long)
Declare Sub M_DestroyConn Lib "libmonetra_stdcall" (ByRef conn As Long)
Declare Function M_SetSSL_CAfile Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal path As String) As Long
Declare Function M_SetSSL_Files Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal sslkeyfile As String, ByVal sslcertfile As String) As Long
Declare Function M_VerifySSLCert Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal tf As Long) As Long
Declare Function M_SetBlocking Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal tf As Long) As Long
Declare Function M_Connect Lib "libmonetra_stdcall" (ByRef conn As Long) As Long
Declare Function M_TransNew Lib "libmonetra_stdcall" (ByRef conn As Long) As Long
Declare Function M_TransKeyVal Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long, ByVal key As String, ByVal val As String) As Long
Declare Function M_TransSend Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_Monitor Lib "libmonetra_stdcall" (ByRef conn As Long) As Long
Declare Function M_CheckStatus Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_ReturnStatus Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_DeleteTrans Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
' map to different name so we can wrap it
Declare Function M_ResponseParam_int Lib "libmonetra_stdcall" Alias "M_ResponseParam" (ByRef conn As Long, ByVal id As Long, ByVal key As String) As Long
' map to different name so we can wrap it
Declare Function M_ConnectionError_int Lib "libmonetra_stdcall" Alias "M_ConnectionError" (ByRef conn As Long) As Long
Declare Function M_ResponseKeys Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long, ByRef num_keys As Long) As Long
' map to different name so we can wrap it
Declare Function M_ResponseKeys_index_int Lib "libmonetra_stdcall" Alias "M_ResponseKeys_index" (ByVal keys As Long, ByVal num_keys As Long, ByVal idx As Long) As Long
Declare Function M_FreeResponseKeys Lib "libmonetra_stdcall" (ByVal keys As Long, ByVal num_keys As Long) As Long
Declare Function M_IsCommaDelimited Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_ParseCommaDelimited Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
' map to different name so we can wrap it
Declare Function M_GetCell_int Lib "libmonetra_stdcall" Alias "M_GetCell" (ByRef conn As Long, ByVal id As Long, ByVal column As String, ByVal row As Long) As Long
Declare Function M_GetCellByNum_int Lib "libmonetra_stdcall" Alias "M_GetCellByNum" (ByRef conn As Long, ByVal id As Long, ByVal column As Long, ByVal row As Long) As Long
Declare Function M_GetHeader_int Lib "libmonetra_stdcall" Alias "M_GetHeader" (ByRef conn As Long, ByVal id As Long, ByVal column As Long) As Long
Declare Function M_NumRows Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_NumColumns Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal id As Long) As Long
Declare Function M_SetIP Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal host As String, ByVal port As Integer) As Long
Declare Function M_SetSSL Lib "libmonetra_stdcall" (ByRef conn As Long, ByVal host As String, ByVal port As Integer) As Long
' Functions to help convert Ansi C-Strings to VB Strings
Declare Function lstrlenA Lib "kernel32" (ByVal lpString As Long) As Long
Declare Function SysAllocStringByteLen Lib "oleaut32" (ByVal ansistr As Long, ByVal BLen As Long) As String

' Wrap this function to check for a NULL pointer being
' returned to avoid an exception
Function M_GetCell(ByRef conn As Long, ByVal id As Long, ByVal column As String, ByVal row As Long) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_GetCell_int(conn, id, column, row)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_GetCell = mystr
End Function

Function M_GetCellbyNum(ByRef conn As Long, ByVal id As Long, ByVal column As Long, ByVal row As Long) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_GetCellByNum_int(conn, id, column, row)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_GetCellbyNum = mystr
End Function

Function M_GetHeader(ByRef conn As Long, ByVal id As Long, ByVal column As Long) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_GetHeader_int(conn, id, column)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_GetHeader = mystr
End Function

Function M_ResponseKeys_index(ByVal keys As Long, ByVal num_keys As Long, ByVal idx As Long) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_ResponseKeys_index_int(keys, num_keys, idx)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_ResponseKeys_index = mystr
End Function

Function M_ResponseParam(ByRef conn As Long, ByVal id As Long, ByVal key As String) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_ResponseParam_int(conn, id, key)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_ResponseParam = mystr
End Function

Function M_ConnectionError(ByRef conn As Long) As String
        Dim retval As Long
        Dim mystr As String

        mystr = ""
        retval = M_ConnectionError_int(conn)
        If Not retval = 0 Then
                mystr = SysAllocStringByteLen(retval, lstrlenA(retval))
        End If
        M_ConnectionError = mystr
End Function

Sub RunTrans(ByVal conn As Long)
        Dim retval As Long
        Dim id As Long
        Dim i As Long
        Dim key As String
        Dim num_keys As Long
        Dim keys As Long
        Dim output As String

        id = M_TransNew(conn)
        M_TransKeyVal conn, id, "username", "vitale"
        M_TransKeyVal conn, id, "password", "test"
        M_TransKeyVal conn, id, "action", "sale"
        M_TransKeyVal conn, id, "account", "4012888888881881"
        M_TransKeyVal conn, id, "expdate", "0512"
        M_TransKeyVal conn, id, "amount", "12.00"
        If M_TransSend(conn, id) = 0 Then
                MsgBox ("Failed to send trans")
                Exit Sub
        End If

        output = ""
        retval = M_ReturnStatus(conn, id)
        If Not retval = M_SUCCESS Then
                output = "Bad return status: " & retval & Chr(13) & Chr(10)
        End If
        output = output & "code: " & M_ResponseParam(conn, id, "code") & Chr(13) & Chr(10)
        output = output & "verbiage: " & M_ResponseParam(conn, id, "verbiage") & Chr(13) & Chr(10)

        keys = M_ResponseKeys(conn, id, num_keys)

        output = output & "All " & num_keys & " response parameters:" & Chr(13) & Chr(10)

        For i = 0 To num_keys - 1
                key = M_ResponseKeys_index(keys, num_keys, i)
                output = output & key & "=" & M_ResponseParam(conn, id, key) & Chr(13) & Chr(10)
        Next
        M_FreeResponseKeys keys, num_keys
        M_DeleteTrans conn, id

        MsgBox (output)
End Sub

Sub RunReport(ByVal conn As Long)
        Dim id As Long
        Dim retval As Long
        Dim rows As Long
        Dim cols As Long
        Dim i As Long
        Dim j As Long
        Dim line As String
        Dim output As String

        id = M_TransNew(conn)
        M_TransKeyVal conn, id, "username", "vitale"
        M_TransKeyVal conn, id, "password", "test"
        M_TransKeyVal conn, id, "action", "admin"
        M_TransKeyVal conn, id, "admin", "gut"
        If M_TransSend(conn, id) = 0 Then
                MsgBox ("Failed to send trans: " & M_ConnectionError(conn))
                Exit Sub
        End If

        output = ""

        retval = M_ReturnStatus(conn, id)
        If Not retval = M_SUCCESS Then
                output = "Bad return status: " & retval
		Exit Sub
        End If
        M_ParseCommaDelimited conn, id

        rows = M_NumRows(conn, id)
        cols = M_NumColumns(conn, id)

        output = output & "Report Data (" & rows & " rows, " & cols & " cols):" & Chr(13) & Chr(10)

        line = ""
        For i = 0 To cols - 1
                If Not i = 0 Then
                        line = line + "|"
                End If
                line = line + M_GetHeader(conn, id, i)
        Next
        output = output & line & Chr(13) & Chr(10)

        For i = 0 To rows - 1
                line = ""
                For j = 0 To cols - 1
                        If Not j = 0 Then
                                line = line + "|"
                        End If
                        line = line & M_GetCellbyNum(conn, id, j, i)
                Next
                output = output & line & Chr(13) & Chr(10)
        Next

        M_DeleteTrans conn, id
        MsgBox (output)
End Sub

Sub Main()
    Dim conn As Long
    Dim error As Long
    
    M_InitEngine ("")
    M_InitConn conn
    If (conn = 0) Then
        MsgBox "Initialization failed"
        Exit Sub
    End If
    error = M_SetSSL(conn, "testbox.monetra.com", 8665)
    M_SetBlocking conn, 1
    If M_Connect(conn) = 0 Then
            MsgBox ("M_Connect() failed" & M_ConnectionError(conn))
            Exit Sub
    End If

    RunTrans conn
    RunReport conn

    M_DestroyConn conn
    M_DestroyEngine
        
End Sub

