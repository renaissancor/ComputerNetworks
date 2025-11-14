Sent File to Server 

Header Protocol 

```cpp
struct st_PACKET_HEADER
{
   DWORD dwPacketCode; // 0x11223344 우리의 패킷확인 고정값

   WCHAR szName[32]; // 본인이름, 유니코드 utf-16 NULL 문자 끝
   WCHAR szFileName[128]; // 파일이름, 유니코드 utf-16 NULL 문자 끝
    int iFileSize;
};
```

Logic Order 

1 Get Access to Server 
Open Certain File 
Check Size 
Generate Header 
Send Header to Server 
Send File Data to Server, 1000 Bytes Unit 
After file transfer success, Server will send `0xdddddddd` and receive it 

First, find the IP address of the server from domain address 

```powershell
nslookup 
procademyserver.iptime.org
```

Response is Address : `106.245.38.108` 
Port number is designated as `10010`. 
Then, connect to the server using these IP and port
number. 