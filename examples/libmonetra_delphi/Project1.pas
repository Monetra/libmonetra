// The example below demonstrates how to call libmonetra.dll from Borland Delphi (Tested with
// version 2005). To use this example, create a console project name Project1 and use the code
// below as Project1.pas . When you execute the test, it will connect over the Internet to our test
// server at test.transafe.com and run the transaction. Notice each function from libmonetra
// must be prototyped for Delphi. Please reference the C prototypes in the previous sections to see
// how different variable types map.
//
// Precompiled Windows versions of libmonetra can be obtained from:
//   https://download.monetra.com/APIs/libmonetra/binaries/windows/
program Project1;

{$APPTYPE CONSOLE}

{$R *.RES}

type
	M_CONN = Pointer;
	M_UINTPTR = LongWord;

var
	conn: M_CONN;
	id: M_UINTPTR;

const
	host = 'test.transafe.com';
	port = 8665;

function M_InitEngine(CAfile: PChar): Boolean; cdecl; external 'libmonetra.dll';
function M_InitConn(var conn: M_CONN): Boolean; cdecl; external 'libmonetra.dll';
function M_SetIP(var conn: M_CONN; host: PChar; port: Integer): Boolean; cdecl; external 'libmonetra.dll';
function M_SetBlocking(var conn: M_CONN; tf: Integer): Boolean; cdecl; external 'libmonetra.dll';
function M_ConnectionError(var conn: M_CONN): PChar; cdecl; external 'libmonetra.dll';
function M_Connect(var conn: M_CONN): Boolean; cdecl; external 'libmonetra.dll';
function M_TransNew(var conn: M_CONN): M_UINTPTR; cdecl; external 'libmonetra.dll';
function M_TransKeyVal(var conn: M_CONN; id: M_UINTPTR; key: PChar; val: PChar): Boolean; cdecl; external 'libmonetra.dll';
function M_TransSend(var conn: M_CONN; id: M_UINTPTR): Boolean; cdecl; external 'libmonetra.dll';
procedure M_DestroyConn(var conn: M_CONN); cdecl; external 'libmonetra.dll';
procedure M_DestroyEngine(); cdecl; external 'libmonetra.dll';
function M_ResponseParam(var conn: M_CONN; id: M_UINTPTR; key: PChar): PChar; cdecl; external 'libmonetra.dll';

function Int2Str(Number : Int64) : String;
var Minus : Boolean;
begin
	{
		SysUtils is not in the Uses clause so I can
		not use IntToStr() so I have to
		define an Int2Str function here
	}
	Result := '';
	if Number = 0 then
		Result := '0';
	Minus := Number < 0;
	if Minus then
		Number := -Number;
	while Number > 0 do
	begin
		Result := Char((Number mod 10) + Integer('0')) + Result;
		Number := Number div 10;
	end;
	if Minus then
		Result := '-' + Result;
end;

begin // main program begin
	M_InitEngine(nil);
	M_InitConn(conn);
	M_SetSSL(conn, PChar(host), port);
	M_SetBlocking(conn, 1);
	Writeln('Connecting to ' + host + ':' + Int2Str(port) + '...');
	if not M_Connect(conn) then
	begin
		Writeln('Could not connect to ' + host + ':' + Int2Str(port));
		M_DestroyConn(conn);
		M_DestroyEngine();
		Halt;
	end;
	id := M_TransNew(conn);
	M_TransKeyVal(conn, id, PChar('username'), PChar('test_retail:public'));
	M_TransKeyVal(conn, id, PChar('password'), PChar('publ1ct3st'));
	M_TransKeyVal(conn, id, PChar('action'), PChar('sale'));
	M_TransKeyVal(conn, id, PChar('account'), PChar('4012888888881881'));
	M_TransKeyVal(conn, id, PChar('expdate'), PChar('0525'));
	M_TransKeyVal(conn, id, PChar('amount'), PChar('12.00'));
	M_TransKeyVal(conn, id, PChar('zip'), PChar('32606'));
	Writeln('Running Transaction...');
	if not M_TransSend(conn, id) then
	begin
		Writeln('Connectivity Error: ' + M_ConnectionError(conn));
		M_DestroyConn(conn);
		M_DestroyEngine();
		Halt;
	end;

	Writeln('Code : ' + M_ResponseParam(conn, id, PChar('code')));
	Writeln('Verbiage: ' + M_ResponseParam(conn, id, PChar('verbiage')));
	Writeln('Item : ' + M_ResponseParam(conn, id, PChar('item')));
	Writeln('Batch : ' + M_ResponseParam(conn, id, PChar('batch')));
	Writeln('TTID : ' + M_ResponseParam(conn, id, PChar('ttid')));
	M_DestroyConn(conn);
	M_DestroyEngine();
	Halt;
end.
