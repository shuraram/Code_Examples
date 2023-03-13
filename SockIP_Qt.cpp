//////////////////////////////////////////////////////////////////////
//                                                                  //
//   SockIP_Qt.cpp -  ������ ������� ������� IP � UDP               //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

//  ��������
#include <SockIP_Qt.h>
#if defined(QT_LINUX)  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

///////////////////// ����� ������ IP ////////////////////

// �����������
SockIPQt::SockIPQt (QString* pSelfAddr,     //  ����������� IP-�����
					WORD wSelfSocket,       //  ����� ������������ �����
                    BYTE byType)
{
 quint64 nSocketID; 

 //  ������� �������� ������
 bFlagCreateSocket=FALSE;
 // ������������ ������ ���������
 uMaxMessageSize=SOCK_BUFF_SIZE;
  // ���������� ����������
 bySockType=byType;
 wPortSelf=wSelfSocket;
 // ������ ������ ������
 pHostAddress= new QHostAddress(*pSelfAddr);
 // ���������� ������������ ��������
 nConnectedClientCount=0;
 // ���� ���������� ����������� � �������
 bSingleFlag=FALSE;

 nSocketID=0;
 while (nSocketID < SOCKETCLIENT_ID_MAX)
	{
	 pSocketClient[nSocketID]=NULL;
	 nSocketID++;
    }
}

// ����������
SockIPQt::~SockIPQt ()
{
 if (bySockType == REGIM_SERVER_SOCK)
    {
	 pServer->close();
    }
 delete pHostAddress;
}

// �������� ������
// ���������� RCSOCK_OK, RCSOCK_NOTCREATE
int SockIPQt::CreateSocketIPQt()
{
 // �������� ������
 pSocketSelf=NULL;
 // ����� �� ���� ������ 
 if (bySockType == REGIM_CLIENT_SOCK)
    {
	 pSocketSelf=new QTcpSocket();
    }
 else
    {
     pServer=new QTcpServer(); 
    }

return RCSOCK_OK;
}

// �������� �����������
// �� ����� � nSocketID ������� SOCKETCLIENT_SINGLE ��� ���������� �����������
// ���������� nSocketID - ������ ����������� ������ ��� �������
// ���������� RCSOCK_OK, RCSOCK_NOTCREATE, RCSOCK_TIMEOUT
int SockIPQt::WaitConnectSocketIPQt (DWORD dwTimeOut,       //  ����� ��������
	                       QString* ServerAddressString,    //  IP-����� �������
						   WORD uServerPort,                //  ����� ����� �������
						   quint64* nSocketID)              //  ������ ����������� ������ ��� �������
{
 quint32 nState;
 quint16 port;
 qint64 nSize;
 quint64 nCurrentSocketID; 

 if (bySockType == REGIM_CLIENT_SOCK)
    {
	 nState = pSocketSelf->state();
	 if (nState != pSocketSelf->ConnectingState)
	    {
	     if (nState == pSocketSelf->ConnectedState)
	        {
             // ��������� ������� ������
             nSize=(qint64)SOCK_BUFF_SIZE;
             pSocketSelf->setReadBufferSize (nSize);
             if (nSize != pSocketSelf->readBufferSize())
	             return RCSOCK_NOTCREATE;

		     return RCSOCK_OK;
	        }
		 else
		    {
		     // ������� �����������
		     pHostAddressServer=new QHostAddress (*ServerAddressString);
	 	     port=(quint16)uServerPort;
	 	     pSocketSelf->connectToHost (*pHostAddressServer,port,pSocketSelf->ReadWrite);
		     return RCSOCK_TIMEOUT;
		    }
	    }
	 else
	    {
		 return RCSOCK_TIMEOUT;
	    }
    }
 else
	{
	 // ������
     if (!pServer->isListening())
	    {
	     // ������ �������� ��� ������ ���������
		 if (*nSocketID == SOCKETCLIENT_SINGLE)
			 pServer->setMaxPendingConnections(1);
	     else
		     pServer->setMaxPendingConnections(SOCKETCLIENT_ID_MAX);

		 if (!(pServer->listen(*pHostAddress, (quint16) wPortSelf)))
			 return RCSOCK_NOTCREATE;
	    }
	 if (!pServer->hasPendingConnections())
		 return RCSOCK_TIMEOUT;
	 // ������ �����������
	 pSocketSelf=pServer->nextPendingConnection();
	 if (*nSocketID == SOCKETCLIENT_SINGLE)
	    {// ��������� �����������
	     pServer->close();
	     bSingleFlag=TRUE;
	    }
	 else
	    {
	     // ����� ���������� ������
	     nCurrentSocketID=0;
	     while (nCurrentSocketID < SOCKETCLIENT_ID_MAX)
	        {
		     if (pSocketClient[nCurrentSocketID] == NULL)
		        {
			     pSocketClient[nCurrentSocketID]=pSocketSelf;
			     *nSocketID=nCurrentSocketID;

			     nConnectedClientCount++;
			     nCurrentSocketID=SOCKETCLIENT_ID_MAX;
		        }
		     nCurrentSocketID++;
	        }
	    }
	}
 nState = pSocketSelf->state();
 if (nState == pSocketSelf->ConnectedState)
	{
     // ��������� ������� ������
     nSize=(qint64)SOCK_BUFF_SIZE;
     pSocketSelf->setReadBufferSize (nSize);
     if (nSize != pSocketSelf->readBufferSize())
	     return RCSOCK_NOTCREATE;	    
	 return RCSOCK_OK;
	}
 else
	return RCSOCK_NOTCREATE;	 
}

// �������� ��������� ����������
// ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockIPQt::GetStatusConnectionIPQt ()
{
 quint32 nState;
 nState = pSocketSelf->state();
 if (nState == pSocketSelf->UnconnectedState ||
     nState == pSocketSelf->HostLookupState ||
     nState == pSocketSelf->ConnectingState)
    {
     return RCSOCK_TIMEOUT;
    }
 else
    {
	 if (nState == pSocketSelf->ConnectedState)
	    {
	     return RCSOCK_OK;
	    }
	 else
	    {
         return RCSOCK_ERROR;
	    }
    }
}

//  ������ �� ������
//  ����������: RCSOCK_OK, RCSOCK_ERROR
int SockIPQt::ReadSocketIPQt (void* pReadBuffer,       //  ����� ������
                              UINT* pLenRead,
							  quint64 nSocketID)         
{
 QTcpSocket* pSocket;          // �����
 qint64 nLenRead;
 qint64 nBytesAvailable;

  if (bySockType == REGIM_SERVER_SOCK &&
	  !bSingleFlag)
     { // ������������� �����������
	  if (nSocketID < SOCKETCLIENT_ID_MAX)
	     {
		  pSocket=pSocketClient[nSocketID];
		  if (pSocket != NULL)
		     {
              nBytesAvailable=pSocket->bytesAvailable();
              if (nBytesAvailable > *pLenRead)
                  return RCSOCK_ERROR;
              nLenRead=pSocket->read((char*) pReadBuffer,nBytesAvailable);
		     }
		  else
             return RCSOCK_ERROR;
	     }
	  else
         return RCSOCK_ERROR;
     }
  else
    { // ��������� �����������
     nBytesAvailable=pSocketSelf->bytesAvailable();
     if (nBytesAvailable > *pLenRead)
         return RCSOCK_ERROR;
     nLenRead=pSocketSelf->read((char*) pReadBuffer,nBytesAvailable);
    }

 if (nBytesAvailable != nLenRead)
        return RCSOCK_ERROR;

 if (nLenRead > 0)
	{
     *pLenRead=nLenRead;
	 return RCSOCK_OK;
    }
 else
    if (nLenRead == -1)
       {
        return RCSOCK_ERROR;
       }
	else
	   {
        return RCSOCK_TIMEOUT;
	   }

}

//  ������ � �����
//  ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockIPQt::WriteSocketIPQt(void* pWriteBuffer,      //  ����� ������
                              UINT* pLenWrite,         //  ����� ������������/������� ���������� ����
                              DWORD dwTimeOut,         //  ����� ��������, ��
							  quint64 nSocketID)         
{
 qint64 nLenWrite;
 QTcpSocket* pSocket;          // �����

 // �������� �����
 if (*pLenWrite > SOCK_BUFF_SIZE)
    {
     return RCSOCK_ERROR;
    }
 // ������ � �����
 nLenWrite=(qint64)*pLenWrite;

 if (bySockType == REGIM_SERVER_SOCK &&
	 !bSingleFlag)
     { // ������������� �����������
	  if (nSocketID < SOCKETCLIENT_ID_MAX)
	     {
		  pSocket=pSocketClient[nSocketID];
		  if (pSocket != NULL)
		     {
              nLenWrite=pSocket->write((char*)pWriteBuffer,nLenWrite);
              if (nLenWrite == *pLenWrite)
                 {
	              pSocket->flush ();
                  return RCSOCK_OK;
                 }
              else
                 return RCSOCK_ERROR;
		     }
		  else
             return RCSOCK_ERROR;
	     }
	  else
         return RCSOCK_ERROR;
     }
 else 
    { // ��������� �����������
     nLenWrite=pSocketSelf->write((char*)pWriteBuffer,nLenWrite);
     if (nLenWrite == *pLenWrite)
        {
	     pSocketSelf->flush ();
         return RCSOCK_OK;
        }
     else
        return RCSOCK_ERROR;
    }

}

//  ��������� ������� ����������� ����������� (��� ������� ��� ������������� �����������)
// ���������� RCSOCK_OK, RCSOCK_ERROR
int SockIPQt::SetStatusConnectionServerIPQt (quint64 nSocketID)
{
 if (nSocketID < SOCKETCLIENT_ID_MAX)
    {
	 if (pSocketClient[nSocketID] != NULL )
	    {
	     pSocketClient[nSocketID]=NULL;
	     nConnectedClientCount--;
		 return RCSOCK_OK;
	    }
	 else
		return RCSOCK_ERROR;
	}
 else
	return RCSOCK_ERROR;

}

//////////////////////////////////////////////////////////////////


/////////////////////// ����� ������ ��� ������ �� ///////////////

// �����������
SockIPQtSlots::SockIPQtSlots (qint64 n64NomThisDZ,
							  qint64 n64NomHandlingDZ,
							  quint64 nSockID)  //  ������ ����������� ������ ��� ������������� �����������
{
 n64NomDZConnect=n64NomThisDZ;
 n64NomDZRead=n64NomHandlingDZ;
 nSocketID=nSockID;
}

// ����������
SockIPQtSlots::~SockIPQtSlots ()
{
}

// ���� ��������� ������ ��� ������
void SockIPQtSlots :: ReadReady() 
{
 qint64 nSockID=SOCKETCLIENT_SINGLE;
 if (nSocketID != SOCKETCLIENT_SINGLE)
    nSockID=nSocketID;
 // ������ �� �� ������ ���������
 SetCallDZ_Qt (n64NomDZConnect,n64NomDZRead,READY_READ_SLOT,0,nSockID);
}

// ���� ���������� �������� (��� �������)
void SockIPQtSlots :: Disconnect() 
{
 // ������ �� �� ������ ���������
 SetCallDZ_Qt (n64NomDZConnect,n64NomDZRead,DISCONNECT_SLOT,0);
}

// ���� ��������� ��������� ������ (��� �������)
void SockIPQtSlots :: NewStatus(QAbstractSocket::SocketState nStatus) 
{
 qint64 nSockID=SOCKETCLIENT_SINGLE;
 if (nStatus == QAbstractSocket::UnconnectedState)
    {
     if (nSocketID != SOCKETCLIENT_SINGLE)
         nSockID=nSocketID;
     // ������ �� �� ������ ���������
     SetCallDZ_Qt (n64NomDZConnect,n64NomDZConnect,DISCONNECT_SLOT,0,nSockID);
    }
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////// ����� ������ UDP ///////////////////////////////////

// �����������
SockUDPQt::SockUDPQt ()
{
 //  ������� �������� ������
 bFlagCreateSocket=FALSE;
 // ������������ ������ ���������
 uMaxMessageSize=8192;           
}

// ����������
SockUDPQt::~SockUDPQt ()
{
 delete pHostAddress;
 if (pSocket != NULL)
    delete pSocket;
}

// �������� ������
// ���������� RCSOCK_OK, RCSOCK_NOTCREATE
int SockUDPQt::CreateSocket(WORD wSocketSelf,
                            WORD wAbonSocket,       //  ����� ����� ��������
					        QString* pSelfAddr,     //  IP-����� ����
					        QString* pAbonAddr,     //  IP-����� ��������
                            BYTE byType)            //  ��� ������ - ����� ��� ������
{
 BOOL bFlagBind;

 #ifdef QT_LINUX
 quint32 nIPAddr;
 #endif

 // ���������� ����������
 bySockType=byType;
 wPortSelf=wSocketSelf;
 wPortAbon=wAbonSocket;

  // ������ �������
 pHostAddress= new QHostAddress(*pSelfAddr);
 pHostAddressAbon= new QHostAddress(*pAbonAddr);
 //n64HostAddressAbon=(qint64)pHostAddressAbon->toIPv4Address();

 // �������� ������
 pSocket=NULL;
 pSocket=new QUdpSocket();
 if (byType == REGIM_RECEIVE_SOCK)
    {
     bFlagBind=pSocket->bind (*pHostAddress,
                              (quint16) wPortSelf, 
                              //pSocket->DontShareAddress);
                              pSocket->ShareAddress | pSocket->ReuseAddressHint);
	 // bFlagBind=TRUE;
     if (bFlagBind)
        {
          //  ������� �������� ������
          bFlagCreateSocket=TRUE;                  
          return RCSOCK_OK;
        }
     else
        {
         //  ������� �������� ������
         bFlagCreateSocket=FALSE;           
         return RCSOCK_NOTCREATE;        
        }
    }
 else
    {
     // �������� ������ - ��� ������ "�� ������������� �����"
	 bFlagBind = pSocket->bind(*pHostAddress,
		                       (quint16)wPortSelf,
		                       //pSocket->DontShareAddress);
		                       pSocket->ShareAddress | pSocket->ReuseAddressHint);
	 // bFlagBind=TRUE;
	 if (bFlagBind)
	    {
		 //  ������� �������� ������
		 bFlagCreateSocket = TRUE;
		 return RCSOCK_OK;
	    }
	 else
	    {
		 //  ������� �������� ������
		 bFlagCreateSocket = FALSE;
		 return RCSOCK_NOTCREATE;
	    }

    }

}

// ������ � �����
// ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockUDPQt::WriteSocket(void* pWriteBuffer,UINT* pLenWrite,DWORD dwTimeOut)
{
 UINT uLen;
 int nRet;

 // �������� ������������ �����
 if (*pLenWrite > uMaxMessageSize)
    { 
     return RCSOCK_ERROR;
    }

 uLen=*pLenWrite;
 nRet=pSocket->writeDatagram ((char*) pWriteBuffer,
                              (qint64) uLen, 
                              *pHostAddressAbon, 
                              (quint16) wPortAbon);
 if (nRet == -1)
    {
     //pSocket->error();
     return RCSOCK_ERROR;
    }

 if ((UINT)nRet != uLen)
    {
     return RCSOCK_ERROR;
    }
 
 return RCSOCK_OK; 

}

//  ������ �� ������
//  ����������: RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockUDPQt::ReadSocket (void* pReadBuffer,       //  ����� ������
                           UINT* pLenRead,          //  ������ ������, ����/����� ����������� ����
                           DWORD dwTimeOut,         //  ����� ��������, ��
					       BYTE  byControlSender)   //  ������� �������� ������ �����������
{
 int nInLength;
 int nTimeOut;
 qint64 n64HostAddressSender;


 if (dwTimeOut == 0)
    nTimeOut=-1;
 else
    nTimeOut=(int)dwTimeOut;
     
 if (pSocket->hasPendingDatagrams())
    {
     // ���� ������������� ���������
     nInLength=pSocket->pendingDatagramSize();
     if (nInLength > 0)
        {
         if ((UINT)nInLength > *pLenRead)
            {
             // ������� ������� ���������, �������� � ��������
             *pLenRead=nInLength;
             pSocket->readDatagram ((char*)pReadBuffer,0,0,0);
             return RCSOCK_ERROR;
            }
         else
            {             
             // ������ ���������
			 if (byControlSender == CONTROL_SENDER_UDP_OFF)
			    {
                 nInLength=pSocket->readDatagram ((char*)pReadBuffer,*pLenRead,0,0);
			    }
			 else
			    {
			     nInLength=pSocket->readDatagram ((char*)pReadBuffer,*pLenRead,&HostAddressSender,0);
			    }
             if (nInLength != -1)
                {
                 *pLenRead=nInLength;
				 if (byControlSender == CONTROL_SENDER_UDP_ON)
				    {
					 // �������� IP-������ �����������
					 //n64HostAddressSender=(qint64)HostAddressSender.toIPv4Address();
					 if (HostAddressSender != *pHostAddressAbon)
					    return RCSOCK_TIMEOUT;
				    }
                 return RCSOCK_OK;
                }
             else
                {
                 *pLenRead=0;
                 return RCSOCK_ERROR;
                }
            }
        }
    }

 // �������� ������ ��� ������
 if (pSocket->waitForReadyRead(nTimeOut))
    {
     // ���� ������������� ���������
     nInLength=pSocket->pendingDatagramSize();
     if (nInLength > 0)
        {
         if ((UINT)nInLength > *pLenRead)
            {
             // ������� ������� ���������, �������� � ��������
             *pLenRead=nInLength;
             pSocket->readDatagram ((char*)pReadBuffer,0,0,0);
             return RCSOCK_ERROR;
            }
         else
            {             
             // ������ ���������
			 if (byControlSender == CONTROL_SENDER_UDP_OFF)
			    {
                 nInLength=pSocket->readDatagram ((char*)pReadBuffer,*pLenRead,0,0);
			    }
			 else
			    {
			     nInLength=pSocket->readDatagram ((char*)pReadBuffer,*pLenRead,&HostAddressSender,0);
			    }
             if (nInLength != -1)
                {
                 *pLenRead=nInLength;
				 if (byControlSender == CONTROL_SENDER_UDP_ON)
				    {
					 // �������� IP-������ �����������
					 //n64HostAddressSender=(qint64)HostAddressSender.toIPv4Address();
					 //if (n64HostAddressSender != n64HostAddressAbon)
					 if (HostAddressSender != *pHostAddressAbon)
					    return RCSOCK_TIMEOUT;
				    }                 
				 return RCSOCK_OK;
                }
             else
                {
                 *pLenRead=0;
                 return RCSOCK_ERROR;
                }
            }
        }
     else
        {
         *pLenRead=0;
         return RCSOCK_ERROR;         
        }
    }
 else
    {
     // ������� ����� ��� ������
     return RCSOCK_TIMEOUT;
    } 

}
 
// ��������� �������� �������� ������
// TRUE - ����� ������, FALSE - ����� �� ������
BOOL SockUDPQt::GetFlagCreate()
{

 return bFlagCreateSocket;

}

//  ����������� ������ � ������
//  ����������: RCSOCK_OK, RCSOCK_ERROR
int SockUDPQt::ConnectMulticast(char* pMulticastAddr)    //  ����������������� IP-�����
{
 int SocketID;  // ���������� ������

 SocketID=pSocket->socketDescriptor();
 if (SocketID == -1)
	return RCSOCK_ERROR;

 // ���� �����
 pString= new QString ();
 *pString= pHostAddress->toString ();

 // ��� Windows
 #if !defined(QT_LINUX)  
 ip_mreq mreq;  // ��������� ��� ���������� � ������ ������, ���� Ws2ipdef.h

 // ��������� �����
 mreq.imr_multiaddr.s_addr = inet_addr(pMulticastAddr); 
 //mreq.imr_interface.s_addr = inet_addr("192.168.4.102"); 
 //mreq.imr_interface.s_addr = inet_addr((char*)pString->constData());
 mreq.imr_interface.s_addr = inet_addr((char*)(pString->toLatin1()).constData()); 

 if(setsockopt(SocketID, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq,sizeof(mreq)) != 0)
 	return RCSOCK_ERROR;
 // ����� ��� Windows

 // ��� LINUX
 #else 
 ip_mreq mreq;

 mreq.imr_interface.s_addr = inet_addr(pMulticastAddr); 
 mreq.imr_multiaddr.s_addr = inet_addr((char*)(pString->toAscii()).constData()); 
 // mreq.imr_interface.s_addr = htonl(INADDR_ANY);
 if (setsockopt (SocketID,
					   IPPROTO_IP,
					   IP_ADD_MEMBERSHIP,
					   &mreq,
					   sizeof(mreq))  != 0)
	 {
	  return RCSOCK_ERROR;
	 };


 // ����� ��� LINUX
 #endif

 return RCSOCK_OK;

}

///////////////////////////////////////////////////////////