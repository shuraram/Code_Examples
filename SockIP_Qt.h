//////////////////////////////////////////////////////////////////////
//                                                                  //
//      SockIP_Qt.h - описание для классов сокетов IP и UDP         //
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


// типы сокета IP
#define REGIM_SERVER_SOCK 1
#define REGIM_CLIENT_SOCK 2

//  типы сокета UDP
#define REGIM_RECEIVE_SOCK   1
#define REGIM_SEND_SOCK      2

// размер буфера в сокете IP
const int SOCK_BUFF_SIZE=256000;

//  Время ожидания завершения чтения, мс
const DWORD TIME_OUT_READ_SOCK  = 100;
//  Время ожидания завершения записи, мс
const DWORD TIME_OUT_WRITE_SOCK = 100;

//  Коды возврата для функций класса сокета
const int RCSOCK_ERROR          = 100;  //  ошибка
const int RCSOCK_OK             =   0;  //  норма
const int RCSOCK_NOTCREATE      =   1;  //  сокет не создан
const int RCSOCK_TIMEOUT        =   2;  //  истекло время ожидания
const int RCSOCK_DISCONNECT     =   3;  //  разрыв соединения

// признаки чтения из сокета (для ДЗ чтения) 
const WORD READY_READ_SLOT         =  0xAAA1;       
const WORD DISCONNECT_SLOT         =  0xAAA2;       
const WORD READY_WRITE_SLOT        =  0xAAA3; 

// признак контроля адреса отправителя при чтении для сокета UDP
const BYTE CONTROL_SENDER_UDP_ON   =  0xFF;       
const BYTE CONTROL_SENDER_UDP_OFF  =  0x0;

//  максимальное число подключений к серверу  
const int SOCKETCLIENT_ID_MAX = 32;      
const int SOCKETCLIENT_SINGLE = 0xFF;      

//////////////////////// Класс сокета TCP\IP ////////////////////

class SockIPQt : public QObject
   {
    Q_OBJECT

    public:
    // конструктор
    SockIPQt (QString* pSelfAddr,     //  собственный IP-адрес
	          WORD wSelfPort,         //  номер собственного порта
              BYTE byType);           //  тип сокета - клиент или сервер

    // деструктор
    ~SockIPQt ();

    // создание сокета
    // возвращает RCSOCK_OK, RCSOCK_NOTCREATE
    int CreateSocketIPQt();
	
    // проверка установки соединения
    // возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int GetStatusConnectionIPQt ();

    // ожидание подключения
	// на входе в nSocketID признак SOCKETCLIENT_SINGLE для одиночного подключения
    // возвращает nSocketID - индекс клиентского сокета для сервера
    // возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WaitConnectSocketIPQt (DWORD dwTimeOut,
	                           QString* ServerAddressString = NULL, // IP-адрес сервера
							   WORD uServerPort = 0,                //  номер порта сервера
							   quint64* nSocketID = NULL);           //  индекс клиентского сокета для сервера


    //  Чтение из сокета
    //  Возвращает: RCSOCK_OK, RCSOCK_ERROR
    int ReadSocketIPQt (void* pReadBuffer,           //  буфер данных
                        UINT* pLenRead,              //  время ожидания, мс
					    quint64 nSocketID = SOCKETCLIENT_SINGLE);  //  индекс клиентского сокета для сервера

    //  Запись в сокет
    //  Возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WriteSocketIPQt(void* pWriteBuffer,          //  буфер данных
                        UINT* pLenWrite,             //  число записываемых/реально записанных байт
                        DWORD dwTimeOut,             //  время ожидания, мс
					    quint64 nSocketID = SOCKETCLIENT_SINGLE);  //  индекс клиентского сокета для сервера

    //  Установка статуса клиентского подключения (для сервера при множественном подключении)
    // возвращает RCSOCK_OK, RCSOCK_ERROR
    int SetStatusConnectionServerIPQt (quint64 nSocketID);

    QTcpSocket* pSocketSelf;          //  свой сокет
    QTcpServer* pServer;              //  сервер

    private:

    QHostAddress* pHostAddress;       //  IP-адрес свой
    QHostAddress* pHostAddressServer; //  IP-адрес сервера
    BYTE bySockType;                  //  тип сокета
    WORD wPortSelf;                   //  порты
    WORD wPortAbon;
    BOOL bFlagCreateSocket;           //  признак создания сокета
    qint64 uMaxMessageSize;             //  максимальный размер сообщения
    QString* pString;
	QTcpSocket* pSocketClient[SOCKETCLIENT_ID_MAX];        //  сокеты для клиентов
	int nConnectedClientCount;        // количество подключенных клиентов
	BOOL bSingleFlag;                 // флаг одиночного подключения к серверу, TRUE для одиночного подключения
   };

//////////////////////////////////////////////////////////////////////


//////////////////// Класс слотов для сокета IP //////////////////////

class SockIPQtSlots : public QObject
   {
    Q_OBJECT

    public:
    // конструктор
    SockIPQtSlots (qint64 n64NomThisDZ,
		           qint64 n64NomHandlingDZ,
				   quint64 nSockID = SOCKETCLIENT_SINGLE);  //  индекс клиентского сокета при множественном подключении
	// деструктор
    ~SockIPQtSlots ();

	public slots:

     void ReadReady();
     void Disconnect();
     void NewStatus(QAbstractSocket::SocketState nStatus);

    private:
	qint64 n64NomDZConnect;
	qint64 n64NomDZRead;
	quint64 nSocketID;           //  индекс клиентского сокета для сервера при множественном подключении
   };

/////////////////////////////////////////////////////////////////////////


//////////////////////// Класс сокета UDP ///////////////////////////////

class SockUDPQt : public QObject
   {
    Q_OBJECT

    public :

    // конструктор
    SockUDPQt ();

    // деструктор
    ~SockUDPQt ();

    // создание сокета
    // возвращает RCSOCK_OK, RCSOCK_NOTCREATE
    int CreateSocket(WORD wSocketSelf,       //  номер собственного порта
                     WORD wAbonSocket,       //  номер порта абонента
					 QString* pSelfAddr,     //  IP-адрес свой
					 QString* pAbonAddr,     //  IP-адрес абонента
                     BYTE byType);           //  тип сокета - прием или выдача

    //  Запись в сокет
    //  Возвращает RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int WriteSocket(void* pWriteBuffer,      //  буфер данных
                    UINT* pLenWrite,         //  число записываемых/реально записанных байт
                    DWORD dwTimeOut);        //  время ожидания, мс

    //  Чтение из сокета
    //  Возвращает: RCSOCK_OK, RCSOCK_ERROR, RCSOCK_TIMEOUT
    int ReadSocket (void* pReadBuffer,       //  буфер данных
                    UINT* pLenRead,          //  размер буфера, байт/число прочитанных байт
                    DWORD dwTimeOut,         //  время ожидания, мс
					BYTE  byControlSender=CONTROL_SENDER_UDP_OFF);        
	                                         //  признак проверки адреса отправителя

    //  Получение признака создания сокета
    //  Возвращает: TRUE - сокет создан, FALSE - сокет не создан  
    BOOL GetFlagCreate ();

	//  Подключение сокета к группе
    //  Возвращает: RCSOCK_OK, RCSOCK_ERROR
	int ConnectMulticast(char* pMulticastAddr);    //  широковещательный IP-адрес

    private:
      QUdpSocket* pSocket;              //  сокет
      QHostAddress* pHostAddress;       //  IP-адрес свой
      QHostAddress* pHostAddressAbon;   //  IP-адрес абонента
      QHostAddress  HostAddressSender;  //  IP-адрес отправителя
      BYTE bySockType;                  //  тип сокета
      WORD wPortSelf;                   //  порты
      WORD wPortAbon;
      BOOL bFlagCreateSocket;           //  признак создания сокета
      UINT uMaxMessageSize;             //  максимальный размер сообщения
      QString* pString;

   };

/////////////////////////////////////////////////////////////////////////////

#endif

