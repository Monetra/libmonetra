Option Explicit On
Option Strict On
Imports System
Imports System.String
Imports System.Runtime.InteropServices

Public Class Monetra
    Declare Ansi Function M_InitEngine Lib "libmonetra.dll" (ByVal cafile As String) As Integer
    Declare Ansi Sub M_DestroyEngine Lib "libmonetra.dll" ()
    Declare Ansi Sub M_InitConn Lib "libmonetra.dll" (ByRef conn As IntPtr)
    Declare Ansi Sub M_DestroyConn Lib "libmonetra.dll" (ByRef conn As IntPtr)
    Declare Ansi Function M_SetIP Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal host As String, ByVal port As Short) As Integer
    Declare Ansi Function M_SetSSL Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal host As String, ByVal port As Short) As Integer
    Declare Ansi Function M_SetSSL_CAfile Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal path As String) As Integer
    Declare Ansi Function M_SetSSL_Files Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal sslkeyfile As String, ByVal sslcertfile As String) As Integer
    Declare Ansi Function M_VerifySSLCert Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal tf As Integer) As Integer
    Declare Ansi Function M_SetBlocking Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal tf As Integer) As Integer
    Declare Ansi Function M_Connect Lib "libmonetra.dll" (ByRef conn As IntPtr) As Integer
    Declare Ansi Function M_TransNew Lib "libmonetra.dll" (ByRef conn As IntPtr) As IntPtr
    Declare Ansi Function M_TransKeyVal Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr, ByVal key As String, ByVal val As String) As Integer
    Declare Ansi Function M_TransSend Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    Declare Ansi Function M_Monitor Lib "libmonetra.dll" (ByRef conn As IntPtr) As Integer
    Declare Ansi Function M_CheckStatus Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    Declare Ansi Function M_ReturnStatus Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    Declare Ansi Function M_DeleteTrans Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    ' map to different name so we can wrap it
    Declare Ansi Function M_ResponseParam_int Lib "libmonetra.dll" Alias "M_ResponseParam" (ByRef conn As IntPtr, ByVal id As IntPtr, ByVal key As String) As IntPtr
    ' map to different name so we can wrap it
    Declare Ansi Function M_ConnectionError_int Lib "libmonetra.dll" Alias "M_ConnectionError" (ByRef conn As IntPtr) As IntPtr
    Declare Ansi Function M_ResponseKeys Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr, ByRef num_keys As Integer) As IntPtr
    ' map to different name so we can wrap it
    Declare Ansi Function M_ResponseKeys_index_int Lib "libmonetra.dll" Alias "M_ResponseKeys_index" (ByVal keys As IntPtr, ByVal num_keys As Integer, ByVal idx As Integer) As IntPtr
    Declare Ansi Function M_FreeResponseKeys Lib "libmonetra.dll" (ByVal keys As IntPtr, ByVal num_keys As Integer) As Integer
    Declare Ansi Function M_IsCommaDelimited Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    Declare Ansi Function M_ParseCommaDelimited Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    ' map to different name so we can wrap it
    Declare Ansi Function M_GetCell_int Lib "libmonetra.dll" Alias "M_GetCell" (ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As String, ByVal row As Integer) As IntPtr
    Declare Ansi Function M_GetCellByNum_int Lib "libmonetra.dll" Alias "M_GetCellByNum" (ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As Integer, ByVal row As Integer) As IntPtr
    Declare Ansi Function M_GetHeader_int Lib "libmonetra.dll" Alias "M_GetHeader" (ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As Integer) As IntPtr
    Declare Ansi Function M_NumRows Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer
    Declare Ansi Function M_NumColumns Lib "libmonetra.dll" (ByRef conn As IntPtr, ByVal id As IntPtr) As Integer


    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_GetCell(ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As String, ByVal row As Integer) As String
        Dim ret As IntPtr
        ret = M_GetCell_int(conn, id, column, row)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function

    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_GetCellbyNum(ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As Integer, ByVal row As Integer) As String
        Dim ret As IntPtr
        ret = M_GetCellByNum_int(conn, id, column, row)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function

    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_GetHeader(ByRef conn As IntPtr, ByVal id As IntPtr, ByVal column As Integer) As String
        Dim ret As IntPtr
        ret = M_GetHeader_int(conn, id, column)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function

    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_ResponseKeys_index(ByVal keys As IntPtr, ByVal num_keys As Integer, ByVal idx As Integer) As String
        Dim ret As IntPtr
        ret = M_ResponseKeys_index_int(keys, num_keys, idx)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function

    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_ResponseParam(ByRef conn As IntPtr, ByVal id As IntPtr, ByVal key As String) As String
        Dim ret As IntPtr
        ret = M_ResponseParam_int(conn, id, key)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function
    ' Wrap this function to check for a NULL pointer being
    ' returned to avoid an exception
    Public Shared Function M_ConnectionError(ByRef conn As IntPtr) As String
        Dim ret As IntPtr
        ret = M_ConnectionError_int(conn)
        If ret.ToInt64 = 0 Then
            Return ""
        End If
        Return Marshal.PtrToStringAnsi(ret)
    End Function
    ' M_ReturnStatus returns
    Public Const M_SUCCESS As Integer = 1
    Public Const M_FAIL As Integer = 0
    Public Const M_ERROR As Integer = -1
    ' M_CheckStatus returns
    Public Const M_DONE As Integer = 2
    Public Const M_PENDING As Integer = 1
End Class

Module Module1
    Sub RunTrans(ByVal conn As IntPtr)
        Dim retval As Integer
        Dim id As IntPtr
        Dim i As Integer
        Dim key As String
        Dim num_keys As Integer
        Dim keys As IntPtr
        Console.WriteLine("Running transaction....")
        id = Monetra.M_TransNew(conn)
        Monetra.M_TransKeyVal(conn, id, "username", "vitale")
        Monetra.M_TransKeyVal(conn, id, "password", "test")
        Monetra.M_TransKeyVal(conn, id, "action", "sale")
        Monetra.M_TransKeyVal(conn, id, "account", "4012888888881")
        Monetra.M_TransKeyVal(conn, id, "expdate", "0512")
        Monetra.M_TransKeyVal(conn, id, "amount", "12.00")
        If Monetra.M_TransSend(conn, id) = 0 Then
            Console.WriteLine("Failed to send trans")
            Return
        End If
        retval = Monetra.M_ReturnStatus(conn, id)
        If retval <> Monetra.M_SUCCESS Then
            Console.WriteLine("Bad return status: " + retval.ToString())
        End If
        Console.WriteLine("code: " + Monetra.M_ResponseParam(conn, id, "code"))
        Console.WriteLine("verbiage: " + Monetra.M_ResponseParam(conn, id, "verbiage"))
        keys = Monetra.M_ResponseKeys(conn, id, num_keys)
        Console.WriteLine("All " + num_keys.ToString() + " response parameters:")
        For i = 0 To num_keys - 1
            key = Monetra.M_ResponseKeys_index(keys, num_keys, i)
            Console.WriteLine(key + "=" + Monetra.M_ResponseParam(conn, id, key))
        Next
        Monetra.M_FreeResponseKeys(keys, num_keys)
        Monetra.M_DeleteTrans(conn, id)
        Console.WriteLine("Transaction Done")
    End Sub

    Sub RunReport(ByVal conn As IntPtr)
        Dim id As IntPtr
        Dim retval As Integer
        Dim rows As Integer
        Dim cols As Integer
        Dim i As Integer
        Dim j As Integer
        Dim line As String

        Console.WriteLine("Running report....")
        id = Monetra.M_TransNew(conn)
        Monetra.M_TransKeyVal(conn, id, "username", "vitale")
        Monetra.M_TransKeyVal(conn, id, "password", "test")
        Monetra.M_TransKeyVal(conn, id, "action", "admin")
        Monetra.M_TransKeyVal(conn, id, "admin", "gut")
        If Monetra.M_TransSend(conn, id) = 0 Then
            Console.WriteLine("Failed to send trans")
            Return
        End If
        retval = Monetra.M_ReturnStatus(conn, id)
        If retval <> Monetra.M_SUCCESS Then
            Console.WriteLine("Bad return status: " + retval.ToString())
        End If
        Monetra.M_ParseCommaDelimited(conn, id)

        rows = Monetra.M_NumRows(conn, id)
        cols = Monetra.M_NumColumns(conn, id)

        Console.WriteLine("Report Data (" + rows.ToString() + " rows, " + cols.ToString() + " cols):")

        line = ""
        For i = 0 To cols - 1
            If Not i = 0 Then
                line = line + "|"
            End If
            line = line + Monetra.M_GetHeader(conn, id, i)
        Next
        Console.WriteLine(line)
        For i = 0 To rows - 1
            line = ""
            For j = 0 To cols - 1
                If Not j = 0 Then
                    line = line + "|"
                End If
                line = line + Monetra.M_GetCellbyNum(conn, id, j, i)
            Next
            Console.WriteLine(line)
        Next

        Monetra.M_DeleteTrans(conn, id)
        Console.WriteLine("Report Done")
    End Sub

    Public Sub Main()
        Dim conn As IntPtr

        Monetra.M_InitEngine("")
        Monetra.M_InitConn(conn)
        Monetra.M_SetSSL(conn, "testbox.monetra.com", 8665)
        Monetra.M_SetBlocking(conn, 1)
        If Monetra.M_Connect(conn) = 0 Then
            Console.WriteLine("M_Connect() failed" + Monetra.M_ConnectionError(conn))
            Return
        End If

        RunTrans(conn)
        RunReport(conn)

        Monetra.M_DestroyConn(conn)
        Monetra.M_DestroyEngine()
        Return
    End Sub

End Module
