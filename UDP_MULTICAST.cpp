#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>



#include "UDP_MULTICAST.hpp"

#define INVALID_FD -1

/*Return Types*/
#define UNDEFINED -1
#define FAILURE    1
#define SUCCESS    0


UDPMulticast::UDPMulticast()
{
    m_ushSendingPortNo=0;
    m_ushReceivingPortNo=0;

    m_sSendingMulticastGroupAddress.clear();
    m_sMulticastGroupAddressToJoin.clear();

    m_bMulticastSenderInitialized=false;
    m_bMulticastReceiverInitialized=false;

    m_sender_fd     =   INVALID_FD;
    m_receiver_fd   =   INVALID_FD;

}

UDPMulticast::~UDPMulticast()
{
    /*closing opened fds if any*/
    if(m_sender_fd != (INVALID_FD))
    {
        close(m_sender_fd);
    }
    if(m_receiver_fd != (INVALID_FD))
    {
        close(m_receiver_fd);
    }

}

int UDPMulticast::IntializeMulticastSender(const u_short &ushSendingPortNo, const std::string &sSendingMulticastGroupAddress)
{
    int iReturn = UNDEFINED;
    try
    {
        /* create what looks like an ordinary UDP socket */
        if ((m_sender_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("socket");
            throw std::runtime_error("Failed to create sending socket");
        }
        /* set up destination address */
        memset(&m_send_addr,0,sizeof(m_send_addr));
        m_send_addr.sin_family=AF_INET;
        m_send_addr.sin_addr.s_addr=inet_addr(sSendingMulticastGroupAddress.c_str());
        m_send_addr.sin_port=htons(ushSendingPortNo);

        m_bMulticastSenderInitialized=true;

        iReturn = SUCCESS;
    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;

        m_bMulticastSenderInitialized=false;

        if(m_sender_fd != (INVALID_FD))
        {
            close(m_sender_fd);
        }
        iReturn = FAILURE;
    }

    return iReturn;

}

int UDPMulticast::InitilizeMulticastReceiver(const u_short& ushReceivingPortNo,const std::string& sMulticastGroupAddressToJoin)
{

    int iReturn = UNDEFINED;

    try
    {
        /* create what looks like an ordinary UDP socket */
        if ((m_receiver_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("socket");
            throw std::runtime_error("Failed to create receiving socket");
        }

        /* allow multiple sockets to use the same PORT number */
        u_int yes=1;
        if (setsockopt(m_receiver_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)
        {
            perror("Reusing ADDR failed");
            throw std::runtime_error("Failed to resue ADDR");
        }

        /* set up port */
        memset(&m_receive_addr,0,sizeof(m_receive_addr));
        m_receive_addr.sin_family=AF_INET;
        m_receive_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        m_receive_addr.sin_port=htons(ushReceivingPortNo);

        /* bind to receive address */
        if (bind(m_receiver_fd,(struct sockaddr *) &m_receive_addr,sizeof(m_receive_addr)) < 0)
        {
            perror("bind");
            throw std::runtime_error("Failed to bind receiving socket");
        }

        /* use setsockopt() to request that the kernel join a multicast group */
        m_mreq.imr_multiaddr.s_addr=inet_addr(sMulticastGroupAddressToJoin.c_str());
        m_mreq.imr_interface.s_addr=htonl(INADDR_ANY);

        if (setsockopt(m_receiver_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&m_mreq,sizeof(m_mreq)) < 0)
        {
            perror("setsockopt");
            throw std::runtime_error("Failed to set receiving socket");
        }

        m_bMulticastReceiverInitialized=true;

        iReturn = SUCCESS;

    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;

        m_bMulticastReceiverInitialized=false;

        if(m_receiver_fd != (INVALID_FD))
        {
            close(m_receiver_fd);
        }

        iReturn = FAILURE;
    }

    return iReturn;

}

int UDPMulticast::SendMulticastData(const char *cBuffer,const unsigned int &iMsgLength)
{
    int iReturn = UNDEFINED;
    try
    {
        if(m_bMulticastSenderInitialized)
        {

#ifdef DEBUG
            printf("\nSENDING UDP MULTICAST DATA ON MULTICAST GROUP %s AND PORT NO %d OF LENGTH %d \n",sSendingMulticastGroupAddress.c_str(),ushSendingPortNo,iMsgLength);

            printf("\n####################################################\n");
            for(unsigned int i=0; i<iMsgLength;++i)
            {
                printf("%x ",(unsigned char)cBuffer[i]);
            }
            printf("\n####################################################\n");
#endif

            /* now just sendto() our destination! */
            if(sendto(m_sender_fd,cBuffer,iMsgLength,0,(struct sockaddr *) &m_send_addr,sizeof(m_send_addr)) < 0)
            {
                perror("sendto");
                throw std::runtime_error("Failed to send");
            }
            iReturn = SUCCESS;
        }
        else
        {
            std::cerr<<"Initialize multicast sender first"<<std::endl;
            throw std::logic_error("Sender not Initialized");
        }
    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;
        iReturn = FAILURE;
    }
    return iReturn;


}

int UDPMulticast::ReceiveMulticastData(unsigned int &iMsgLength, char *cBuffer)
{
    int iReturn = UNDEFINED;
    try
    {
        if(m_bMulticastReceiverInitialized)
        {
            /* now just enter a read-print loop */
            unsigned int addrlen=sizeof(m_receive_addr);
            int iReceivedBytes = -1;

            if ((iReceivedBytes=recvfrom(m_receiver_fd,cBuffer,MAX_PACKET_SIZE,0,(struct sockaddr *) &m_receive_addr,&addrlen)) < 0)
            {
                perror("\nERROR IN RECEIVING\n");
                throw std::runtime_error("Failed to Receive");

            }
            iMsgLength=iReceivedBytes;

#ifdef DEBUG
            printf("\nRECEIVED UDP MULTICAST DATA ON MULTICAST GROUP %s AND PORT NO %d OF LENGTH %d \n",sMulticastGroupAddressToJoin.c_str(),ushReceivingPortNo,iMsgLength);

            printf("\n####################################################\n");

            for(unsigned int i=0; i<iMsgLength;++i)
            {
                printf("%x ",(unsigned char)cBuffer[i]);
            }
            printf("\n####################################################\n");
#endif

            iReturn = SUCCESS;
        }
        else
        {
            std::cerr<<"Initialize multicast receiver first"<<std::endl;
            throw std::logic_error("Receiver not Initialized");
        }
    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;
        iReturn = FAILURE;

    }
    return iReturn;

}


int UDPMulticast::SendUDPMulticast(const char *cBuffer,const unsigned int &iMsgLength, const u_short &ushSendingPortNo,const std::string &sSendingMulticastGroupAddress)
{
    struct sockaddr_in addr;
    int fd=INVALID_FD;

    int iReturn = UNDEFINED;

    try
    {
        /* create what looks like an ordinary UDP socket */
        if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("socket");
            throw std::runtime_error("Failed to set sending socket");
        }

        /* set up destination address */
        memset(&addr,0,sizeof(addr));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=inet_addr(sSendingMulticastGroupAddress.c_str());
        addr.sin_port=htons(ushSendingPortNo);

#ifdef DEBUG
        printf("\nSENDING UDP MULTICAST DATA ON MULTICAST GROUP %s AND PORT NO %d OF LENGTH %d \n",sSendingMulticastGroupAddress.c_str(),ushSendingPortNo,iMsgLength);

        printf("\n####################################################\n");
        for(unsigned int i=0; i<iMsgLength;++i)
        {
            printf("%x ",(unsigned char)cBuffer[i]);
        }
        printf("\n####################################################\n");
#endif

        /* now just sendto() our destination! */
        if (sendto(fd,cBuffer,iMsgLength,0,(struct sockaddr *) &addr,sizeof(addr)) < 0)
        {
            perror("sendto");
            throw std::runtime_error("Failed to send");
        }

        close(fd);

        iReturn = SUCCESS;
    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;
        iReturn = FAILURE;

        if(fd != (INVALID_FD))
        {
            close(fd);
        }
    }

    return iReturn;
}

int UDPMulticast::ReceiveUDPMulticast(unsigned int &iMsgLength, char* cBuffer, const u_short& ushReceivingPortNo,const std::string &sMulticastGroupAddressToJoin)
{

    struct sockaddr_in addr;
    struct ip_mreq mreq;
    int fd =INVALID_FD;
    int nbytes=0;

    int iReturn = UNDEFINED;

    unsigned int addrlen;

    try
    {
        /* create what looks like an ordinary UDP socket */
        if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("socket");
            throw std::runtime_error("Failed to Create receiving socket");
        }

        /* allow multiple sockets to use the same PORT number */
        u_int yes=1;
        if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)
        {
            perror("Reusing ADDR failed");
            throw std::runtime_error("Reusing ADDR failed");
        }

        /* set up destination address */
        memset(&addr,0,sizeof(addr));
        addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=htonl(INADDR_ANY);
        addr.sin_port=htons(ushReceivingPortNo);

        /* bind to receive address */
        if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0)
        {
            perror("bind");
            throw std::runtime_error("Failed to bind receiving socket");
        }

        /* use setsockopt() to request that the kernel join a multicast group */
        mreq.imr_multiaddr.s_addr=inet_addr(sMulticastGroupAddressToJoin.c_str());
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
        {
            perror("setsockopt");
            throw std::runtime_error("Failed to set receiving socket");
        }

        /* now just enter a read-print loop */
        addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,cBuffer,MAX_PACKET_SIZE,0,(struct sockaddr *) &addr,&addrlen)) < 0)
        {
            perror("\nERROR IN RECEIVING\n");
            throw std::runtime_error("Failed to Receive");
        }
        iMsgLength=nbytes;

#ifdef DEBUG
        printf("\nRECEIVED UDP MULTICAST DATA ON MULTICAST GROUP %s AND PORT NO %d OF LENGTH %d \n",sMulticastGroupAddressToJoin.c_str(),ushReceivingPortNo,iMsgLength);

        printf("\n####################################################\n");

        for(unsigned int i=0; i<iMsgLength;++i)
        {
            printf("%x ",(unsigned char)cBuffer[i]);
        }
        printf("\n####################################################\n");
#endif

        close(fd);
    }
    catch(std::exception &e)
    {
        std::cerr<<e.what()<<" Exception in function "<<__FUNCTION__<<std::endl;
        iReturn = FAILURE;

        if(fd != (INVALID_FD))
        {
            close(fd);
        }
    }
    return iReturn;



}
