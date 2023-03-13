//////////////////////////////////////////////////////////////////////
//                                                                  //
//   Server_IP_Qt.cpp - поток для сервера при обмене по ЛВС         //
//                      через сокеты IP                             //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

//  Описания
...................
#include <SockIP_Qt.h>
...................


// поток ожидания подключения со стороны других приложений
// и выдачи сообщений в эти приложения
// wParam -   READY_WRITE при вызове из функции выдачи в сеть
//            DISCONNECT_SLOT при вызове по сигналу при отключении
//            READY_READ_SLOT при вызове по сигналу при появлении данных для чтения
// lParam1 - размер сообщения для выдачи в другое приложение 
// lParam2 - ID сокета при вызове по сигналу READY_READ_SLOT

BEGIN_DZQT (DZWaitConnectToSource)
{
 qint64 n64NomThisDZ;                 // номер этого потока
 int nRet;

 COMMONDATA CommonData;          // общие данные для приложения
 CONFIGSRV ConfigSRV;            // конфигурация приложения
 DATACOMPL* pDataCompl;          // данные по приложения
 LANADDRESS* pLinkData;          // данные для обмена по сети

 OBJECTSOCKET* pObjectSocket;    //  объект сокета
 SockIPQt*   pSocket;            //  сокет
 QString* szAddrIP;              //  IP-адрес
 quint64 nSocketID;              //  индекс клиентского сокета для сервера

 BYTE* pbyMess;                  //  сообщение для записи или чтения
 HEADRMES* pHeader;              //  заголовок сообщения
 DWORD dwLenMess;                //  длина сообщения

 VARIABLEDZREAD* pVarDZConnect;  // переменные для этого потока
 VARIABLEDZREAD* pVarDZRead;     // переменные для потока чтения из сети от другого приложения

 float fTime;

 //  Стартовый код
 //  Выполняется один раз до начала цикла обработки
 BEGIN_START_DZQT

 // установка указателей
 pLinkData=&(CommonData.LinkData.AddrCompl);
 pDataCompl=&(ConfigSRV.DataSRV);
 pVarDZConnect=&g_DZConnectVar;


 // выделение памяти под буфер
 pVarDZConnect->nMessSize=MAX_MESS_LAN_SIZE; 
 pVarDZConnect->pbyMess=new BYTE[pVarDZConnect->nMessSize];

 // создание строки для вывода текста
 pVarDZConnect->pStringTech.szOutput = new QString (); 
 pVarDZConnect->pStringTech.pText = new QTextStream (pVarDZConnect->pStringTech.szOutput);

 //сброс счетчиков выданных кодограмм
 fTime=GetTime_MUA();
 pVarDZConnect->nCountRecv=0;
 pVarDZConnect->nCountRecvAll=0;
 pVarDZConnect->fTimeLastRead=fTime;
 pVarDZConnect->fTimePrint1=fTime;

 // сброс глобального указателя
 pObjectSocket=NULL;

 // создание объекта для сокета
 pObjectSocket= new OBJECTSOCKET;
 if (pObjectSocket != NULL)
    {
     // установка начальных значений
     pObjectSocket->pSocket=NULL;
     pObjectSocket->bAddressReady=TSRF_NOT_ADDR;
     pObjectSocket->byLinkState=TSCS_FAIL;
     g_pObjectSocketSRV=pObjectSocket;
	 pObjectSocket->n64NomDZRead=g_nNomDZReadSourceMess_PNA;
	 // чтение адресов
     nRet=g_pConfigSRV->Read (&ConfigSRV,          //  куда поместить данные
                              0,         //  начиная с какого элемента
                              1);							  
     if (nRet == RC_OK)
        {
         nRet=g_pCommonData->Read (&CommonData,
                                   0,         //  начиная с какого элемента
                                   1);							  
         if (nRet == RC_OK)
            {
             pObjectSocket->wSelfPortIP=pLinkData->wNumPort;
		     pObjectSocket->wParam1=pDataCompl->wNumCompl;
             CopyMemoryQt ((BYTE*)&pObjectSocket->szAbonAddrIP[0],
			               (BYTE*)&pLinkData->szAddress[0],
                           ADDR_LEN);
			 CopyMemoryQt ((BYTE*)&pObjectSocket->szSelfAddrIP[0],
			               (BYTE*)&CommonData.LinkData.AddrCompl.szAddress[0],
                           ADDR_LEN);
             pObjectSocket->bAddressReady=TSRF_OK;
		     pObjectSocket->pSocket=NULL;
		     // сохранение кода и номера своего приложения
		     g_DZConnectVar.byCodeSend=CommonData.LinkData.byCodeCompl;
		     g_DZConnectVar.dwNumSend=CommonData.LinkData.wNumCompl;
		    }
        }
    }
 // чистка параметров для потока обработки сообщений от другого приложения
 nSocketID=0;
 while (nSocketID < MAX_ARM+MAX_NO_ARM)
    {
     g_nNomDZProcessMessKPClient_PNA[nSocketID]=0;
	 g_DZProcessVar[nSocketID].byCodeSend=0;      //  код приложения-клиента
	 g_DZProcessVar[nSocketID].dwNumSend=0;       //  номер приложения-клиента
	 g_DZProcessVar[nSocketID].pObjectSocket=NULL;
     nSocketID++;
    }

 //  Цикл обработки
 BEGIN_PROC_DZQT

 // получение номера потока
 n64NomThisDZ=GetNomCurrentDZ_Qt ();

 // установка указателей
 pObjectSocket=g_pObjectSocketSRV;
 pVarDZConnect=&g_DZConnectVar;

 if (pObjectSocket != NULL)
    {
     // включение по сигналам
     if (MesDZ.n64NomSourceDZ == n64NomThisDZ)
        {    
		 nSocketID=MesDZ.lParam2;

         switch (MesDZ.wParam)
            {
	         case DISCONNECT_SLOT :
			    {
                 pSocket=pObjectSocket->pSocket;
                 if (pSocket != NULL)
                    {
				     // установка статуса клиентского подключения
				     nRet=pSocket->SetStatusConnectionServerIPQt (nSocketID);					

	                 fTime=GetTime_MUA();
                     if (nRet == RCSOCK_OK)
				        {							  

					     // удаление потока обработки сообщений от другого приложения
				         nRet=DeleteDZ_Qt (g_nNomDZProcessMessKPClient_PNA[nSocketID]);
                         if (nRet == RCSOCK_OK)
				            {
					         // удаление OBJECTSOCKET - для хранения номера потока обработки сообщения
							 g_nNomDZProcessMessKPClient_PNA[nSocketID]=0;
	                         g_DZProcessVar[nSocketID].byCodeSend=0;      //  код приложения-клиента
	                         g_DZProcessVar[nSocketID].dwNumSend=0;       //  номер приложения-клиента

						    }
				        }

				    }

				 break;
				}
		     case READY_READ_SLOT :
			    {
                 pSocket=pObjectSocket->pSocket;
                 pbyMess=pVarDZConnect->pbyMess;
                 if (pSocket != NULL)
                    {
                     // чтение данных из сокета
                    dwLenMess=pVarDZConnect->nMessSize;
                    nRet=pSocket->ReadSocketIPQt ((void*) pbyMess,       //  буфер данных
                                                  (UINT*)&dwLenMess,
												  nSocketID);         
                    switch (nRet)
                       {                  
                        case RCSOCK_OK: 
                           { // успешное чтение
							// заявка на поток приема сообщений
                            SetCallDZ_Qt (n64NomThisDZ,
							              g_nNomDZProcessMessKPClient_PNA[nSocketID],
								          READY_READ_SLOT,
									      dwLenMess,
										  nSocketID,
										  0,
									      (void*) pbyMess,
									      dwLenMess);

                            break;                   
                           }
                        case RCSOCK_ERROR:
                           { // ошибка при чтении 
                             break;
                           } 
			            case RCSOCK_TIMEOUT:
                           {                                                                                   
                            break;
                           }
                       }  // end switch 
                    }
			     break;			
			    }  // конец обработки чтения
		    }
		 return;
	    }

     if (MesDZ.n64NomSourceDZ == NOM_DZ_OPOQT)
        {
         // включение по периодике, ожидание подключения
         pSocket=pObjectSocket->pSocket;
         if (pObjectSocket->bAddressReady == TSRF_OK)
            { 
             if (pObjectSocket->byLinkState == TSCS_FAIL) 
                { // первое включение - создание "сокета"-сервера
			     //  IP-адрес
                 szAddrIP=new QString (pObjectSocket->szSelfAddrIP);  

                 // создание сокета 
                 pObjectSocket->pSocket= new SockIPQt (szAddrIP,
                                                       pObjectSocket->wSelfPortIP,
                                                       REGIM_SERVER_SOCK);
			     delete szAddrIP;
                 if (pObjectSocket->pSocket != NULL)
                    {
                     pSocket=pObjectSocket->pSocket;
                     nRet=pSocket->CreateSocketIPQt ();

                     if (nRet == RCSOCK_OK)
                        {
                         pObjectSocket->byLinkState=TSCS_WAITCON;
                        }
                    }
                }
             // проверка подключения
		     if (pSocket != NULL)
		        {
                  if (pObjectSocket->byLinkState == TSCS_WAITCON)
                     {
                      // попытка подключения
				      szAddrIP=new QString (pObjectSocket->szAbonAddrIP); 
					  nSocketID=0;
                      nRet=pSocket->WaitConnectSocketIPQt(0,
				                                          szAddrIP,
                                                          pObjectSocket->wAbonPortIP,
														  &nSocketID);
				      delete szAddrIP;
                      switch (nRet)
                         {
                          case RCSOCK_OK:
                             { // клиент подключился
					           fTime=GetTime_MUA();

					          // запись номера потока подключения
					          pObjectSocket->n64NomDZConnect=n64NomThisDZ;
					          // создание и привязка класса слотов
			                  pObjectSocket->pSocketSlots= new SockIPQtSlots (n64NomThisDZ,n64NomThisDZ,nSocketID);
						  
						      bool bRet;
					          bRet=QObject::connect(pSocket->pSocketSelf, SIGNAL(readyRead ()),
                                                    pObjectSocket->pSocketSlots, SLOT(ReadReady()),
									                Qt::DirectConnection);
						      qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError") ;
					          bRet=QObject::connect(pSocket->pSocketSelf, SIGNAL(stateChanged (QAbstractSocket::SocketState)),
                                                    pObjectSocket->pSocketSlots, SLOT(NewStatus(QAbstractSocket::SocketState)),
									                Qt::DirectConnection);

 							  // запись указателя на объект сокета
							  pVarDZConnect->pObjectSocket=pObjectSocket;
							  pVarDZConnect->fTimeLastRead=fTime;
							  pVarDZConnect->fTimePrint1=fTime;

							  // создание потока обработки сообщений от другого приложения
                              nRet = SetupDZ_Qt (g_nNomDZProcessMessKPClient_PNA[nSocketID], DZProcessMessKPClient_PNA,
                                                 _T("DZProcessMessKPClient_PNA"), INFINITE, PR_NORMAL, 
                                                 nSocketID+1);
							  if (nRet == RC_OK)
							     {
							      // запуск потока обработки сообщений от другого приложения
				                  nRet = StartDZ_Qt (g_nNomDZProcessMessKPClient_PNA[nSocketID]);	
							     }

                              break;
                             }
                          case RCSOCK_TIMEOUT:
                             {
                              break;
                             }
                          case RCSOCK_NOTCREATE:
                             {
                              break;
                             }
                         } // конец switch
                     }
                 }
	         }

		 return;
	    }

	 // заявка для записи в сокет от функции выдачи сообщения в другое приложение
     if (MesDZ.wParam == READY_WRITE &&
		 MesDZ.lParam1 > 0 &&
		 MesDZ.lParam1 < pVarDZConnect->nMessSize)
	    {
	     pSocket=pObjectSocket->pSocket;
		 if (pSocket != NULL)
		    {
             if (pObjectSocket->byLinkState == TSCS_WAITCON)
                {
			     pbyMess=pVarDZConnect->pbyMess;    //  сообщение для записи
				 pHeader=(HEADRMES*)pbyMess;
				 dwLenMess=MesDZ.lParam1;
				 CopyMemoryQt (pbyMess,(BYTE*)pvData,dwLenMess);

				 // запись номера и кода отправителя
				 pHeader->dwNumSend=g_DZConnectVar.dwNumSend;
                 pHeader->byCodeSend=g_DZConnectVar.byCodeSend;

				 // выбор сокета для получателя
                 if (pHeader->dwNumRes != NUM_ALL_ARM)
				    {
					 // поиск ID сокета
					 nSocketID=0;
                     while (nSocketID < MAX_ARM+MAX_NO_ARM)
                        {
					     pVarDZRead=&g_DZProcessVar[nSocketID];
					     if (pVarDZRead->byCodeSend == pHeader->byCodeRes &&
						     pVarDZRead->dwNumSend == pHeader->dwNumRes)
							{
				             // запись в сокет
                             // возвращает RCSOCK_OK, RCSOCK_ERROR
                             nRet=pSocket->WriteSocketIPQt (pbyMess,(UINT*)&dwLenMess,0,nSocketID);
                             switch (nRet)
                                {
                                 case RCSOCK_OK:
                                    {							  
						             break;
                                    }    
                                 case RCSOCK_ERROR:
                                    {
					                  // вывод в технол. окно
					                  fTime=GetTime_MUA();
                                      *pVarDZConnect->pStringTech.pText  << fTime <<
						                            " Error to KP ID " <<
						                            nSocketID <<
                                                    "  send  " << 
										           dwLenMess <<
										           " bytes";					  ;
                                      SetCallDZ_Qt (n64NomThisDZ, g_nNomDZToolsInfo_PNA, 2, 0L, 0L, 0,
                                                    pVarDZConnect->pStringTech.szOutput->data(),
                                                    sizeof(TCHAR)*(pVarDZConnect->pStringTech.szOutput->size()));
                                      pVarDZConnect->pStringTech.szOutput->clear();


						             break;
                                    }    
                                }  // end switch

							nSocketID=MAX_ARM+MAX_NO_ARM;
							}
						 nSocketID++;
					    }
				    }
				 else
				    {
 		             // цикл выдачи "всем АРМам"
					 nSocketID=0;
                     nRet=g_pConfigSRV->Read (&ConfigSRV,          //  куда поместить данные
                                              0,         //  начиная с какого элемента
                                              1);
                     if (nRet == RC_OK)
				        {					
                         while (nSocketID < MAX_ARM+MAX_NO_ARM && 
							    ConfigSRV.byAmARM > 0)
                            {
					         pVarDZRead=&g_DZProcessVar[nSocketID];
					         if (pVarDZRead->byCodeSend == CPCODE_ARM)
						        {
								 pHeader->dwNumRes=pVarDZRead->dwNumSend;
								 pHeader->byCodeRes=CPCODE_ARM;
				                 // запись в сокет
                                 // возвращает RCSOCK_OK, RCSOCK_ERROR
                                 nRet=pSocket->WriteSocketIPQt (pbyMess,(UINT*)&dwLenMess,0,nSocketID);
                                 switch (nRet)
                                    {
                                     case RCSOCK_OK:
                                        {
						                 // регистрация сетевого сообщения
	                                     FormObjReg_PNA(REG_MESSLAN_VIXOD,  // код объекта
                                                        pbyMess, // указатель на память объекта
                                                        (WORD)dwLenMess);
							  
						                 break;
                                        }    
                                     case RCSOCK_ERROR:
                                        {
					                      // вывод в технол. окно
					                      fTime=GetTime_MUA();
                                          *pVarDZConnect->pStringTech.pText  << fTime <<
						                            " Error to KP ID " <<
						                            nSocketID <<
                                                    "  send  " << 
										           dwLenMess <<
										           " bytes";
                                          SetCallDZ_Qt (n64NomThisDZ, g_nNomDZToolsInfo_PNA, 2, 0L, 0L, 0,
                                                        pVarDZConnect->pStringTech.szOutput->data(),
                                                        sizeof(TCHAR)*(pVarDZConnect->pStringTech.szOutput->size()));
                                          pVarDZConnect->pStringTech.szOutput->clear();


						                 break;
                                        }    
                                    }  // end switch

								 ConfigSRV.byAmARM--;
						        }
						     nSocketID++;
					        }
					    }
				    }
			    }
			}
		 return;
	    }
    }
 else
    { // объект сокета не был создан

           // вывод в технол. окно
		 fTime=GetTime_MUA();
         *pVarDZConnect->pStringTech.pText << fTime <<
                         "  Error ObjectSocket NULL !";					  ;
         SetCallDZ_Qt (n64NomThisDZ, g_nNomDZToolsInfo_PNA, 2, 0L, 0L, 0,
                       pVarDZConnect->pStringTech.szOutput->data(),
                       sizeof(TCHAR)*(pVarDZConnect->pStringTech.szOutput->size()));
                       pVarDZConnect->pStringTech.szOutput->clear();
         pVarDZConnect->pStringTech.szOutput->clear();

    }

 BEGIN_STOP_DZQT
 //  Код завершения

 // удаление объектов
 pObjectSocket=g_pObjectSocketSRV;
 pVarDZConnect=&g_DZConnectVar;

 // удаление буфера сообщения
 delete pVarDZConnect->pbyMess;

 // удаление строк для вывода 
 delete pVarDZConnect->pStringTech.szOutput;
 delete pVarDZConnect->pStringTech.pText;

 // пауза для завершения работы сокета
 Sleep_Qt (1000);
 // уничтожение объекта для сокета
 if (pObjectSocket != NULL)
	{
	 // уничтожение объекта для сокета
	 delete pObjectSocket;
	 pObjectSocket=NULL;
	}

 END_DZQT
}
