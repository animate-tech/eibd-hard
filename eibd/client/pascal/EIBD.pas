Unit EIBD;
{
    EIBD client library
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
}

Interface

{$mode objfpc}

Type 
  TEIBAddr = Word;
  TByteArray = Array Of Byte;
  TUINT32 = Longword;
  TUINT16 = Word;
  TUINT8 = Byte;
  TEIBKey = Array[0..4] Of Byte;
  PEIBAddr = ^TEIBAddr;
  PUINT32 = ^TUINT32;
  PUINT16 = ^TUINT16;
  PUINT8 = ^TUINT8;
  PByteArray = ^TByteArray;

  Complete_Function = Function : Integer Of object;




Const IMG_UNKNOWN_ERROR = 0;

Const IMG_UNRECOG_FORMAT = 1;

Const IMG_INVALID_FORMAT = 2;

Const IMG_NO_BCUTYPE = 3;

Const IMG_UNKNOWN_BCUTYPE = 4;

Const IMG_NO_CODE = 5;

Const IMG_NO_SIZE = 6;

Const IMG_LODATA_OVERFLOW = 7;

Const IMG_HIDATA_OVERFLOW = 8;

Const IMG_TEXT_OVERFLOW = 9;

Const IMG_NO_ADDRESS = 10;

Const IMG_WRONG_SIZE = 11;

Const IMG_IMAGE_LOADABLE = 12;

Const IMG_NO_DEVICE_CONNECTION = 13;

Const IMG_MASK_READ_FAILED = 14;

Const IMG_WRONG_MASK_VERSION = 15;

Const IMG_CLEAR_ERROR = 16;

Const IMG_RESET_ADDR_TAB = 17;

Const IMG_LOAD_HEADER = 18;

Const IMG_LOAD_MAIN = 19;

Const IMG_ZERO_RAM = 20;

Const IMG_FINALIZE_ADDR_TAB = 21;

Const IMG_PREPARE_RUN = 22;

Const IMG_RESTART = 23;

Const IMG_LOADED = 24;

Const IMG_NO_START = 25;

Const IMG_WRONG_ADDRTAB = 26;

Const IMG_ADDRTAB_OVERFLOW = 27;

Const IMG_OVERLAP_ASSOCTAB = 28;

Const IMG_OVERLAP_TEXT = 29;

Const IMG_NEGATIV_TEXT_SIZE = 30;

Const IMG_OVERLAP_PARAM = 31;

Const IMG_OVERLAP_EEPROM = 32;

Const IMG_OBJTAB_OVERFLOW = 33;

Const IMG_WRONG_LOADCTL = 34;

Const IMG_UNLOAD_ADDR = 35;

Const IMG_UNLOAD_ASSOC = 36;

Const IMG_UNLOAD_PROG = 37;

Const IMG_LOAD_ADDR = 38;

Const IMG_WRITE_ADDR = 39;

Const IMG_SET_ADDR = 40;

Const IMG_FINISH_ADDR = 41;

Const IMG_LOAD_ASSOC = 42;

Const IMG_WRITE_ASSOC = 43;

Const IMG_SET_ASSOC = 44;

Const IMG_FINISH_ASSOC = 45;

Const IMG_LOAD_PROG = 46;

Const IMG_ALLOC_LORAM = 47;

Const IMG_ALLOC_HIRAM = 48;

Const IMG_ALLOC_INIT = 49;

Const IMG_ALLOC_RO = 50;

Const IMG_ALLOC_EEPROM = 51;

Const IMG_ALLOC_PARAM = 52;

Const IMG_SET_PROG = 53;

Const IMG_SET_TASK_PTR = 54;

Const IMG_SET_OBJ = 55;

Const IMG_SET_TASK2 = 56;

Const IMG_FINISH_PROC = 57;

Const IMG_WRONG_CHECKLIM = 58;

Const IMG_INVALID_KEY = 59;

Const IMG_AUTHORIZATION_FAILED = 60;

Const IMG_KEY_WRITE = 61;


Type 
  EIBConnection = Class

    Private Function EIBGetAPDU_Complete: Integer;
    Public Function EIBGetAPDU_async (buf: PByteArray): Integer;
    Public Function EIBGetAPDU (buf: PByteArray): Integer;
    Private Function EIBGetAPDU_Src_Complete: Integer;
    Public Function EIBGetAPDU_Src_async (buf: PByteArray; src: PEIBAddr): Integer;
    Public Function EIBGetAPDU_Src (buf: PByteArray; src: PEIBAddr): Integer;
    Private Function EIBGetBusmonitorPacket_Complete: Integer;
    Public Function EIBGetBusmonitorPacket_async (buf: PByteArray): Integer;
    Public Function EIBGetBusmonitorPacket (buf: PByteArray): Integer;
    Private Function EIBGetGroup_Src_Complete: Integer;
    Public Function EIBGetGroup_Src_async (buf: PByteArray; src: PEIBAddr; dest: PEIBAddr): Integer;
    Public Function EIBGetGroup_Src (buf: PByteArray; src: PEIBAddr; dest: PEIBAddr): Integer;
    Private Function EIBGetTPDU_Complete: Integer;
    Public Function EIBGetTPDU_async (buf: PByteArray; src: PEIBAddr): Integer;
    Public Function EIBGetTPDU (buf: PByteArray; src: PEIBAddr): Integer;
    Private Function EIB_Cache_Clear_Complete: Integer;
    Public Function EIB_Cache_Clear_async (): Integer;
    Public Function EIB_Cache_Clear (): Integer;
    Private Function EIB_Cache_Disable_Complete: Integer;
    Public Function EIB_Cache_Disable_async (): Integer;
    Public Function EIB_Cache_Disable (): Integer;
    Private Function EIB_Cache_Enable_Complete: Integer;
    Public Function EIB_Cache_Enable_async (): Integer;
    Public Function EIB_Cache_Enable (): Integer;
    Private Function EIB_Cache_Read_Complete: Integer;
    Public Function EIB_Cache_Read_async (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray): Integer;
    Public Function EIB_Cache_Read (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray): Integer;
    Private Function EIB_Cache_Read_Sync_Complete: Integer;
    Public Function EIB_Cache_Read_Sync_async (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray; age: TUINT16): Integer;
    Public Function EIB_Cache_Read_Sync (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray; age: TUINT16): Integer;
    Private Function EIB_Cache_Remove_Complete: Integer;
    Public Function EIB_Cache_Remove_async (dest: TEIBAddr): Integer;
    Public Function EIB_Cache_Remove (dest: TEIBAddr): Integer;
    Private Function EIB_Cache_LastUpdates_Complete: Integer;
    Public Function EIB_Cache_LastUpdates_async (start: TUINT16; timeout: TUINT8; buf: PByteArray; ende: PUINT16): Integer;
    Public Function EIB_Cache_LastUpdates (start: TUINT16; timeout: TUINT8; buf: PByteArray; ende: PUINT16): Integer;
    Private Function EIB_LoadImage_Complete: Integer;
    Public Function EIB_LoadImage_async (image: TByteArray): Integer;
    Public Function EIB_LoadImage (image: TByteArray): Integer;
    Private Function EIB_MC_Authorize_Complete: Integer;
    Public Function EIB_MC_Authorize_async (key: TEIBKey): Integer;
    Public Function EIB_MC_Authorize (key: TEIBKey): Integer;
    Private Function EIB_MC_Connect_Complete: Integer;
    Public Function EIB_MC_Connect_async (dest: TEIBAddr): Integer;
    Public Function EIB_MC_Connect (dest: TEIBAddr): Integer;
    Private Function EIB_MC_Individual_Open_Complete: Integer;
    Public Function EIB_MC_Individual_Open_async (dest: TEIBAddr): Integer;
    Public Function EIB_MC_Individual_Open (dest: TEIBAddr): Integer;
    Private Function EIB_MC_GetMaskVersion_Complete: Integer;
    Public Function EIB_MC_GetMaskVersion_async (): Integer;
    Public Function EIB_MC_GetMaskVersion (): Integer;
    Private Function EIB_MC_GetPEIType_Complete: Integer;
    Public Function EIB_MC_GetPEIType_async (): Integer;
    Public Function EIB_MC_GetPEIType (): Integer;
    Private Function EIB_MC_Progmode_Off_Complete: Integer;
    Public Function EIB_MC_Progmode_Off_async (): Integer;
    Public Function EIB_MC_Progmode_Off (): Integer;
    Private Function EIB_MC_Progmode_On_Complete: Integer;
    Public Function EIB_MC_Progmode_On_async (): Integer;
    Public Function EIB_MC_Progmode_On (): Integer;
    Private Function EIB_MC_Progmode_Status_Complete: Integer;
    Public Function EIB_MC_Progmode_Status_async (): Integer;
    Public Function EIB_MC_Progmode_Status (): Integer;
    Private Function EIB_MC_Progmode_Toggle_Complete: Integer;
    Public Function EIB_MC_Progmode_Toggle_async (): Integer;
    Public Function EIB_MC_Progmode_Toggle (): Integer;
    Private Function EIB_MC_PropertyDesc_Complete: Integer;
    Public Function EIB_MC_PropertyDesc_async (obj: TUINT8; propertyno: TUINT8; proptype: PUINT8; max_nr_of_elem: PUINT16; access: PUINT8): Integer;
    Public Function EIB_MC_PropertyDesc (obj: TUINT8; propertyno: TUINT8; proptype: PUINT8; max_nr_of_elem: PUINT16; access: PUINT8): Integer;
    Private Function EIB_MC_PropertyRead_Complete: Integer;
    Public Function EIB_MC_PropertyRead_async (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: PByteArray): Integer;
    Public Function EIB_MC_PropertyRead (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: PByteArray): Integer;
    Private Function EIB_MC_PropertyScan_Complete: Integer;
    Public Function EIB_MC_PropertyScan_async (buf: PByteArray): Integer;
    Public Function EIB_MC_PropertyScan (buf: PByteArray): Integer;
    Private Function EIB_MC_PropertyWrite_Complete: Integer;
    Public Function EIB_MC_PropertyWrite_async (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: TByteArray; res: PByteArray): Integer;
    Public Function EIB_MC_PropertyWrite (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: TByteArray; res: PByteArray): Integer;
    Private Function EIB_MC_ReadADC_Complete: Integer;
    Public Function EIB_MC_ReadADC_async (channel: TUINT8; count: TUINT8; val: PUINT16): Integer;
    Public Function EIB_MC_ReadADC (channel: TUINT8; count: TUINT8; val: PUINT16): Integer;
    Private Function EIB_MC_Read_Complete: Integer;
    Public Function EIB_MC_Read_async (addr: TUINT16; buf_len: Integer; buf: PByteArray): Integer;
    Public Function EIB_MC_Read (addr: TUINT16; buf_len: Integer; buf: PByteArray): Integer;
    Private Function EIB_MC_Restart_Complete: Integer;
    Public Function EIB_MC_Restart_async (): Integer;
    Public Function EIB_MC_Restart (): Integer;
    Private Function EIB_MC_SetKey_Complete: Integer;
    Public Function EIB_MC_SetKey_async (key: TEIBKey; level: TUINT8): Integer;
    Public Function EIB_MC_SetKey (key: TEIBKey; level: TUINT8): Integer;
    Private Function EIB_MC_Write_Complete: Integer;
    Public Function EIB_MC_Write_async (addr: TUINT16; buf: TByteArray): Integer;
    Public Function EIB_MC_Write (addr: TUINT16; buf: TByteArray): Integer;
    Private Function EIB_MC_Write_Plain_Complete: Integer;
    Public Function EIB_MC_Write_Plain_async (addr: TUINT16; buf: TByteArray): Integer;
    Public Function EIB_MC_Write_Plain (addr: TUINT16; buf: TByteArray): Integer;
    Private Function EIB_M_GetMaskVersion_Complete: Integer;
    Public Function EIB_M_GetMaskVersion_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_GetMaskVersion (dest: TEIBAddr): Integer;
    Private Function EIB_M_Progmode_Off_Complete: Integer;
    Public Function EIB_M_Progmode_Off_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_Progmode_Off (dest: TEIBAddr): Integer;
    Private Function EIB_M_Progmode_On_Complete: Integer;
    Public Function EIB_M_Progmode_On_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_Progmode_On (dest: TEIBAddr): Integer;
    Private Function EIB_M_Progmode_Status_Complete: Integer;
    Public Function EIB_M_Progmode_Status_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_Progmode_Status (dest: TEIBAddr): Integer;
    Private Function EIB_M_Progmode_Toggle_Complete: Integer;
    Public Function EIB_M_Progmode_Toggle_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_Progmode_Toggle (dest: TEIBAddr): Integer;
    Private Function EIB_M_ReadIndividualAddresses_Complete: Integer;
    Public Function EIB_M_ReadIndividualAddresses_async (buf: PByteArray): Integer;
    Public Function EIB_M_ReadIndividualAddresses (buf: PByteArray): Integer;
    Private Function EIB_M_WriteIndividualAddress_Complete: Integer;
    Public Function EIB_M_WriteIndividualAddress_async (dest: TEIBAddr): Integer;
    Public Function EIB_M_WriteIndividualAddress (dest: TEIBAddr): Integer;
    Private Function EIBOpenBusmonitor_Complete: Integer;
    Public Function EIBOpenBusmonitor_async (): Integer;
    Public Function EIBOpenBusmonitor (): Integer;
    Private Function EIBOpenBusmonitorText_Complete: Integer;
    Public Function EIBOpenBusmonitorText_async (): Integer;
    Public Function EIBOpenBusmonitorText (): Integer;
    Private Function EIBOpen_GroupSocket_Complete: Integer;
    Public Function EIBOpen_GroupSocket_async (write_only: Boolean): Integer;
    Public Function EIBOpen_GroupSocket (write_only: Boolean): Integer;
    Private Function EIBOpenT_Broadcast_Complete: Integer;
    Public Function EIBOpenT_Broadcast_async (write_only: Boolean): Integer;
    Public Function EIBOpenT_Broadcast (write_only: Boolean): Integer;
    Private Function EIBOpenT_Connection_Complete: Integer;
    Public Function EIBOpenT_Connection_async (dest: TEIBAddr): Integer;
    Public Function EIBOpenT_Connection (dest: TEIBAddr): Integer;
    Private Function EIBOpenT_Group_Complete: Integer;
    Public Function EIBOpenT_Group_async (dest: TEIBAddr; write_only: Boolean; priority: Boolean): Integer;
    Public Function EIBOpenT_Group (dest: TEIBAddr; write_only: Boolean; priority: Boolean): Integer;
    Private Function EIBOpenT_Individual_Complete: Integer;
    Public Function EIBOpenT_Individual_async (dest: TEIBAddr; write_only: Boolean): Integer;
    Public Function EIBOpenT_Individual (dest: TEIBAddr; write_only: Boolean): Integer;
    Private Function EIBOpenT_TPDU_Complete: Integer;
    Public Function EIBOpenT_TPDU_async (src: TEIBAddr): Integer;
    Public Function EIBOpenT_TPDU (src: TEIBAddr): Integer;
    Private Function EIBOpenVBusmonitor_Complete: Integer;
    Public Function EIBOpenVBusmonitor_async (): Integer;
    Public Function EIBOpenVBusmonitor (): Integer;
    Private Function EIBOpenVBusmonitorText_Complete: Integer;
    Public Function EIBOpenVBusmonitorText_async (): Integer;
    Public Function EIBOpenVBusmonitorText (): Integer;
    Private Function EIBReset_Complete: Integer;
    Public Function EIBReset_async (): Integer;
    Public Function EIBReset (): Integer;
    Public Function EIBSendAPDU (data: TByteArray): Integer;
    Public Function EIBSendGroup (dest: TEIBAddr; data: TByteArray): Integer;
    Public Function EIBSendTPDU (dest: TEIBAddr; data: TByteArray): Integer;
    Private Function EIB_State_Threads_Complete: Integer;
    Public Function EIB_State_Threads_async (buf: PByteArray): Integer;
    Public Function EIB_State_Threads (buf: PByteArray): Integer;
    Private Function EIB_State_Backends_Complete: Integer;
    Public Function EIB_State_Backends_async (buf: PByteArray): Integer;
    Public Function EIB_State_Backends (buf: PByteArray): Integer;
    Private Function EIB_State_Servers_Complete: Integer;
    Public Function EIB_State_Servers_async (buf: PByteArray): Integer;
    Public Function EIB_State_Servers (buf: PByteArray): Integer;
    Private complete : Complete_Function;
    Private recvdata : TByteArray;
    Private recvhead : TByteArray;
    Private ptr1 : PUINT16;
    Private ptr2 : PUINT8;
    Private ptr3 : PUINT8;
    Private ptr4 : PUINT16;
    Private ptr5 : PEIBAddr;
    Private ptr6 : PEIBAddr;
    Private ptr7 : PUINT32;
    Private buffer : PByteArray;
    Private sendlen : Integer;
    Private readlen : Integer;
    Private datalen : Integer;
    Private fd : LongInt;

    Public Function EIBComplete : Integer;
    Public Function EIBClose : Integer;
    Public Function EIBClose_sync : Integer;
    Public Function EIB_Poll_Complete : Integer;
    Public Function EIB_Poll_FD : Integer;
    Private Function EIB_SendRequest (data: TByteArray): Integer;
    Private Function EIB_GetRequest: Integer;
    Private Function EIB_CheckRequest (block: Boolean): Integer;

    Public constructor Create;
    Public Function EIBSocketLocal (path: String): Integer;
    Public Function EIBSocketRemote (host: String): Integer;
    Public Function EIBSocketRemote (host: String; port: Integer): Integer;
    Public Function EIBSocketURL (url: String): Integer;
 
  End;
Implementation
{
    EIBD client library
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
}
Uses BaseUnix, Sockets, Strings, NetDB;

Function GetHostAddr(Var addr: sockaddr_in; host: String): boolean;
Var he : THostEntry;
Begin
  If gethostbyname(host, he) Then
    Begin
      Result := true;
      addr.sin_addr.s_addr := htonl(he.addr.s_addr);
      exit;
    End;
  Result := false;
End;

Function EIBConnection.EIBComplete : Integer;
Begin
  If complete = Nil Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  Result := complete();
End;

Function EIBConnection.EIBClose_sync : Integer;
Begin
  EIBReset();
  Result := EIBClose();
End;

Function EIBConnection.EIB_SendRequest(data : TByteArray): Integer;
Var head: Array[0..1] Of byte;
Begin
  If fd = -1 Then
    Begin;
      fpseterrno(ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If (Length(data) < 2) Or (Length(data) >= $ffff) Then
    Begin
      fpseterrno(ESysEINVAL);
      Result := -1;
      exit;
    End;
  head[0] := (length(data) shr 8) And $ff;
  head[1] := (length(data)) And $ff;
  If fpsend(fd, @head[0], length(head), 0) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If fpsend(fd, @data[0], length(data), 0) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := 0;
End;

Function EIBConnection.EIBClose : Integer;
Begin
  If fd = -1 Then
    Begin;
      fpseterrno(ESysEINVAL);
      Result := -1;
      exit;
    End;
  CloseSocket(fd);
  fd := -1;
  Result := 0;
End;

Function EIBConnection.EIB_Poll_Complete: Integer;
Begin
  If EIB_CheckRequest(false) = -1 Then
    Begin;
      Result := -1;
      exit;
    End;
  If (readlen < 2) Or ((readlen >= 2) And (readlen < datalen + 2)) Then
    Result := 0
  Else
    Result := 1;
End;

Function EIBConnection.EIB_Poll_FD: Integer;
Begin
  result := fd;
End;

Function EIBConnection.EIB_GetRequest: Integer;
Begin
  Repeat
    If EIB_CheckRequest(true) = -1 Then
      Begin;
        Result := -1;
        exit;
      End;
    If (readlen >= 2) And (readlen >= datalen + 2) Then
      Begin
        readlen := 0;
        result := 0;
        exit;
      End;
  Until false;
End;

Function EIBConnection.EIB_CheckRequest(block: boolean): Integer;
Var i : Integer;
Begin
  If fd = -1 Then
    Begin;
      fpseterrno(ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If readlen = 0 Then
    Begin
      setLength(recvdata, 0);
      setLength(recvhead, 2);
    End;
  If readlen < 2 Then
    Begin
      If block Then
        fpfcntl(fd, F_SETFL, fpfcntl(fd, F_GETFL, 0) And Not O_NONBLOCK)
      Else
        fpfcntl(fd, F_SETFL, fpfcntl(fd, F_GETFL, 0) Or O_NONBLOCK);
      i := fprecv(fd, @recvhead[readlen], 2-readlen, 0);
      If i = -1 Then
        Begin
          Result := -1;
          exit;
        End;
      If i = 0 Then
        Begin
          fpseterrno(ESysECONNRESET);
          Result := -1;
          exit;
        End;
      readlen := readlen + i;
    End;
  If readlen < 2 Then
    Begin
      Result := 0;
      exit;
    End;
  datalen := (recvhead[0] shr 8) Or recvhead[1];
  setLength (recvdata, datalen);
  If readlen < datalen + 2 Then
    Begin
      If block Then
        fpfcntl(fd, F_SETFL, fpfcntl(fd, F_GETFL, 0) And Not O_NONBLOCK)
      Else
        fpfcntl(fd, F_SETFL, fpfcntl(fd, F_GETFL, 0) Or O_NONBLOCK);
      i := fprecv(fd, @recvdata[readlen-2], datalen+2-readlen, 0);
      If i = -1 Then
        Begin
          Result := -1;
          exit;
        End;
      If i = 0 Then
        Begin
          fpseterrno(ESysECONNRESET);
          Result := -1;
          exit;
        End;
      readlen := readlen + i;
    End;
  Result := 0;
End;

Constructor EIBConnection.Create;
Begin
  readlen := 0;
  datalen := 0;
  fd := -1;
  complete := Nil;
  setLength(recvdata, 0);
End;

Function EIBConnection.EIBSocketLocal (path:String): Integer;
Var sock : LongInt;
  addr : sockaddr_un;
Begin
  If fd <> -1 Then
    Begin;
      fpseterrno(ESysEUSERS);
      Result := -1;
      exit;
    End;
  sock := fpsocket(AF_UNIX, SOCK_STREAM, 0);
  If sock = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  addr.sun_family := AF_UNIX;
  strpcopy (addr.sun_path, path);
  If fpconnect(sock, @addr, length(path)+3) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  fd := sock;
End;

Function EIBConnection.EIBSocketRemote (host: String) : Integer;
Begin
  Result := EIBSocketRemote (host, 6720);
End;

Function EIBConnection.EIBSocketURL (url:String) : Integer;
var host, portstr: String;
  port,code : Integer;
Begin
  If copy(url, 1, 6) = 'local:' Then
    Begin
      Result := EIBSocketLocal (copy (url, 7, length(url)));
      exit;
    End;
  If copy(url, 1, 3) = 'ip:' Then
    Begin
      host := copy (url, 4, length(url));
      If pos(':', host) <> 0 Then
        Begin
          portstr := copy (host, pos(':', host)+1, length(url));
          host := copy (host, 1, pos(':', host)-1);
          val (portstr, port, code);
          If code <> 0 Then
            Begin
              fpseterrno(ESysEINVAL);
              Result := -1;
              exit;
            End;
          Result := EIBSocketRemote (host, port);
        End
      Else
        Begin
          Result := EIBSocketRemote (host);
        End;
      exit;
    End;
  fpseterrno(ESysEINVAL);
  Result := -1;
End;

Function EIBConnection.EIBSocketRemote (host: String; port: Integer) : Integer;
var sock : LongInt;
  addr : sockaddr_in; 
  arg : Integer;
Begin
  If fd <> -1 Then
    Begin;
      fpseterrno(ESysEUSERS);
      Result := -1;
      exit;
    End;
  If Not GetHostAddr(addr, host) Then
    Begin
      fpseterrno(ESysECONNREFUSED);
      Result := -1;
      exit;
    End;
  sock := fpsocket(AF_INET, SOCK_STREAM, 0);
  If sock = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  addr.sin_family := AF_INET;
  addr.sin_port := htons (port);
  If fpconnect(sock, @addr, sizeof(addr)) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  arg := 1;
  fpsetsockopt(sock, IPPROTO_TCP, TCP_NODELAY, @arg, sizeof(arg));
  fd := sock;
  Result := 0;
End;


Function EIBConnection.EIBGetAPDU_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 37) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIBGetAPDU_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  complete := @EIBGetAPDU_Complete;
  Result := 0;
End;

Function EIBConnection.EIBGetAPDU (buf: PByteArray): Integer;
Begin
  If EIBGetAPDU_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBGetAPDU_Src_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 37) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr5 <> Nil Then
    ptr5^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  setLength(buffer^, length(recvdata) - 4);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[4 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIBGetAPDU_Src_async (buf: PByteArray; src: PEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ptr5 := src;
  complete := @EIBGetAPDU_Src_Complete;
  Result := 0;
End;

Function EIBConnection.EIBGetAPDU_Src (buf: PByteArray; src: PEIBAddr): Integer;
Begin
  If EIBGetAPDU_Src_async (buf, src) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBGetBusmonitorPacket_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 20) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIBGetBusmonitorPacket_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  complete := @EIBGetBusmonitorPacket_Complete;
  Result := 0;
End;

Function EIBConnection.EIBGetBusmonitorPacket (buf: PByteArray): Integer;
Begin
  If EIBGetBusmonitorPacket_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBGetGroup_Src_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 39) Or (length(recvdata) < 6) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr5 <> Nil Then
    ptr5^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  If ptr6 <> Nil Then
    ptr6^ := ((recvdata[4] shl 8) Or (recvdata[4+1]));
  setLength(buffer^, length(recvdata) - 6);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[6 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIBGetGroup_Src_async (buf: PByteArray; src: PEIBAddr; dest: PEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ptr5 := src;
  ptr6 := dest;
  complete := @EIBGetGroup_Src_Complete;
  Result := 0;
End;

Function EIBConnection.EIBGetGroup_Src (buf: PByteArray; src: PEIBAddr; dest: PEIBAddr): Integer;
Begin
  If EIBGetGroup_Src_async (buf, src, dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBGetTPDU_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 37) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr5 <> Nil Then
    ptr5^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  setLength(buffer^, length(recvdata) - 4);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[4 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIBGetTPDU_async (buf: PByteArray; src: PEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ptr5 := src;
  complete := @EIBGetTPDU_Complete;
  Result := 0;
End;

Function EIBConnection.EIBGetTPDU (buf: PByteArray; src: PEIBAddr): Integer;
Begin
  If EIBGetTPDU_async (buf, src) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Clear_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 114) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_Cache_Clear_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 114;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Clear_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Clear (): Integer;
Begin
  If EIB_Cache_Clear_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Disable_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 113) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_Cache_Disable_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 113;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Disable_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Disable (): Integer;
Begin
  If EIB_Cache_Disable_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Enable_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 1 Then
    Begin
      fpseterrno (ESysEBUSY);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 112) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_Cache_Enable_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 112;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Enable_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Enable (): Integer;
Begin
  If EIB_Cache_Enable_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Read_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 117) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ((recvdata[4] shl 8) Or (recvdata[4+1])) = 0 Then
    Begin
      fpseterrno (ESysENODEV);
      Result := -1;
      exit;
    End;
  If length(recvdata) <= 6 Then
    Begin
      fpseterrno (ESysENOENT);
      Result := -1;
      exit;
    End;
  If ptr5 <> Nil Then
    ptr5^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  setLength(buffer^, length(recvdata) - 6);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[6 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_Cache_Read_async (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  buffer := buf;
  ptr5 := src;
  ibuf[2] := ((dst shr 8) And $ff);
  ibuf[3] := ((dst) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 117;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Read_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Read (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray): Integer;
Begin
  If EIB_Cache_Read_async (dst, src, buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Read_Sync_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 116) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ((recvdata[4] shl 8) Or (recvdata[4+1])) = 0 Then
    Begin
      fpseterrno (ESysENODEV);
      Result := -1;
      exit;
    End;
  If length(recvdata) <= 6 Then
    Begin
      fpseterrno (ESysENOENT);
      Result := -1;
      exit;
    End;
  If ptr5 <> Nil Then
    ptr5^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  setLength(buffer^, length(recvdata) - 6);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[6 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_Cache_Read_Sync_async (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray; age: TUINT16): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  buffer := buf;
  ptr5 := src;
  ibuf[2] := ((dst shr 8) And $ff);
  ibuf[3] := ((dst) And $ff);
  ibuf[4] := ((age shr 8) And $ff);
  ibuf[5] := ((age) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 116;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Read_Sync_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Read_Sync (dst: TEIBAddr; src: PEIBAddr; buf: PByteArray; age: TUINT16): Integer;
Begin
  If EIB_Cache_Read_Sync_async (dst, src, buf, age) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_Remove_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 115) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_Cache_Remove_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 115;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_Remove_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_Remove (dest: TEIBAddr): Integer;
Begin
  If EIB_Cache_Remove_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_Cache_LastUpdates_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 118) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr4 <> Nil Then
    ptr4^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  setLength(buffer^, length(recvdata) - 4);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[4 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_Cache_LastUpdates_async (start: TUINT16; timeout: TUINT8; buf: PByteArray; ende: PUINT16): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  buffer := buf;
  ptr4 := ende;
  ibuf[2] := ((start shr 8) And $ff);
  ibuf[3] := ((start) And $ff);
  ibuf[4] := timeout;
  ibuf[0] := 0;
  ibuf[1] := 118;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_Cache_LastUpdates_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_Cache_LastUpdates (start: TUINT16; timeout: TUINT8; buf: PByteArray; ende: PUINT16): Integer;
Begin
  If EIB_Cache_LastUpdates_async (start, timeout, buf, ende) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_LoadImage_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 99) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  exit;
End;

Function EIBConnection.EIB_LoadImage_async (image: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  If length(image) < 0 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(image);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := image[i];
  ibuf[0] := 0;
  ibuf[1] := 99;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_LoadImage_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_LoadImage (image: TByteArray): Integer;
Begin
  If EIB_LoadImage_async (image) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Authorize_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 87) Or (length(recvdata) < 3) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := recvdata[2];
  exit;
End;

Function EIBConnection.EIB_MC_Authorize_async (key: TEIBKey): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  ibuf[2] := key[0];
  ibuf[3] := key[1];
  ibuf[4] := key[2];
  ibuf[5] := key[3];
  ibuf[0] := 0;
  ibuf[1] := 87;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Authorize_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Authorize (key: TEIBKey): Integer;
Begin
  If EIB_MC_Authorize_async (key) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Connect_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 80) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Connect_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 80;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Connect_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Connect (dest: TEIBAddr): Integer;
Begin
  If EIB_MC_Connect_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Individual_Open_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 73) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Individual_Open_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 73;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Individual_Open_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Individual_Open (dest: TEIBAddr): Integer;
Begin
  If EIB_MC_Individual_Open_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_GetMaskVersion_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 89) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  exit;
End;

Function EIBConnection.EIB_MC_GetMaskVersion_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 89;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_GetMaskVersion_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_GetMaskVersion (): Integer;
Begin
  If EIB_MC_GetMaskVersion_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_GetPEIType_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 85) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  exit;
End;

Function EIBConnection.EIB_MC_GetPEIType_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 85;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_GetPEIType_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_GetPEIType (): Integer;
Begin
  If EIB_MC_GetPEIType_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Progmode_Off_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 96) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Progmode_Off_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 3);
  ibuf[2] := 0;
  ibuf[0] := 0;
  ibuf[1] := 96;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Progmode_Off_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Progmode_Off (): Integer;
Begin
  If EIB_MC_Progmode_Off_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Progmode_On_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 96) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Progmode_On_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 3);
  ibuf[2] := 1;
  ibuf[0] := 0;
  ibuf[1] := 96;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Progmode_On_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Progmode_On (): Integer;
Begin
  If EIB_MC_Progmode_On_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Progmode_Status_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 96) Or (length(recvdata) < 3) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := recvdata[2];
  exit;
End;

Function EIBConnection.EIB_MC_Progmode_Status_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 3);
  ibuf[2] := 3;
  ibuf[0] := 0;
  ibuf[1] := 96;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Progmode_Status_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Progmode_Status (): Integer;
Begin
  If EIB_MC_Progmode_Status_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Progmode_Toggle_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 96) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Progmode_Toggle_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 3);
  ibuf[2] := 2;
  ibuf[0] := 0;
  ibuf[1] := 96;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Progmode_Toggle_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Progmode_Toggle (): Integer;
Begin
  If EIB_MC_Progmode_Toggle_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_PropertyDesc_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 97) Or (length(recvdata) < 6) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr2 <> Nil Then
    ptr2^ := recvdata[2];
  If ptr4 <> Nil Then
    ptr4^ := ((recvdata[3] shl 8) Or (recvdata[3+1]));
  If ptr3 <> Nil Then
    ptr3^ := recvdata[5];
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_PropertyDesc_async (obj: TUINT8; propertyno: TUINT8; proptype: PUINT8; max_nr_of_elem: PUINT16; access: PUINT8): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ptr2 := proptype;
  ptr4 := max_nr_of_elem;
  ptr3 := access;
  ibuf[2] := obj;
  ibuf[3] := propertyno;
  ibuf[0] := 0;
  ibuf[1] := 97;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_PropertyDesc_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_PropertyDesc (obj: TUINT8; propertyno: TUINT8; proptype: PUINT8; max_nr_of_elem: PUINT16; access: PUINT8): Integer;
Begin
  If EIB_MC_PropertyDesc_async (obj, propertyno, proptype, max_nr_of_elem, access) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_PropertyRead_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 83) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_MC_PropertyRead_async (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 7);
  buffer := buf;
  ibuf[2] := obj;
  ibuf[3] := propertyno;
  ibuf[4] := ((start shr 8) And $ff);
  ibuf[5] := ((start) And $ff);
  ibuf[6] := nr_of_elem;
  ibuf[0] := 0;
  ibuf[1] := 83;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_PropertyRead_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_PropertyRead (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: PByteArray): Integer;
Begin
  If EIB_MC_PropertyRead_async (obj, propertyno, start, nr_of_elem, buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_PropertyScan_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 98) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_MC_PropertyScan_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ibuf[0] := 0;
  ibuf[1] := 98;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_PropertyScan_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_PropertyScan (buf: PByteArray): Integer;
Begin
  If EIB_MC_PropertyScan_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_PropertyWrite_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 84) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_MC_PropertyWrite_async (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: TByteArray; res: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 7);
  ibuf[2] := obj;
  ibuf[3] := propertyno;
  ibuf[4] := ((start shr 8) And $ff);
  ibuf[5] := ((start) And $ff);
  ibuf[6] := nr_of_elem;
  If length(buf) < 0 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(buf);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := buf[i];
  buffer := res;
  ibuf[0] := 0;
  ibuf[1] := 84;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_PropertyWrite_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_PropertyWrite (obj: TUINT8; propertyno: TUINT8; start: TUINT16; nr_of_elem: TUINT8; buf: TByteArray; res: PByteArray): Integer;
Begin
  If EIB_MC_PropertyWrite_async (obj, propertyno, start, nr_of_elem, buf, res) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_ReadADC_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 86) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  If ptr1 <> Nil Then
    ptr1^ := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_ReadADC_async (channel: TUINT8; count: TUINT8; val: PUINT16): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ptr1 := val;
  ibuf[2] := channel;
  ibuf[3] := count;
  ibuf[0] := 0;
  ibuf[1] := 86;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_ReadADC_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_ReadADC (channel: TUINT8; count: TUINT8; val: PUINT16): Integer;
Begin
  If EIB_MC_ReadADC_async (channel, count, val) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Read_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 81) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_MC_Read_async (addr: TUINT16; buf_len: Integer; buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  buffer := buf;
  ibuf[2] := ((addr shr 8) And $ff);
  ibuf[3] := ((addr) And $ff);
  ibuf[4] := ((buf_len shr 8) And $ff);
  ibuf[5] := ((buf_len) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 81;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Read_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Read (addr: TUINT16; buf_len: Integer; buf: PByteArray): Integer;
Begin
  If EIB_MC_Read_async (addr, buf_len, buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Restart_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 90) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_Restart_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 90;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Restart_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Restart (): Integer;
Begin
  If EIB_MC_Restart_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_SetKey_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 2 Then
    Begin
      fpseterrno (ESysEPERM);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 88) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_MC_SetKey_async (key: TEIBKey; level: TUINT8): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 7);
  ibuf[2] := key[0];
  ibuf[3] := key[1];
  ibuf[4] := key[2];
  ibuf[5] := key[3];
  ibuf[6] := level;
  ibuf[0] := 0;
  ibuf[1] := 88;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_SetKey_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_SetKey (key: TEIBKey; level: TUINT8): Integer;
Begin
  If EIB_MC_SetKey_async (key, level) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Write_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 68 Then
    Begin
      fpseterrno (ESysEIO);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 82) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := sendlen;
  exit;
End;

Function EIBConnection.EIB_MC_Write_async (addr: TUINT16; buf: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  ibuf[2] := ((addr shr 8) And $ff);
  ibuf[3] := ((addr) And $ff);
  ibuf[4] := ((length(buf) shr 8) And $ff);
  ibuf[5] := ((length(buf)) And $ff);
  If length(buf) < 0 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(buf);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := buf[i];
  ibuf[0] := 0;
  ibuf[1] := 82;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Write_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Write (addr: TUINT16; buf: TByteArray): Integer;
Begin
  If EIB_MC_Write_async (addr, buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_MC_Write_Plain_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 91) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := sendlen;
  exit;
End;

Function EIBConnection.EIB_MC_Write_Plain_async (addr: TUINT16; buf: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  ibuf[2] := ((addr shr 8) And $ff);
  ibuf[3] := ((addr) And $ff);
  ibuf[4] := ((length(buf) shr 8) And $ff);
  ibuf[5] := ((length(buf)) And $ff);
  If length(buf) < 0 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(buf);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := buf[i];
  ibuf[0] := 0;
  ibuf[1] := 91;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_MC_Write_Plain_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_MC_Write_Plain (addr: TUINT16; buf: TByteArray): Integer;
Begin
  If EIB_MC_Write_Plain_async (addr, buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_GetMaskVersion_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 49) Or (length(recvdata) < 4) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := ((recvdata[2] shl 8) Or (recvdata[2+1]));
  exit;
End;

Function EIBConnection.EIB_M_GetMaskVersion_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 49;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_GetMaskVersion_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_GetMaskVersion (dest: TEIBAddr): Integer;
Begin
  If EIB_M_GetMaskVersion_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_Progmode_Off_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 48) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_M_Progmode_Off_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[4] := 0;
  ibuf[0] := 0;
  ibuf[1] := 48;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_Progmode_Off_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_Progmode_Off (dest: TEIBAddr): Integer;
Begin
  If EIB_M_Progmode_Off_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_Progmode_On_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 48) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_M_Progmode_On_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[4] := 1;
  ibuf[0] := 0;
  ibuf[1] := 48;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_Progmode_On_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_Progmode_On (dest: TEIBAddr): Integer;
Begin
  If EIB_M_Progmode_On_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_Progmode_Status_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 48) Or (length(recvdata) < 3) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := recvdata[2];
  exit;
End;

Function EIBConnection.EIB_M_Progmode_Status_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[4] := 3;
  ibuf[0] := 0;
  ibuf[1] := 48;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_Progmode_Status_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_Progmode_Status (dest: TEIBAddr): Integer;
Begin
  If EIB_M_Progmode_Status_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_Progmode_Toggle_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 48) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_M_Progmode_Toggle_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[4] := 2;
  ibuf[0] := 0;
  ibuf[1] := 48;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_Progmode_Toggle_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_Progmode_Toggle (dest: TEIBAddr): Integer;
Begin
  If EIB_M_Progmode_Toggle_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_ReadIndividualAddresses_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 50) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_M_ReadIndividualAddresses_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ibuf[0] := 0;
  ibuf[1] := 50;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_ReadIndividualAddresses_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_ReadIndividualAddresses (buf: PByteArray): Integer;
Begin
  If EIB_M_ReadIndividualAddresses_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_M_WriteIndividualAddress_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 65 Then
    Begin
      fpseterrno (ESysEADDRINUSE);
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 67 Then
    Begin
      fpseterrno (ESysETIMEDOUT);
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 66 Then
    Begin
      fpseterrno (ESysEADDRNOTAVAIL);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 64) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIB_M_WriteIndividualAddress_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 64;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_M_WriteIndividualAddress_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_M_WriteIndividualAddress (dest: TEIBAddr): Integer;
Begin
  If EIB_M_WriteIndividualAddress_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenBusmonitor_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 1 Then
    Begin
      fpseterrno (ESysEBUSY);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 16) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenBusmonitor_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 16;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenBusmonitor_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenBusmonitor (): Integer;
Begin
  If EIBOpenBusmonitor_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenBusmonitorText_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 1 Then
    Begin
      fpseterrno (ESysEBUSY);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 17) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenBusmonitorText_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 17;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenBusmonitorText_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenBusmonitorText (): Integer;
Begin
  If EIBOpenBusmonitorText_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpen_GroupSocket_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 38) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpen_GroupSocket_async (write_only: Boolean): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  If write_only Then
    ibuf[4] := $ff
  Else
    ibuf[4] := $00;
  ibuf[0] := 0;
  ibuf[1] := 38;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpen_GroupSocket_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpen_GroupSocket (write_only: Boolean): Integer;
Begin
  If EIBOpen_GroupSocket_async (write_only) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenT_Broadcast_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 35) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenT_Broadcast_async (write_only: Boolean): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  If write_only Then
    ibuf[4] := $ff
  Else
    ibuf[4] := $00;
  ibuf[0] := 0;
  ibuf[1] := 35;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenT_Broadcast_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenT_Broadcast (write_only: Boolean): Integer;
Begin
  If EIBOpenT_Broadcast_async (write_only) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenT_Connection_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 32) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenT_Connection_async (dest: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 32;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenT_Connection_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenT_Connection (dest: TEIBAddr): Integer;
Begin
  If EIBOpenT_Connection_async (dest) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenT_Group_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 34) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenT_Group_async (dest: TEIBAddr; write_only: Boolean; priority: Boolean): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 6);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  If write_only Then
    ibuf[4] := $ff
  Else
    ibuf[4] := $00;
  ibuf[5] := priority;
  ibuf[0] := 0;
  ibuf[1] := 34;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenT_Group_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenT_Group (dest: TEIBAddr; write_only: Boolean; priority: Boolean): Integer;
Begin
  If EIBOpenT_Group_async (dest, write_only, priority) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenT_Individual_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 33) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenT_Individual_async (dest: TEIBAddr; write_only: Boolean): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  If write_only Then
    ibuf[4] := $ff
  Else
    ibuf[4] := $00;
  ibuf[0] := 0;
  ibuf[1] := 33;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenT_Individual_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenT_Individual (dest: TEIBAddr; write_only: Boolean): Integer;
Begin
  If EIBOpenT_Individual_async (dest, write_only) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenT_TPDU_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 36) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenT_TPDU_async (src: TEIBAddr): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 5);
  ibuf[2] := ((src shr 8) And $ff);
  ibuf[3] := ((src) And $ff);
  ibuf[0] := 0;
  ibuf[1] := 36;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenT_TPDU_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenT_TPDU (src: TEIBAddr): Integer;
Begin
  If EIBOpenT_TPDU_async (src) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenVBusmonitor_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 1 Then
    Begin
      fpseterrno (ESysEBUSY);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 18) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenVBusmonitor_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 18;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenVBusmonitor_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenVBusmonitor (): Integer;
Begin
  If EIBOpenVBusmonitor_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBOpenVBusmonitorText_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If ((recvdata[0] shl 8) Or (recvdata[0+1])) <> 1 Then
    Begin
      fpseterrno (ESysEBUSY);
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 19) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBOpenVBusmonitorText_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 19;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBOpenVBusmonitorText_Complete;
  Result := 0;
End;

Function EIBConnection.EIBOpenVBusmonitorText (): Integer;
Begin
  If EIBOpenVBusmonitorText_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBReset_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 4) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  Result := 0;
  exit;
End;

Function EIBConnection.EIBReset_async (): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  ibuf[0] := 0;
  ibuf[1] := 4;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIBReset_Complete;
  Result := 0;
End;

Function EIBConnection.EIBReset (): Integer;
Begin
  If EIBReset_async () = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIBSendAPDU (data: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  If length(data) < 2 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(data);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := data[i];
  ibuf[0] := 0;
  ibuf[1] := 37;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := sendlen;
  exit;
End;

Function EIBConnection.EIBSendGroup (dest: TEIBAddr; data: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  If length(data) < 2 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(data);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := data[i];
  ibuf[0] := 0;
  ibuf[1] := 39;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := sendlen;
  exit;
End;

Function EIBConnection.EIBSendTPDU (dest: TEIBAddr; data: TByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 4);
  ibuf[2] := ((dest shr 8) And $ff);
  ibuf[3] := ((dest) And $ff);
  If length(data) < 2 Then
    Begin
      fpseterrno (ESysEINVAL);
      Result := -1;
      exit;
    End;
  sendlen := length(data);
  setLength(ibuf, length(ibuf) + sendlen);
  For i := 0 To sendlen Do
    ibuf[length(ibuf) - sendlen + i] := data[i];
  ibuf[0] := 0;
  ibuf[1] := 37;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := sendlen;
  exit;
End;

Function EIBConnection.EIB_State_Threads_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  If (((recvdata[0] shl 8) Or (recvdata[0+1])) <> 257) Or (length(recvdata) < 2) Then
    Begin
      fpseterrno (ESysECONNRESET);
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_State_Threads_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ibuf[0] := 1;
  ibuf[1] := 1;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_State_Threads_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_State_Threads (buf: PByteArray): Integer;
Begin
  If EIB_State_Threads_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_State_Backends_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_State_Backends_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ibuf[0] := 1;
  ibuf[1] := 2;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_State_Backends_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_State_Backends (buf: PByteArray): Integer;
Begin
  If EIB_State_Backends_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;

Function EIBConnection.EIB_State_Servers_Complete: Integer;
Var i : Integer;
Begin
  complete := Nil;
  If EIB_GetRequest() = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  setLength(buffer^, length(recvdata) - 2);
  For i:= 0 To length(buffer^) Do
    buffer^[i] := recvdata[2 + i];
  Result := length(buffer^);
  exit;
End;

Function EIBConnection.EIB_State_Servers_async (buf: PByteArray): Integer;
Var ibuf : TByteArray;
  i : Integer;
Begin
  setLength(ibuf, 2);
  buffer := buf;
  ibuf[0] := 1;
  ibuf[1] := 3;
  If EIB_SendRequest(ibuf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  complete := @EIB_State_Servers_Complete;
  Result := 0;
End;

Function EIBConnection.EIB_State_Servers (buf: PByteArray): Integer;
Begin
  If EIB_State_Servers_async (buf) = -1 Then
    Begin
      Result := -1;
      exit;
    End;
  Result := EIBComplete();
End;


End.
