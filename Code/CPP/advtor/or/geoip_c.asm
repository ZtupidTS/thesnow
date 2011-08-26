.586
.model flat,stdcall
option casemap:none


.code

geoip_get_country_name	PROC	STDCALL	idx:DWORD
	lea	eax,c_names
	.if idx>=num_c
		ret
	.endif
	mov	ecx,idx
	.while ecx!=0
		.while (byte ptr[eax]!=0)
			inc	eax
		.endw
		inc	eax
		.while (byte ptr[eax]!=0)
			inc	eax
		.endw
		inc	eax
		.break .if byte ptr[eax]==0
		dec	ecx
	.endw
	ret
	include	geoip_c.h
geoip_get_country_name	ENDP

GeoIP_getfullname	PROC	STDCALL	idx:DWORD
	mov	ecx,idx
	.if ecx<num_c
		lea	eax,c_names
		.while ecx!=0
			.while (byte ptr[eax]!=0)
				inc	eax
			.endw
			inc	eax
			.while (byte ptr[eax]!=0)
				inc	eax
			.endw
			inc	eax
			.break .if byte ptr[eax]==0
			dec	ecx
		.endw
	.endif
	.if byte ptr[eax]
		.while (byte ptr[eax]!=0)
			inc	eax
		.endw
		inc	eax
	.endif
	ret
GeoIP_getfullname	ENDP

geoip_get_country_by_ip	PROC	STDCALL	uses ebx ip:DWORD
	mov	eax,ip
	lea	ebx,c_data
	.while 1
		.break .if al<=[ebx]
		.break .if dword ptr[ebx+1]==0
		add	ebx,[ebx+1]
	.endw
	.if al<[ebx]
		add	ebx,[ebx+1+4]
		add	ebx,[ebx+1+4]
		add	ebx,[ebx+1+4]
		mov	al,[ebx+1+4]
	.elseif al==[ebx]
		ror	eax,8
		push	ebx
		add	ebx,[ebx+1+4]
		.while 1
			.break .if al<=[ebx]
			.break .if dword ptr[ebx+1]==0
			add	ebx,[ebx+1]
		.endw
		.if al<[ebx]
			add	ebx,[ebx+1+4]
			add	ebx,[ebx+1+4]
			mov	al,[ebx+1+4]
			pop	ebx
		.elseif al==[ebx]
			ror	eax,8
			push	ebx
			add	ebx,[ebx+1+4]
			.while 1
				.break .if al<=[ebx]
				.break .if dword ptr[ebx+1]==0
				add	ebx,[ebx+1]
			.endw
			.if al<[ebx]
				add	ebx,[ebx+1+4]
				mov	al,[ebx+1+4]
				pop	ebx
				pop	ebx
			.elseif al==[ebx]
				ror	eax,8
				push	ebx
				add	ebx,[ebx+1+4]
				.while 1
					.break .if al<[ebx]
					.break .if dword ptr[ebx+1]==0
					add	ebx,[ebx+1]
				.endw
				.if al<[ebx]
					mov	al,[ebx+5]
					pop	ebx
					pop	ebx
					pop	ebx
				.else
					pop	ebx
					.if dword ptr[ebx+1]
						add	ebx,[ebx+1]
						add	ebx,[ebx+1+4]
						mov	al,[ebx+1+4]
						pop	ebx
						pop	ebx
					.else
						pop	ebx
						.if dword ptr[ebx+1]
							add	ebx,[ebx+1]
							add	ebx,[ebx+1+4]
							add	ebx,[ebx+1+4]
							mov	al,[ebx+1+4]
							pop	ebx
						.else
							pop	ebx
							.if dword ptr[ebx+1]
								add	ebx,[ebx+1]
								add	ebx,[ebx+1+4]
								add	ebx,[ebx+1+4]
								add	ebx,[ebx+1+4]
								mov	al,[ebx+1+4]
							.endif
						.endif
					.endif
				.endif
			.else
				pop	ebx
				.if dword ptr[ebx+1]
					add	ebx,[ebx+1]
					add	ebx,[ebx+1+4]
					add	ebx,[ebx+1+4]
					mov	al,[ebx+1+4]
					pop	ebx
				.else
					pop	ebx
					.if dword ptr[ebx+1]
						add	ebx,[ebx+1]
						add	ebx,[ebx+1+4]
						add	ebx,[ebx+1+4]
						add	ebx,[ebx+1+4]
						mov	al,[ebx+1+4]
					.endif
				.endif
			.endif
		.else
			pop	ebx
			.if dword ptr[ebx+1]
				add	ebx,[ebx+1]
				add	ebx,[ebx+1+4]
				add	ebx,[ebx+1+4]
				add	ebx,[ebx+1+4]
				mov	al,[ebx+1+4]
			.endif
		.endif
	.endif
	movzx	eax,al
	ret
geoip_get_country_by_ip	ENDP

geoip_get_n_countries	PROC
	mov	eax,num_c
	ret
geoip_get_n_countries	ENDP

geoip_get_country	PROC	uses ebx lpCountry:DWORD
	xor	ecx,ecx
	lea	ebx,c_names
	mov	edx,lpCountry
	mov	dx,[edx]
	or	dx,2020h
	.while ecx<num_c
		mov	ax,[ebx]
		or	ax,2020h
		.break .if dx==ax
		.while (byte ptr[ebx]!=0)
			inc	ebx
		.endw
		inc	ebx
		.while (byte ptr[ebx]!=0)
			inc	ebx
		.endw
		inc	ebx
		.break .if byte ptr[ebx]==0
		inc	ecx
	.endw
	.if ecx==num_c
		xor	ecx,ecx
		dec	ecx
	.endif
	mov	eax,ecx
	ret
geoip_get_country	ENDP

geoip_reverse	PROC	raddr:DWORD
	mov	eax,raddr
	xchg	al,ah
	rol	eax,16
	xchg	al,ah
	ret
geoip_reverse	ENDP


atoi	PROC
	xor	eax,eax
	xor	ebx,ebx
	push	edx
getdec0:mov	bl,[esi]
	sub	bl,'0'
	cmp	bl,10
	jnc	_getdec
	inc	esi
	push	ebx
	xor	edx,edx
	mov	bl,10
	mul	ebx
	pop	ebx
	add	eax,ebx
	jmp	getdec0
_getdec:pop	edx
	ret
atoi	ENDP

itoa:	mov	ecx,10
	xor	edx,edx
	div	ecx
	or	eax,eax
	jz	_d1
	push	edx
	call	itoa
	pop	edx
_d1:	or	dl,30h
	mov	al,dl
	stosb
	ret


is_ip	PROC	uses esi ebx str1:DWORD
	mov	esi,str1
	.if (byte ptr[esi]<'0')||(byte ptr[esi]>'9')
		xor	eax,eax
		ret
	.endif
	call	atoi
	.if eax>255
		xor	eax,eax
		ret
	.endif
	.if (byte ptr[esi]!='.')
		xor	eax,eax
		ret
	.endif
	inc	esi
	.if (byte ptr[esi]<'0')||(byte ptr[esi]>'9')
		xor	eax,eax
		ret
	.endif
	call	atoi
	.if eax>255
		xor	eax,eax
		ret
	.endif
	.if (byte ptr[esi]!='.')
		xor	eax,eax
		ret
	.endif
	inc	esi
	.if (byte ptr[esi]<'0')||(byte ptr[esi]>'9')
		xor	eax,eax
		ret
	.endif
	call	atoi
	.if eax>255
		xor	eax,eax
		ret
	.endif
	.if (byte ptr[esi]!='.')
		xor	eax,eax
		ret
	.endif
	inc	esi
	.if (byte ptr[esi]<'0')||(byte ptr[esi]>'9')
		xor	eax,eax
		ret
	.endif
	call	atoi
	.if eax>255
		xor	eax,eax
		ret
	.endif
	xor	eax,eax
	inc	eax
	ret
is_ip	ENDP

FormatMemInt	PROC	uses edi lpstr:DWORD,value:DWORD
	mov	edi,lpstr
	.if value & 0c0000000h
		mov	eax,value
		rol	eax,2
		and	eax,3
		call	itoa
		mov	al,'.'
		stosb
		mov	eax,value
		shr	eax,20
		and	eax,3ffh
		lea	eax,[eax*4+eax]
		lea	eax,[eax*4+eax]
		shr	eax,8
		.if al<10
			mov	byte ptr[edi],'0'
			inc	edi
		.endif
		call	itoa
		mov	eax,'BG '
		stosd
	.elseif value&03ff00000h
		mov	eax,value
		rol	eax,2+10
		and	eax,3ffh
		call	itoa
		mov	al,'.'
		stosb
		mov	eax,value
		shr	eax,10
		and	eax,3ffh
		lea	eax,[eax*4+eax]
		lea	eax,[eax*4+eax]
		shr	eax,8
		.if al<10
			mov	byte ptr[edi],'0'
			inc	edi
		.endif
		call	itoa
		mov	eax,'BM '
		stosd
	.elseif value&0000ffc00h
		mov	eax,value
		shr	eax,10
		and	eax,3ffh
		call	itoa
		mov	al,'.'
		stosb
		mov	eax,value
		and	eax,3ffh
		lea	eax,[eax*4+eax]
		lea	eax,[eax*4+eax]
		shr	eax,8
		.if al<10
			mov	byte ptr[edi],'0'
			inc	edi
		.endif
		call	itoa
		mov	eax,'BK '
		stosd
	.else
		mov	eax,value
		and	eax,3ffh
		call	itoa
		mov	eax,'B '
		stosd
	.endif
	ret
FormatMemInt	ENDP

FormatMemInt64	PROC	uses edi lpstr:DWORD,lpValue:DWORD
	mov	eax,lpValue
	.if dword ptr[eax+4]
		mov	edi,lpstr
		mov	eax,[eax+4]
		.if eax&0ffffff00h
			shr	eax,8
			call	itoa
			mov	al,'.'
			stosb
			mov	eax,lpValue
			mov	edx,[eax]
			mov	eax,[eax+4]
			shl	edx,1
			rcl	eax,1
			shl	edx,1
			rcl	eax,1
			and	eax,3ffh
			lea	eax,[eax*4+eax]
			lea	eax,[eax*4+eax]
			shr	eax,8
			.if al<10
				mov	byte ptr[edi],'0'
				inc	edi
			.endif
			call	itoa
			mov	eax,'BT '
			stosd
		.else
			mov	eax,lpValue
			mov	edx,[eax]
			mov	eax,[eax+4]
			shl	edx,1
			rcl	eax,1
			shl	edx,1
			rcl	eax,1
			and	eax,3ffh
			call	itoa
			mov	al,'.'
			stosb
			mov	eax,lpValue
			mov	eax,[eax]
			shr	eax,20
			and	eax,3ffh
			lea	eax,[eax*4+eax]
			lea	eax,[eax*4+eax]
			shr	eax,8
			.if al<10
				mov	byte ptr[edi],'0'
				inc	edi
			.endif
			call	itoa
			mov	eax,'BG '
			stosd
		.endif
	.else
		mov	eax,[eax]
		invoke	FormatMemInt,lpstr,eax
	.endif
	ret
FormatMemInt64	ENDP

SortIPList	PROC	uses edi esi ebx listPtr:DWORD,hMem:DWORD
	local	tmp_ip:DWORD
	mov	edi,listPtr
	mov	esi,edi
	mov	edi,hMem
	lea	edi,[edi+32768]
	.while byte ptr[esi]
		.while (byte ptr[esi]<33)&&(byte ptr[esi]!=0)
			inc	esi
		.endw
		.if byte ptr[esi]==';'
			.while (byte ptr[esi]!=0)&&(byte ptr[esi]!=13)&&(byte ptr[esi]!=10)
				inc	esi
			.endw
		.else
			.if (byte ptr[esi]>='0')&&(byte ptr[esi]<='9')
				call	atoi
				.if (eax<256)&&(byte ptr[esi]=='.')&&(byte ptr[esi+1]>='0')&&(byte ptr[esi+1]<='9')
					mov	byte ptr tmp_ip,al
					inc	esi
					call	atoi
					.if (eax<256)&&(byte ptr[esi]=='.')&&(byte ptr[esi+1]>='0')&&(byte ptr[esi+1]<='9')
						mov	byte ptr tmp_ip[1],al
						inc	esi
						call	atoi
						.if (eax<256)&&(byte ptr[esi]=='.')&&(byte ptr[esi+1]>='0')&&(byte ptr[esi+1]<='9')
							mov	byte ptr tmp_ip[2],al
							inc	esi
							call	atoi
							.if eax<256
								mov	byte ptr tmp_ip[3],al
								.if byte ptr[esi]==':'
									inc	esi
									call	atoi
								.else
									mov	eax,443
								.endif
								stosw
								mov	eax,tmp_ip
								xchg	al,ah
								rol	eax,16
								xchg	al,ah
								stosd
								mov	eax,tmp_ip
								invoke	geoip_get_country_by_ip,eax
								invoke	geoip_get_country_name,eax
								mov	ax,[eax]
								.if al==0
									mov	ax,'??'
								.endif
								xchg	al,ah
								stosw
								mov	eax,esi
								stosd
							.endif
						.endif
					.endif
				.endif
			.endif
			.while (byte ptr[esi]!=0)&&(byte ptr[esi]!=13)&&(byte ptr[esi]!=10)
				inc	esi
			.endw
		.endif
	.endw
	mov	esi,hMem
	lea	esi,[esi+32768]
	.while esi<edi
		lea	edx,[esi+12]
		mov	eax,[esi]
		mov	ecx,[esi+4]
		.while edx<edi
			.if (ecx>[edx+4])||((ecx==[edx+4])&&(eax>[edx]))
				xchg	eax,[edx]
				mov	[esi],eax
				xchg	ecx,[edx+4]
				mov	[esi+4],ecx
				push	dword ptr[edx+8]
				push	dword ptr[esi+8]
				pop	dword ptr[edx+8]
				pop	dword ptr[esi+8]
			.endif
			lea	edx,[edx+12]
		.endw
		lea	esi,[esi+12]
	.endw
	xor	eax,eax
	stosd
	mov	esi,hMem
	lea	esi,[esi+32768]
	mov	cx,-1
	mov	edi,hMem
	.while dword ptr[esi]
		mov	eax,[esi]
		.if eax==[esi+12]
			mov	eax,[esi+4]
			.if eax==[esi+12+4]
				lea	esi,[esi+12]
				.continue
			.endif
		.endif
		mov	ax,[esi+6]
		xchg	al,ah
		.if cx!=ax
			mov	cx,ax
			mov	ax,' ;'
			stosw
			mov	ax,cx
			stosw
			mov	ax,0a0dh
			stosw
		.endif
		lodsw
		push	ecx
		mov	eax,[esi]
		xchg	al,ah
		rol	eax,16
		xchg	al,ah
		mov	[esi],eax
		lodsb
		movzx	eax,al
		call	itoa
		mov	al,'.'
		stosb
		lodsb
		movzx	eax,al
		call	itoa
		mov	al,'.'
		stosb
		lodsb
		movzx	eax,al
		call	itoa
		mov	al,'.'
		stosb
		lodsb
		movzx	eax,al
		call	itoa
		mov	al,':'
		stosb
		lodsw
		mov	ax,[esi-8]
		call	itoa
		pop	ecx
		lodsd
		mov	edx,eax
		.while (byte ptr[edx]!=0)&&(byte ptr[edx]!=13)&&(byte ptr[edx]!=10)
			mov	al,[edx]
			stosb
			inc	edx
		.endw
		mov	ax,0a0dh
		stosw
	.endw
	mov	al,0
	stosb
	mov	eax,hMem
	ret
	ret
SortIPList	ENDP

RemoveComments	PROC	uses esi edi listPtr:DWORD
	mov	esi,listPtr
	mov	edi,listPtr
	.while (byte ptr[esi]!=0)&&(byte ptr[esi]<33)
		inc	esi
	.endw
	.while byte ptr[esi]
		.if byte ptr[esi]==';'
			.while (byte ptr[esi]!=0)&&(byte ptr[esi]!=13)&&(byte ptr[esi]!=10)
				inc	esi
			.endw
			.while (edi!=listPtr)&&((byte ptr[edi]==32)||(byte ptr[edi]==9))
				dec	edi
			.endw
		.else
			movsb
		.endif
	.endw
	mov	al,0
	stosb
	ret
RemoveComments	ENDP

end
