//////////////////////////////////////////////////////////////////////
//                                                                  //
//   SockIP_Qt.cpp -  методы классов сокетов IP и UDP               //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

//  Описания
#include <SockIP_Qt.h>
#if defined(QT_LINUX)  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

///////////////////// Класс сокета IP ////////////////////

// Конструктор
SockIPQt::SockIPQt (QString* pSelfAddr,     //  собственный IP-адрес
					WORD wSelfSocket,       //  номер собственного порта
                    BYTE byType)
{
 quint64 nSocketID; 

 //  признак создания сокета
 bFlagCreateSocket=FALSE;
 // максимальный размер сообщения
 uMaxMessageSize=SOCK_BUFF_SIZE;
  // сохранение параметров
 bySockType=byType;
 wPortSelf=wSelfSocket;
 // запись своего адреса
 pHostAddress= new QHostAddress(*pSelfAddr);
 // количество подключенных клиентов
 nConnectedClientCount=0;
 // флаг одиночного подключения к серверу
 bSingleFlag=FALSE;

 nSocketID=0;
 while (nSocketID < SOCKETCLIENT_ID_MAX)
	{
	 pSocketClient[nSocketID]=NULL;
	 nSocketID++;
    }
}

// Деструктор
SockIPQt::~SockIPQt ()
{
 if (bySockType == REGIM_SERVER_SOCK)
    {
	 pServer->close();
    }
 delete pHostAddress;
}

// создание сокета
// возвращает RCSOCK_OK, RCSOCK_NOTCREATE
int SockIPQt::CreateSocketIPQt()
{
 // создание сокета
 pSocketSelf=NULL;
 // выбор по типу сокета 
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

// ожидание подключения
// на входе в nSocketID признак SOCKETCLIENT_SINGLE для одиночного подключения
// возвращает nSocketID - индекс клиентского сокета для сервера
// возвращает RCSOCK_OK, RCSOCK_NOTCREATE, RCSOCK_TIMEOUT
int SockIPQt::WaitConnectSocketIPQt (DWORD dwTimeOut,       //  время ожидания
	                       QString* ServerAddressString,    //  IP-адрес сервера
						   WORD uServerPort,                //  номер порта сервера
						   quint64* nSocketID)              //  индекс клиентского сокета для сервера
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
             // установка размера буфера
             nSize=(qint64)SOCK_BUFF_SIZE;
             pSocketSelf->setReadBufferSize (nSize);
             if (nSize != pSocketSelf->readBufferSize())
	             return RCSOCK_NOTCREATE;

		     return RCSOCK_OK;
	        }
		 else
		    {
		     // попытка подключения
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
	 // сервер
     if (!pServer->isListening())
	    {
	     // запуск ожидания при первом включении
		 if (*nSocketID == SOCKETCLIENT_SINGLE)
			 pServer->setMaxPendingConnections(1);
	     else
		     pServer->setMaxPendingConnections(SOCKETCLIENT_ID_MAX);

		 if (!(pServer->listen(*pHostAddress, (quint16) wPortSelf)))
			 return RCSOCK_NOTCREATE;
	    }
	 if (!pServer->hasPendingConnections())
		 return RCSOCK_TIMEOUT;
	 // клиент подключился
	 pSocketSelf=pServer->nextPendingConnection();
	 if (*nSocketID == SOCKETCLIENT_SINGLE)
	    {// одиночное подключение
	     pServer->close();
	     bSingleFlag=TRUE;
	    }
	 else
	    {
	     // поиск свободного сокета
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
     // установка размера буфера
     nSize=(qint64)SOCK_BUFF_SIZE;
     pSocketSelf->setReadBufferSize (nSize);
     if (nSize != pSocketSelf->readBufferSize())
	     return RCSOCK_NOTCREATE;	    
	 return RCSOCK_OK;
	}
 else
	return RCSOCK_NOTCREATE;	 
}

// проверка установки соединения
// возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
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

//  Чтение из сокета
//  Возвращает: RCSOCK_OK, RCSOCK_ERROR
int SockIPQt::ReadSocketIPQt (void* pReadBuffer,       //  буфер данных
                              UINT* pLenRead,
							  quint64 nSocketID)         
{
 QTcpSocket* pSocket;          // сокет
 qint64 nLenRead;
 qint64 nBytesAvailable;

  if (bySockType == REGIM_SERVER_SOCK &&
	  !bSingleFlag)
     { // множественное подключение
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
    { // одиночное подключение
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

//  Запись в сокет
//  Возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockIPQt::WriteSocketIPQt(void* pWriteBuffer,      //  буфер данных
                              UINT* pLenWrite,         //  число записываемых/реально записанных байт
                              DWORD dwTimeOut,         //  время ожидания, мс
							  quint64 nSocketID)         
{
 qint64 nLenWrite;
 QTcpSocket* pSocket;          // сокет

 // проверка длины
 if (*pLenWrite > SOCK_BUFF_SIZE)
    {
     return RCSOCK_ERROR;
    }
 // запись в сокет
 nLenWrite=(qint64)*pLenWrite;

 if (bySockType == REGIM_SERVER_SOCK &&
	 !bSingleFlag)
     { // множественное подключение
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
    { // одиночное подключение
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

//  Установка статуса клиентского подключения (для сервера при множественном подключении)
// возвращает RCSOCK_OK, RCSOCK_ERROR
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


/////////////////////// Класс слотов для вызова ДЗ ///////////////

// Конструктор
SockIPQtSlots::SockIPQtSlots (qint64 n64NomThisDZ,
							  qint64 n64NomHandlingDZ,
							  quint64 nSockID)  //  индекс клиентского сокета при множественном подключении
{
 n64NomDZConnect=n64NomThisDZ;
 n64NomDZRead=n64NomHandlingDZ;
 nSocketID=nSockID;
}

// Деструктор
SockIPQtSlots::~SockIPQtSlots ()
{
}

// слот появления данных для чтения
void SockIPQtSlots :: ReadReady() 
{
 qint64 nSockID=SOCKETCLIENT_SINGLE;
 if (nSocketID != SOCKETCLIENT_SINGLE)
    nSockID=nSocketID;
 // заявка на ДЗ приема сообщений
 SetCallDZ_Qt (n64NomDZConnect,n64NomDZRead,READY_READ_SLOT,0,nSockID);
}

// слот отключения абонента (для клиента)
void SockIPQtSlots :: Disconnect() 
{
 // заявка на ДЗ приема сообщений
 SetCallDZ_Qt (n64NomDZConnect,n64NomDZRead,DISCONNECT_SLOT,0);
}

// слот изменения состояния сокета (для сервера)
void SockIPQtSlots :: NewStatus(QAbstractSocket::SocketState nStatus) 
{
 qint64 nSockID=SOCKETCLIENT_SINGLE;
 if (nStatus == QAbstractSocket::UnconnectedState)
    {
     if (nSocketID != SOCKETCLIENT_SINGLE)
         nSockID=nSocketID;
     // заявка на ДЗ приема сообщений
     SetCallDZ_Qt (n64NomDZConnect,n64NomDZConnect,DISCONNECT_SLOT,0,nSockID);
    }
}

/////////////////////////////////////////////////////////////////////////////


//////////////////////// Класс сокета UDP ///////////////////////////////////

// Конструктор
SockUDPQt::SockUDPQt ()
{
 //  признак создания сокета
 bFlagCreateSocket=FALSE;
 // максимальный размер сообщения
 uMaxMessageSize=8192;           
}

// Деструктор
SockUDPQt::~SockUDPQt ()
{
 delete pHostAddress;
 if (pSocket != NULL)
    delete pSocket;
}

// создание сокета
// возвращает RCSOCK_OK, RCSOCK_NOTCREATE
int SockUDPQt::CreateSocket(WORD wSocketSelf,
                            WORD wAbonSocket,       //  номер порта абонента
					        QString* pSelfAddr,     //  IP-адрес свой
					        QString* pAbonAddr,     //  IP-адрес абонента
                            BYTE byType)            //  тип сокета - прием или выдача
{
 BOOL bFlagBind;

 #ifdef QT_LINUX
 quint32 nIPAddr;
 #endif

 // сохранение параметров
 bySockType=byType;
 wPortSelf=wSocketSelf;
 wPortAbon=wAbonSocket;

  // запись адресов
 pHostAddress= new QHostAddress(*pSelfAddr);
 pHostAddressAbon= new QHostAddress(*pAbonAddr);
 //n64HostAddressAbon=(qint64)pHostAddressAbon->toIPv4Address();

 // создание сокета
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
          //  признак создания сокета
          bFlagCreateSocket=TRUE;                  
          return RCSOCK_OK;
        }
     else
        {
         //  признак создания сокета
         bFlagCreateSocket=FALSE;           
         return RCSOCK_NOTCREATE;        
        }
    }
 else
    {
     // привязка сокета - для выдачи "из определенного порта"
	 bFlagBind = pSocket->bind(*pHostAddress,
		                       (quint16)wPortSelf,
		                       //pSocket->DontShareAddress);
		                       pSocket->ShareAddress | pSocket->ReuseAddressHint);
	 // bFlagBind=TRUE;
	 if (bFlagBind)
	    {
		 //  признак создания сокета
		 bFlagCreateSocket = TRUE;
		 return RCSOCK_OK;
	    }
	 else
	    {
		 //  признак создания сокета
		 bFlagCreateSocket = FALSE;
		 return RCSOCK_NOTCREATE;
	    }

    }

}

// запись в сокет
// возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockUDPQt::WriteSocket(void* pWriteBuffer,UINT* pLenWrite,DWORD dwTimeOut)
{
 UINT uLen;
 int nRet;

 // проверка допустимости длины
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

//  Чтение из сокета
//  Возвращает: RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
int SockUDPQt::ReadSocket (void* pReadBuffer,       //  буфер данных
                           UINT* pLenRead,          //  размер буфера, байт/число прочитанных байт
                           DWORD dwTimeOut,         //  время ожидания, мс
					       BYTE  byControlSender)   //  признак проверки адреса отправителя
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
     // есть непрочитанные сообщения
     nInLength=pSocket->pendingDatagramSize();
     if (nInLength > 0)
        {
         if ((UINT)nInLength > *pLenRead)
            {
             // слишком большое сообщение, прочесть и выкинуть
             *pLenRead=nInLength;
             pSocket->readDatagram ((char*)pReadBuffer,0,0,0);
             return RCSOCK_ERROR;
            }
         else
            {             
             // чтение сообщения
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
					 // проверка IP-адреса отправителя
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

 // ожидание данных для чтения
 if (pSocket->waitForReadyRead(nTimeOut))
    {
     // есть непрочитанные сообщения
     nInLength=pSocket->pendingDatagramSize();
     if (nInLength > 0)
        {
         if ((UINT)nInLength > *pLenRead)
            {
             // слишком большое сообщение, прочесть и выкинуть
             *pLenRead=nInLength;
             pSocket->readDatagram ((char*)pReadBuffer,0,0,0);
             return RCSOCK_ERROR;
            }
         else
            {             
             // чтение сообщения
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
					 // проверка IP-адреса отправителя
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
     // истекло время или ошибка
     return RCSOCK_TIMEOUT;
    } 

}
 
// получение признака создания сокета
// TRUE - сокет создан, FALSE - сокет не создан
BOOL SockUDPQt::GetFlagCreate()
{

 return bFlagCreateSocket;

}

//  Подключение сокета к группе
//  Возвращает: RCSOCK_OK, RCSOCK_ERROR
int SockUDPQt::ConnectMulticast(char* pMulticastAddr)    //  широковещательный IP-адрес
{
 int SocketID;  // дескриптор сокета

 SocketID=pSocket->socketDescriptor();
 if (SocketID == -1)
	return RCSOCK_ERROR;

 // свой адрес
 pString= new QString ();
 *pString= pHostAddress->toString ();

 // для Windows
 #if !defined(QT_LINUX)  
 ip_mreq mreq;  // структура для группового и своего адреса, файл Ws2ipdef.h

 // групповой адрес
 mreq.imr_multiaddr.s_addr = inet_addr(pMulticastAddr); 
 //mreq.imr_interface.s_addr = inet_addr("192.168.4.102"); 
 //mreq.imr_interface.s_addr = inet_addr((char*)pString->constData());
 mreq.imr_interface.s_addr = inet_addr((char*)(pString->toLatin1()).constData()); 

 if(setsockopt(SocketID, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq,sizeof(mreq)) != 0)
 	return RCSOCK_ERROR;
 // конец для Windows

 // для LINUX
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


 // конец для LINUX
 #endif

 return RCSOCK_OK;

}

///////////////////////////////////////////////////////////