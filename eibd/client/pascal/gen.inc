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

