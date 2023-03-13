//////////////////////////////////////////////////////////////////////
//                                                                  //
//   Server_IP_Qt.cpp - ����� ��� ������� ��� ������ �� ���         //
//                      ����� ������ IP                             //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

//  ��������
...................
#include <SockIP_Qt.h>
...................


// ����� �������� ����������� �� ������� ������ ����������
// � ������ ��������� � ��� ����������
// wParam -   READY_WRITE ��� ������ �� ������� ������ � ����
//            DISCONNECT_SLOT ��� ������ �� ������� ��� ����������
//            READY_READ_SLOT ��� ������ �� ������� ��� ��������� ������ ��� ������
// lParam1 - ������ ��������� ��� ������ � ������ ���������� 
// lParam2 - ID ������ ��� ������ �� ������� READY_READ_SLOT

BEGIN_DZQT (DZWaitConnectToSource)
{
 qint64 n64NomThisDZ;                 // ����� ����� ������
 int nRet;

 COMMONDATA CommonData;          // ����� ������ ��� ����������
 CONFIGSRV ConfigSRV;            // ������������ ����������
 DATACOMPL* pDataCompl;          // ������ �� ����������
 LANADDRESS* pLinkData;          // ������ ��� ������ �� ����

 OBJECTSOCKET* pObjectSocket;    //  ������ ������
 SockIPQt*   pSocket;            //  �����
 QString* szAddrIP;              //  IP-�����
 quint64 nSocketID;              //  ������ ����������� ������ ��� �������

 BYTE* pbyMess;                  //  ��������� ��� ������ ��� ������
 HEADRMES* pHeader;              //  ��������� ���������
 DWORD dwLenMess;                //  ����� ���������

 VARIABLEDZREAD* pVarDZConnect;  // ���������� ��� ����� ������
 VARIABLEDZREAD* pVarDZRead;     // ���������� ��� ������ ������ �� ���� �� ������� ����������

 float fTime;

 //  ��������� ���
 //  ����������� ���� ��� �� ������ ����� ���������
 BEGIN_START_DZQT

 // ��������� ����������
 pLinkData=&(CommonData.LinkData.AddrCompl);
 pDataCompl=&(ConfigSRV.DataSRV);
 pVarDZConnect=&g_DZConnectVar;


 // ��������� ������ ��� �����
 pVarDZConnect->nMessSize=MAX_MESS_LAN_SIZE; 
 pVarDZConnect->pbyMess=new BYTE[pVarDZConnect->nMessSize];

 // �������� ������ ��� ������ ������
 pVarDZConnect->pStringTech.szOutput = new QString (); 
 pVarDZConnect->pStringTech.pText = new QTextStream (pVarDZConnect->pStringTech.szOutput);

 //����� ��������� �������� ���������
 fTime=GetTime_MUA();
 pVarDZConnect->nCountRecv=0;
 pVarDZConnect->nCountRecvAll=0;
 pVarDZConnect->fTimeLastRead=fTime;
 pVarDZConnect->fTimePrint1=fTime;

 // ����� ����������� ���������
 pObjectSocket=NULL;

 // �������� ������� ��� ������
 pObjectSocket= new OBJECTSOCKET;
 if (pObjectSocket != NULL)
    {
     // ��������� ��������� ��������
     pObjectSocket->pSocket=NULL;
     pObjectSocket->bAddressReady=TSRF_NOT_ADDR;
     pObjectSocket->byLinkState=TSCS_FAIL;
     g_pObjectSocketSRV=pObjectSocket;
	 pObjectSocket->n64NomDZRead=g_nNomDZReadSourceMess_PNA;
	 // ������ �������
     nRet=g_pConfigSRV->Read (&ConfigSRV,          //  ���� ��������� ������
                              0,         //  ������� � ������ ��������
                              1);							  
     if (nRet == RC_OK)
        {
         nRet=g_pCommonData->Read (&CommonData,
                                   0,         //  ������� � ������ ��������
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
		     // ���������� ���� � ������ ������ ����������
		     g_DZConnectVar.byCodeSend=CommonData.LinkData.byCodeCompl;
		     g_DZConnectVar.dwNumSend=CommonData.LinkData.wNumCompl;
		    }
        }
    }
 // ������ ���������� ��� ������ ��������� ��������� �� ������� ����������
 nSocketID=0;
 while (nSocketID < MAX_ARM+MAX_NO_ARM)
    {
     g_nNomDZProcessMessKPClient_PNA[nSocketID]=0;
	 g_DZProcessVar[nSocketID].byCodeSend=0;      //  ��� ����������-�������
	 g_DZProcessVar[nSocketID].dwNumSend=0;       //  ����� ����������-�������
	 g_DZProcessVar[nSocketID].pObjectSocket=NULL;
     nSocketID++;
    }

 //  ���� ���������
 BEGIN_PROC_DZQT

 // ��������� ������ ������
 n64NomThisDZ=GetNomCurrentDZ_Qt ();

 // ��������� ����������
 pObjectSocket=g_pObjectSocketSRV;
 pVarDZConnect=&g_DZConnectVar;

 if (pObjectSocket != NULL)
    {
     // ��������� �� ��������
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
				     // ��������� ������� ����������� �����������
				     nRet=pSocket->SetStatusConnectionServerIPQt (nSocketID);					

	                 fTime=GetTime_MUA();
                     if (nRet == RCSOCK_OK)
				        {							  

					     // �������� ������ ��������� ��������� �� ������� ����������
				         nRet=DeleteDZ_Qt (g_nNomDZProcessMessKPClient_PNA[nSocketID]);
                         if (nRet == RCSOCK_OK)
				            {
					         // �������� OBJECTSOCKET - ��� �������� ������ ������ ��������� ���������
							 g_nNomDZProcessMessKPClient_PNA[nSocketID]=0;
	                         g_DZProcessVar[nSocketID].byCodeSend=0;      //  ��� ����������-�������
	                         g_DZProcessVar[nSocketID].dwNumSend=0;       //  ����� ����������-�������

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
                     // ������ ������ �� ������
                    dwLenMess=pVarDZConnect->nMessSize;
                    nRet=pSocket->ReadSocketIPQt ((void*) pbyMess,       //  ����� ������
                                                  (UINT*)&dwLenMess,
												  nSocketID);         
                    switch (nRet)
                       {                  
                        case RCSOCK_OK: 
                           { // �������� ������
							// ������ �� ����� ������ ���������
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
                           { // ������ ��� ������ 
                             break;
                           } 
			            case RCSOCK_TIMEOUT:
                           {                                                                                   
                            break;
                           }
                       }  // end switch 
                    }
			     break;			
			    }  // ����� ��������� ������
		    }
		 return;
	    }

     if (MesDZ.n64NomSourceDZ == NOM_DZ_OPOQT)
        {
         // ��������� �� ���������, �������� �����������
         pSocket=pObjectSocket->pSocket;
         if (pObjectSocket->bAddressReady == TSRF_OK)
            { 
             if (pObjectSocket->byLinkState == TSCS_FAIL) 
                { // ������ ��������� - �������� "������"-�������
			     //  IP-�����
                 szAddrIP=new QString (pObjectSocket->szSelfAddrIP);  

                 // �������� ������ 
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
             // �������� �����������
		     if (pSocket != NULL)
		        {
                  if (pObjectSocket->byLinkState == TSCS_WAITCON)
                     {
                      // ������� �����������
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
                             { // ������ �����������
					           fTime=GetTime_MUA();

					          // ������ ������ ������ �����������
					          pObjectSocket->n64NomDZConnect=n64NomThisDZ;
					          // �������� � �������� ������ ������
			                  pObjectSocket->pSocketSlots= new SockIPQtSlots (n64NomThisDZ,n64NomThisDZ,nSocketID);
						  
						      bool bRet;
					          bRet=QObject::connect(pSocket->pSocketSelf, SIGNAL(readyRead ()),
                                                    pObjectSocket->pSocketSlots, SLOT(ReadReady()),
									                Qt::DirectConnection);
						      qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError") ;
					          bRet=QObject::connect(pSocket->pSocketSelf, SIGNAL(stateChanged (QAbstractSocket::SocketState)),
                                                    pObjectSocket->pSocketSlots, SLOT(NewStatus(QAbstractSocket::SocketState)),
									                Qt::DirectConnection);

 							  // ������ ��������� �� ������ ������
							  pVarDZConnect->pObjectSocket=pObjectSocket;
							  pVarDZConnect->fTimeLastRead=fTime;
							  pVarDZConnect->fTimePrint1=fTime;

							  // �������� ������ ��������� ��������� �� ������� ����������
                              nRet = SetupDZ_Qt (g_nNomDZProcessMessKPClient_PNA[nSocketID], DZProcessMessKPClient_PNA,
                                                 _T("DZProcessMessKPClient_PNA"), INFINITE, PR_NORMAL, 
                                                 nSocketID+1);
							  if (nRet == RC_OK)
							     {
							      // ������ ������ ��������� ��������� �� ������� ����������
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
                         } // ����� switch
                     }
                 }
	         }

		 return;
	    }

	 // ������ ��� ������ � ����� �� ������� ������ ��������� � ������ ����������
     if (MesDZ.wParam == READY_WRITE &&
		 MesDZ.lParam1 > 0 &&
		 MesDZ.lParam1 < pVarDZConnect->nMessSize)
	    {
	     pSocket=pObjectSocket->pSocket;
		 if (pSocket != NULL)
		    {
             if (pObjectSocket->byLinkState == TSCS_WAITCON)
                {
			     pbyMess=pVarDZConnect->pbyMess;    //  ��������� ��� ������
				 pHeader=(HEADRMES*)pbyMess;
				 dwLenMess=MesDZ.lParam1;
				 CopyMemoryQt (pbyMess,(BYTE*)pvData,dwLenMess);

				 // ������ ������ � ���� �����������
				 pHeader->dwNumSend=g_DZConnectVar.dwNumSend;
                 pHeader->byCodeSend=g_DZConnectVar.byCodeSend;

				 // ����� ������ ��� ����������
                 if (pHeader->dwNumRes != NUM_ALL_ARM)
				    {
					 // ����� ID ������
					 nSocketID=0;
                     while (nSocketID < MAX_ARM+MAX_NO_ARM)
                        {
					     pVarDZRead=&g_DZProcessVar[nSocketID];
					     if (pVarDZRead->byCodeSend == pHeader->byCodeRes &&
						     pVarDZRead->dwNumSend == pHeader->dwNumRes)
							{
				             // ������ � �����
                             // ���������� RCSOCK_OK, RCSOCK_ERROR
                             nRet=pSocket->WriteSocketIPQt (pbyMess,(UINT*)&dwLenMess,0,nSocketID);
                             switch (nRet)
                                {
                                 case RCSOCK_OK:
                                    {							  
						             break;
                                    }    
                                 case RCSOCK_ERROR:
                                    {
					                  // ����� � ������. ����
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
 		             // ���� ������ "���� �����"
					 nSocketID=0;
                     nRet=g_pConfigSRV->Read (&ConfigSRV,          //  ���� ��������� ������
                                              0,         //  ������� � ������ ��������
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
				                 // ������ � �����
                                 // ���������� RCSOCK_OK, RCSOCK_ERROR
                                 nRet=pSocket->WriteSocketIPQt (pbyMess,(UINT*)&dwLenMess,0,nSocketID);
                                 switch (nRet)
                                    {
                                     case RCSOCK_OK:
                                        {
						                 // ����������� �������� ���������
	                                     FormObjReg_PNA(REG_MESSLAN_VIXOD,  // ��� �������
                                                        pbyMess, // ��������� �� ������ �������
                                                        (WORD)dwLenMess);
							  
						                 break;
                                        }    
                                     case RCSOCK_ERROR:
                                        {
					                      // ����� � ������. ����
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
    { // ������ ������ �� ��� ������

           // ����� � ������. ����
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
 //  ��� ����������

 // �������� ��������
 pObjectSocket=g_pObjectSocketSRV;
 pVarDZConnect=&g_DZConnectVar;

 // �������� ������ ���������
 delete pVarDZConnect->pbyMess;

 // �������� ����� ��� ������ 
 delete pVarDZConnect->pStringTech.szOutput;
 delete pVarDZConnect->pStringTech.pText;

 // ����� ��� ���������� ������ ������
 Sleep_Qt (1000);
 // ����������� ������� ��� ������
 if (pObjectSocket != NULL)
	{
	 // ����������� ������� ��� ������
	 delete pObjectSocket;
	 pObjectSocket=NULL;
	}

 END_DZQT
}
