#ifndef UDP_MULTICAST_HPP
#define UDP_MULTICAST_HPP


#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


//INFO :: MULTICAST

/*
 *  Multicast is a kind of UDP traffic similar to BROADCAST, but only hosts that have explicitly requested to receive this kind of traffic will get it.
    This means that you have to JOIN a multicast group if you want to receive traffic that belongs to that group.
    IP addresses in the range 224.0.0.0 to 239.255.255.255 ( Class D addresses) belongs to multicast.
    No host can have this as IP address, but every machine can join a multicast address group.

    Multicast traffic is only UDP (not reliable).
    Multicast migth be 1 to many or 1 to none.
    Not all networks are multicast enabled (Some routers do not forward Multicast).

    Range Start Address     Range End Address           Description

    224.0.0.0               224.0.0.255                 Reserved for special “well-known” multicast addresses.
    224.0.1.0               238.255.255.255             Globally-scoped (Internet-wide) multicast addresses.
    239.0.0.0               239.255.255.255             Administratively-scoped (local) multicast addresses.
*/


/*MAX Buffer Size while receiving data*/
#define MAX_PACKET_SIZE 2048
#define DEBUG

/*Goal of this library is to provide a cleaner interface for sending and receiving UDP Multicast data.*/


class UDPMulticast
{

    ushort m_ushSendingPortNo;
    ushort m_ushReceivingPortNo;

    std::string m_sSendingMulticastGroupAddress;
    std::string m_sMulticastGroupAddressToJoin;

    bool m_bMulticastSenderInitialized;
    bool m_bMulticastReceiverInitialized;

    struct sockaddr_in m_send_addr;
    int m_sender_fd;

    struct sockaddr_in m_receive_addr;
    struct ip_mreq m_mreq;
    int m_receiver_fd;



public:

    UDPMulticast();
    ~UDPMulticast();

    /**
     * @brief IntializeMulticastSender          ::      This function shall be used to intialize a Multicast sender.
     * @param ushSendingPortNo                  ::      Port No to send Multicast data on.
     * @param sSendingMulticastGroupAddress     ::      Multicast Group IP address to send data on.
     * @return                                  ::      0 if SUCCESS, 1 otherwise
     */
    int IntializeMulticastSender(const u_short& ushSendingPortNo,const std::string& sSendingMulticastGroupAddress);

    /**
     * @brief InitilizeMulticastReceiver        ::      This function shall be used to intialize a Multicast receiver.
     * @param ushReceivingPortNo                ::      Port No to Receive Multicast data on
     * @param sMulticastGroupAddressToJoin      ::      Multicast Group address to join for data reception.
     * @return                                  ::      0 if SUCCESS, 1 otherwise
     */
    int InitilizeMulticastReceiver(const u_short& ushReceivingPortNo,const std::string& sMulticastGroupAddressToJoin);

    /**
     * @brief SendMulticastData                 ::      This function shall be used to send multicast data, once sender is initialized.
     * @param cBuffer                           ::      Pointer to the buffer containing data to be sent.
     * @param iMsgLength                        ::      Length of data to be sent from data Buffer.
     * @return                                  ::      0 if SUCCESS, 1 otherwise

     */
    int SendMulticastData(const char *cBuffer,const unsigned int &iMsgLength);

    /**
     * @brief ReceiveMulticastData              ::      This function shall be used to receive multicast data, once receiver is initialized.
     * @param iMsgLength                        ::      No of bytes received.
     * @param cBuffer                           ::      Pointer to the buffer in which data will be received.
     * @return                                  ::      0 if SUCCESS, 1 otherwise
     */
    int ReceiveMulticastData(unsigned int &iMsgLength, char* cBuffer);


    /*Following two functions are static, with these functions there is no need to explicitly create/initialize sockets, they take care of everything.*/

    /**
     * @brief SendUDPMulticast                  ::      This function can be used to send data buffer pointed by cBuffer of iMsgLength as UDP MULTICAST.
     * @param cBuffer                           ::      Pointer to the buffer containing data to be sent.
     * @param iMsgLength                        ::      Length of data to be sent from data Buffer.
     * @param ushSendingPortNo                  ::      Port No to send Multicast data on.
     * @param sSendingMulticastGroupAddress     ::      Multicast Group IP address to send data on.
     * @return                                  ::      0 if SUCCESS, 1 otherwise
     */
    static int SendUDPMulticast(const char *cBuffer,const unsigned int &iMsgLength, const u_short &ushSendingPortNo,const std::string &sSendingMulticastGroupAddress);


    /**
     * @brief ReceiveUDPMulticast               ::      This function can be used to receive UDP MULTICAST data of length iMsgLength in a buffer pointed by cBuffer.
     * @param iMsgLength                        ::      No of bytes received.
     * @param cBuffer                           ::      Pointer to the buffer in which data will be received.
     * @param ushReceivingPortNo                ::      Port No to Receive Multicast data on
     * @param sMulticastGroupAddressToJoin      ::      Multicast Group address to join for data reception.
     * @return                                  ::      0 if SUCCESS, 1 otherwise

     */
    static int ReceiveUDPMulticast(unsigned int &iMsgLength, char* cBuffer, const u_short& ushReceivingPortNo,const std::string &sMulticastGroupAddressToJoin);


};










#endif // UDP_MULTICAST_HPP
