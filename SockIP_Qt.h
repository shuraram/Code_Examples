//////////////////////////////////////////////////////////////////////
//                                                                  //
//      SockIP_Qt.h - �������� ��� ������� ������� IP � UDP         //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#ifndef SOCKIP_QT_H
#define SOCKIP_QT_H

#if !defined(QT_LINUX)
    #pragma warning (disable:4127)
#endif
#include <QObject>
#include <QMetaType>
#include <QString>
#include <QIODevice>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QHostInfo>
#include <QHostAddress>
#if !defined(QT_LINUX)
    #pragma warning (default:4127)
#endif


// ���� ������ IP
#define REGIM_SERVER_SOCK 1
#define REGIM_CLIENT_SOCK 2

//  ���� ������ UDP
#define REGIM_RECEIVE_SOCK   1
#define REGIM_SEND_SOCK      2

// ������ ������ � ������ IP
const int SOCK_BUFF_SIZE=256000;

//  ����� �������� ���������� ������, ��
const DWORD TIME_OUT_READ_SOCK  = 100;
//  ����� �������� ���������� ������, ��
const DWORD TIME_OUT_WRITE_SOCK = 100;

//  ���� �������� ��� ������� ������ ������
const int RCSOCK_ERROR          = 100;  //  ������
const int RCSOCK_OK             =   0;  //  �����
const int RCSOCK_NOTCREATE      =   1;  //  ����� �� ������
const int RCSOCK_TIMEOUT        =   2;  //  ������� ����� ��������
const int RCSOCK_DISCONNECT     =   3;  //  ������ ����������

// �������� ������ �� ������ (��� �� ������) 
const WORD READY_READ_SLOT         =  0xAAA1;       
const WORD DISCONNECT_SLOT         =  0xAAA2;       
const WORD READY_WRITE_SLOT        =  0xAAA3; 

// ������� �������� ������ ����������� ��� ������ ��� ������ UDP
const BYTE CONTROL_SENDER_UDP_ON   =  0xFF;       
const BYTE CONTROL_SENDER_UDP_OFF  =  0x0;

//  ������������ ����� ����������� � �������  
const int SOCKETCLIENT_ID_MAX = 32;      
const int SOCKETCLIENT_SINGLE = 0xFF;      

//////////////////////// ����� ������ TCP\IP ////////////////////

class SockIPQt : public QObject
   {
    Q_OBJECT

    public:
    // �����������
    SockIPQt (QString* pSelfAddr,     //  ����������� IP-�����
	          WORD wSelfPort,         //  ����� ������������ �����
              BYTE byType);           //  ��� ������ - ������ ��� ������

    // ����������
    ~SockIPQt ();

    // �������� ������
    // ���������� RCSOCK_OK, RCSOCK_NOTCREATE
    int CreateSocketIPQt();
	
    // �������� ��������� ����������
    // ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int GetStatusConnectionIPQt ();

    // �������� �����������
	// �� ����� � nSocketID ������� SOCKETCLIENT_SINGLE ��� ���������� �����������
    // ���������� nSocketID - ������ ����������� ������ ��� �������
    // ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WaitConnectSocketIPQt (DWORD dwTimeOut,
	                           QString* ServerAddressString = NULL, // IP-����� �������
							   WORD uServerPort = 0,                //  ����� ����� �������
							   quint64* nSocketID = NULL);           //  ������ ����������� ������ ��� �������


    //  ������ �� ������
    //  ����������: RCSOCK_OK, RCSOCK_ERROR
    int ReadSocketIPQt (void* pReadBuffer,           //  ����� ������
                        UINT* pLenRead,              //  ����� ��������, ��
					    quint64 nSocketID = SOCKETCLIENT_SINGLE);  //  ������ ����������� ������ ��� �������

    //  ������ � �����
    //  ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WriteSocketIPQt(void* pWriteBuffer,          //  ����� ������
                        UINT* pLenWrite,             //  ����� ������������/������� ���������� ����
                        DWORD dwTimeOut,             //  ����� ��������, ��
					    quint64 nSocketID = SOCKETCLIENT_SINGLE);  //  ������ ����������� ������ ��� �������

    //  ��������� ������� ����������� ����������� (��� ������� ��� ������������� �����������)
    // ���������� RCSOCK_OK, RCSOCK_ERROR
    int SetStatusConnectionServerIPQt (quint64 nSocketID);

    QTcpSocket* pSocketSelf;          //  ���� �����
    QTcpServer* pServer;              //  ������

    private:

    QHostAddress* pHostAddress;       //  IP-����� ����
    QHostAddress* pHostAddressServer; //  IP-����� �������
    BYTE bySockType;                  //  ��� ������
    WORD wPortSelf;                   //  �����
    WORD wPortAbon;
    BOOL bFlagCreateSocket;           //  ������� �������� ������
    qint64 uMaxMessageSize;             //  ������������ ������ ���������
    QString* pString;
	QTcpSocket* pSocketClient[SOCKETCLIENT_ID_MAX];        //  ������ ��� ��������
	int nConnectedClientCount;        // ���������� ������������ ��������
	BOOL bSingleFlag;                 // ���� ���������� ����������� � �������, TRUE ��� ���������� �����������
   };

//////////////////////////////////////////////////////////////////////


//////////////////// ����� ������ ��� ������ IP //////////////////////

class SockIPQtSlots : public QObject
   {
    Q_OBJECT

    public:
    // �����������
    SockIPQtSlots (qint64 n64NomThisDZ,
		           qint64 n64NomHandlingDZ,
				   quint64 nSockID = SOCKETCLIENT_SINGLE);  //  ������ ����������� ������ ��� ������������� �����������
	// ����������
    ~SockIPQtSlots ();

	public slots:

     void ReadReady();
     void Disconnect();
     void NewStatus(QAbstractSocket::SocketState nStatus);

    private:
	qint64 n64NomDZConnect;
	qint64 n64NomDZRead;
	quint64 nSocketID;           //  ������ ����������� ������ ��� ������� ��� ������������� �����������
   };

/////////////////////////////////////////////////////////////////////////


//////////////////////// ����� ������ UDP ///////////////////////////////

class SockUDPQt : public QObject
   {
    Q_OBJECT

    public :

    // �����������
    SockUDPQt ();

    // ����������
    ~SockUDPQt ();

    // �������� ������
    // ���������� RCSOCK_OK, RCSOCK_NOTCREATE
    int CreateSocket(WORD wSocketSelf,       //  ����� ������������ �����
                     WORD wAbonSocket,       //  ����� ����� ��������
					 QString* pSelfAddr,     //  IP-����� ����
					 QString* pAbonAddr,     //  IP-����� ��������
                     BYTE byType);           //  ��� ������ - ����� ��� ������

    //  ������ � �����
    //  ���������� RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WriteSocket(void* pWriteBuffer,      //  ����� ������
                    UINT* pLenWrite,         //  ����� ������������/������� ���������� ����
                    DWORD dwTimeOut);        //  ����� ��������, ��

    //  ������ �� ������
    //  ����������: RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int ReadSocket (void* pReadBuffer,       //  ����� ������
                    UINT* pLenRead,          //  ������ ������, ����/����� ����������� ����
                    DWORD dwTimeOut,         //  ����� ��������, ��
					BYTE  byControlSender=CONTROL_SENDER_UDP_OFF);        
	                                         //  ������� �������� ������ �����������

    //  ��������� �������� �������� ������
    //  ����������: TRUE - ����� ������, FALSE - ����� �� ������  
    BOOL GetFlagCreate ();

	//  ����������� ������ � ������
    //  ����������: RCSOCK_OK, RCSOCK_ERROR
	int ConnectMulticast(char* pMulticastAddr);    //  ����������������� IP-�����

    private:
      QUdpSocket* pSocket;              //  �����
      QHostAddress* pHostAddress;       //  IP-����� ����
      QHostAddress* pHostAddressAbon;   //  IP-����� ��������
      QHostAddress  HostAddressSender;  //  IP-����� �����������
      BYTE bySockType;                  //  ��� ������
      WORD wPortSelf;                   //  �����
      WORD wPortAbon;
      BOOL bFlagCreateSocket;           //  ������� �������� ������
      UINT uMaxMessageSize;             //  ������������ ������ ���������
      QString* pString;

   };

/////////////////////////////////////////////////////////////////////////////

#endif

