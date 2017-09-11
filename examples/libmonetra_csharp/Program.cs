using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

public class Monetra
{
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_InitEngine(string cafile);
	[DllImport("libmonetra.dll")]
		unsafe public static extern void M_DestroyEngine();
	[DllImport("libmonetra.dll")]
		unsafe public static extern void M_InitConn(IntPtr *conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern void M_DestroyConn(IntPtr* conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_SetIP(IntPtr* conn, string host, ushort port);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_SetSSL(IntPtr* conn, string host, ushort port);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_SetSSL_CAfile(IntPtr* conn, string path);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_SetSSL_Files(IntPtr* conn, string sslkeyfile, string sslcertfile);
	[DllImport("libmonetra.dll")]
		unsafe public static extern void M_VerifySSLCert(IntPtr* conn, int tf);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_SetBlocking(IntPtr *conn, int tf);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_Connect(IntPtr *conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern IntPtr M_TransNew(IntPtr *conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_TransKeyVal(IntPtr *conn, IntPtr id, string key, string val);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_TransSend(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_Monitor(IntPtr *conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_CheckStatus(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_ReturnStatus(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll")]
		unsafe public static extern void M_DeleteTrans(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll", EntryPoint="M_ResponseParam")]
		unsafe public static extern IntPtr M_ResponseParam_int(IntPtr *conn, IntPtr id, string key);
	[DllImport("libmonetra.dll", EntryPoint="M_ConnectionError")]
		unsafe public static extern IntPtr M_ConnectionError_int(IntPtr *conn);
	[DllImport("libmonetra.dll")]
		unsafe public static extern IntPtr M_ResponseKeys(IntPtr *conn, IntPtr id, int *num_keys);
	[DllImport("libmonetra.dll", EntryPoint="M_ResponseKeys_index")]
		unsafe public static extern IntPtr M_ResponseKeys_index_int(IntPtr keys, int num_keys, int idx);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_FreeResponseKeys(IntPtr keys, int num_keys);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_IsCommaDelimited(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_ParseCommaDelimited(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll", EntryPoint="M_GetCell")]
		unsafe public static extern IntPtr M_GetCell_int(IntPtr *conn, IntPtr id, string column, int row);
	[DllImport("libmonetra.dll", EntryPoint="M_GetCellByNum")]
		unsafe public static extern IntPtr M_GetCellByNum_int(IntPtr *conn, IntPtr id, int column, int row);
	[DllImport("libmonetra.dll", EntryPoint="M_GetHeader")]
		unsafe public static extern IntPtr M_GetHeader_int(IntPtr *conn, IntPtr id, int column);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_NumRows(IntPtr *conn, IntPtr id);
	[DllImport("libmonetra.dll")]
		unsafe public static extern int M_NumColumns(IntPtr *conn, IntPtr id);

	unsafe public static string M_ResponseParam(IntPtr *conn, IntPtr id, string key)
	{
		return (Marshal.PtrToStringAnsi(M_ResponseParam_int(conn, id, key)));
	}
	unsafe public static string M_ConnectionError(IntPtr *conn)
	{
		return (Marshal.PtrToStringAnsi(M_ConnectionError_int(conn)));
	}
	unsafe public static string M_ResponseKeys_index(IntPtr keys, int num_keys, int idx)
	{
		return (Marshal.PtrToStringAnsi(M_ResponseKeys_index_int(keys, num_keys, idx)));
	}
	unsafe public static string M_GetCell(IntPtr *conn, IntPtr id, string column, int row)
	{
		return(Marshal.PtrToStringAnsi(M_GetCell_int(conn, id, column, row)));
	}
	unsafe public static string M_GetCellByNum(IntPtr *conn, IntPtr id, int column, int row)
	{
		return (Marshal.PtrToStringAnsi(M_GetCellByNum_int(conn, id, column, row)));
	}
	unsafe public static string M_GetHeader(IntPtr *conn, IntPtr id, int column)
	{
		return (Marshal.PtrToStringAnsi(M_GetHeader_int(conn, id, column)));
	}

	/* M_ReturnStatus codes */
	public const int M_SUCCESS	= 1;
	public const int M_FAIL		= 0;
	public const int M_ERROR	= -1;

	/* M_CheckStatus codes */
	public const int M_DONE		= 2;
	public const int M_PENDING	= 1;
}

namespace libmonetra_csharp
{
	public class Program
	{
		unsafe static int RunTrans(IntPtr *conn)
		{
			int retval;
			IntPtr id;
			IntPtr keys;
			string key;
			int num_keys;
			int i;

			Console.WriteLine("Running transaction...");
			id = Monetra.M_TransNew(conn);
			Monetra.M_TransKeyVal(conn, id, "username", "vitale");
			Monetra.M_TransKeyVal(conn, id, "password", "test");
			Monetra.M_TransKeyVal(conn, id, "action", "sale");
			Monetra.M_TransKeyVal(conn, id, "account", "4012888888881");
			Monetra.M_TransKeyVal(conn, id, "expdate", "0512");
			Monetra.M_TransKeyVal(conn, id, "amount", "12.00");
			if (Monetra.M_TransSend(conn, id) == 0) {
				Console.WriteLine("Failed to send trans: " + Monetra.M_ConnectionError(conn));
				return(1);
			}
			retval = Monetra.M_ReturnStatus(conn, id);
			if (retval != Monetra.M_SUCCESS) {
				Console.WriteLine("Bad return status: " + retval);
			}
			/* print results ... */
			Console.WriteLine("code: " + Monetra.M_ResponseParam(conn, id, "code"));
			Console.WriteLine("verbiage: " + Monetra.M_ResponseParam(conn, id, "verbiage"));

			keys = Monetra.M_ResponseKeys(conn, id, &num_keys);
			Console.WriteLine("All " + num_keys + " repsonse parameters:");
			for (i=0; i<num_keys; i++) {
				key = Monetra.M_ResponseKeys_index(keys, num_keys, i);
				Console.WriteLine(key + "=" + Monetra.M_ResponseParam(conn, id, key));
			}
			Monetra.M_FreeResponseKeys(keys, num_keys);
			Monetra.M_DeleteTrans(conn, id);
			Console.WriteLine("Transaction Done");
			return(0);
		}

		unsafe static int RunReport(IntPtr *conn)
		{
			IntPtr id;
			int retval;
			int rows, cols, i, j;
			string line;

			Console.WriteLine("Running report....");
			id = Monetra.M_TransNew(conn);
			Monetra.M_TransKeyVal(conn, id, "username", "vitale");
			Monetra.M_TransKeyVal(conn, id, "password", "test");
			Monetra.M_TransKeyVal(conn, id, "action", "admin");
			Monetra.M_TransKeyVal(conn, id, "admin", "gut");
			if (Monetra.M_TransSend(conn, id) == 0) {
				Console.WriteLine("Failed to send trans: " + Monetra.M_ConnectionError(conn));
				return(1);
			}
			retval = Monetra.M_ReturnStatus(conn, id);
			if (retval != Monetra.M_SUCCESS) {
				Console.WriteLine("Bad return status: " + retval);
			}
			Monetra.M_ParseCommaDelimited(conn, id);

			rows = Monetra.M_NumRows(conn, id);
			cols = Monetra.M_NumColumns(conn, id);

			Console.WriteLine("Report Data (" + rows + " rows, " + cols + " cols):");
			line = "";
			for (i=0; i<cols; i++) {
				if (i != 0) line = line + "|";
				line = line + Monetra.M_GetHeader(conn, id, i);
			}
			Console.WriteLine(line);
			for (i=0; i<rows; i++) {
				line = "";
				for (j=0; j<cols; j++) {
					if (j != 0) line = line + "|";
					line = line + Monetra.M_GetCellByNum(conn, id, j, i);
				}
				Console.WriteLine(line);
			}
			Monetra.M_DeleteTrans(conn, id);
			Console.WriteLine("Report Done");
			return (0);
		}

		unsafe static int Main(string[] args)
		{
			IntPtr conn;

			Monetra.M_InitEngine(null);
			Monetra.M_InitConn(&conn);
			Monetra.M_SetSSL(&conn, "testbox.monetra.com", 8665);
			Monetra.M_SetBlocking(&conn, 1);
			if (Monetra.M_Connect(&conn) == 0) {
				Console.WriteLine("Failed to connect: " + Monetra.M_ConnectionError(&conn));
				return(1);
			}

			RunTrans(&conn);
			RunReport(&conn);

			Monetra.M_DestroyConn(&conn);
			Monetra.M_DestroyEngine();
			return (0);
		}
	}
}

