/*
    EIBD client library
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License,
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any
    restriction coming from the use of this file. (The General Public
    License restrictions do apply in other respects; for example, they
    cover modification of the file, and distribution when not linked into
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef EIBTYPES_H
#define EIBTYPES_H

#define EIB_INVALID_REQUEST             0x0000
#define EIB_CONNECTION_INUSE            0x0001
#define EIB_PROCESSING_ERROR            0x0002
#define EIB_CLOSED                      0x0003
#define EIB_RESET_CONNECTION            0x0004

#define EIB_OPEN_BUSMONITOR             0x0010
#define EIB_OPEN_BUSMONITOR_TEXT        0x0011
#define EIB_OPEN_VBUSMONITOR            0x0012
#define EIB_OPEN_VBUSMONITOR_TEXT       0x0013
#define EIB_BUSMONITOR_PACKET           0x0014

#define EIB_OPEN_T_CONNECTION           0x0020
#define EIB_OPEN_T_INDIVIDUAL           0x0021
#define EIB_OPEN_T_GROUP                0x0022
#define EIB_OPEN_T_BROADCAST            0x0023
#define EIB_OPEN_T_TPDU                 0x0024
#define EIB_APDU_PACKET                 0x0025
#define EIB_OPEN_GROUPCON               0x0026
#define EIB_GROUP_PACKET                0x0027

#define EIB_PROG_MODE                   0x0030
#define EIB_MASK_VERSION                0x0031
#define EIB_M_INDIVIDUAL_ADDRESS_READ   0x0032

#define EIB_M_INDIVIDUAL_ADDRESS_WRITE  0x0040
#define EIB_ERROR_ADDR_EXISTS           0x0041
#define EIB_ERROR_MORE_DEVICE           0x0042
#define EIB_ERROR_TIMEOUT               0x0043
#define EIB_ERROR_VERIFY                0x0044

#define EIB_MC_INDIVIDUAL               0x0049
#define EIB_MC_CONNECTION               0x0050
#define EIB_MC_READ                     0x0051
#define EIB_MC_WRITE                    0x0052
#define EIB_MC_PROP_READ                0x0053
#define EIB_MC_PROP_WRITE               0x0054
#define EIB_MC_PEI_TYPE                 0x0055
#define EIB_MC_ADC_READ                 0x0056
#define EIB_MC_AUTHORIZE                0x0057
#define EIB_MC_KEY_WRITE                0x0058
#define EIB_MC_MASK_VERSION             0x0059
#define EIB_MC_RESTART                  0x005a
#define EIB_MC_WRITE_NOVERIFY           0x005b
#define EIB_MC_PROG_MODE                0x0060
#define EIB_MC_PROP_DESC                0x0061
#define EIB_MC_PROP_SCAN                0x0062
#define EIB_LOAD_IMAGE                  0x0063

#define EIB_CACHE_ENABLE                0x0070
#define EIB_CACHE_DISABLE               0x0071
#define EIB_CACHE_CLEAR                 0x0072
#define EIB_CACHE_REMOVE                0x0073
#define EIB_CACHE_READ                  0x0074
#define EIB_CACHE_READ_NOWAIT           0x0075
#define EIB_CACHE_LAST_UPDATES          0x0076

#define EIB_STATE_REQ_THREADS           0x0101
#define EIB_STATE_REQ_BACKENDS          0x0102
#define EIB_STATE_REQ_SERVERS           0x0103

/** XML constants delivered */

#define XMLEIBDNAMESPACE            "http://www.zeta2.ch/xml/eibd"

#define XMLSTATUSELEMENT            "status"      //< main status element on top
#define XMLSTATUSVERSIONATTR        "version"     //< version of eibd
#define XMLSTATUSSTARTTIMEATTR      "start-time"  //< start time of eibd, optional

#define XMLSTATDROPSATTR            "drops"    //< optional, drops occured
#define XMLSTATSENDERRATTR          "send-errors"   //<  send errors encountered
#define XMLSTATRECVERRATTR          "receive-errors"   //<  receive errors encountered
#define XMLSTATRESETSATTR           "resets"        //< element resets

#define XMLSTATUSUP                 "up"            //< constant string for different status fields
#define XMLSTATUSDOWN               "down"            //< constant string for different status fields
#define XMLSTATUSUNKNOWN            "unknown"            //< constant string for different status fields

/// @{ backend group, can contain any of the optional XMLSTAT... elements
#define XMLBACKENDELEMENT            "backend"  //< top element with backend
#define XMLBACKENDELEMENTTYPEATTR    "type"     //< type of backend
#define XMLBACKENDSTATUSATTR         "status"   //< status of backend (up, down, unknown)
#define XMLBACKENDADDRESSATTR        "address" //< backend's to address, optional
/// @}

/// @{ driver group, contained in backend, can contain any of the optional XMLSTAT... elements
#define XMLDRIVERELEMENT             "driver"   //< backend driver
#define XMLDRIVERELEMENTATTR         "type"     //< type of driver
#define XMLDRIVERSTATUSATTR          "status"   //< status of backend (up, down, unknown)
#define XMLDRIVERDEVICEATTR          "device"   //< optional device string
///

#define XMLQUEUESTATELEMENT          "queue"    //< queue element with all kinds of stats
#define XMLQUEUENAMEATTR             "name"     //< name of the queue
#define XMLQUEUEMAXLENATTR           "maximum-length"  //< length restrictions
#define XMLQUEUEMAXREACHLENATTR      "maximum-reached-length"   //< maximum reached length
#define XMLQUEUECURRENTLENATTR       "current-length" //< current length of queue
#define XMLQUEUEINSERTSATTR          "inserts" //< number of elements inserted
#define XMLQUEUEDROPSATTR            "drops" //< number of drops
#define XMLQUEUEMAXDELAYATTR         "maximum-delay" //< maximum stay of an element in queue
#define XMLQUEUEMEANDELAYATTR        "mean-delay" //< mean delay of an element in queue

#define XMLSERVERELEMENT             "server"  //< internal server
#define XMLSERVERTYPEATTR            "type"    //< type of server eibnet/local/ip, mandatory
#define XMLSERVERADDRESSATTR         "listen-address" //< type specific address/port in ASCII, optional
#define XMLSERVERMAXCLIENTSATTR      "max-reached-clients" //< max. clients on server concurrently
#define XMLSERVERMAXALLOWCLIENTSATTR "max-allowed-clients" //< max. clients on server concurrently allowed, optional
#define XMLSERVERCLIENTSATTR         "current-clients" //< number of clients currently served
#define XMLSERVERCLIENTSTOTALATTR    "total-clients" //< clients over lifetime
#define XMLSERVERCLIENTSREJECTEDATTR "clients-rejected" //< how many clients have been rejected, optional
#define XMLSERVERCLIENTSAUTHFAILATTR "clients-authentication-failed" //< how many clients failed authentication, optional
#define XMLSERVERMESSAGESRECVATTR    "packets-received-all-clients" //< how many packets received by all clients, optional
#define XMLSERVERMESSAGESSENTATTR    "packets-sent-all-clients" //< how many packets received by all clients, optional
#define XMLSERVERSENDERRATTR         "send-errors-all-clients"   //<  send errors encountered by server/all clients, optional
#define XMLSERVERRECVERRATTR         "receive-errors-all-clients"   //<  receive errors encountered by server/all clients, optionals
#define XMLSERVERXMLPROTODEFATTR     "client-protocol-namespace" //< optional attribute returning protocol namespace (xmlns)
#define XMLSERVERSUBNETFILTERS       "subnet-filters" //< optional attribute showing all the IP subnets

#define XMLCLIENTELEMENT             "client"  //< client element within server
#define XMLCLIENTTYPEATTR            "type"    //< mandatory
#define XMLCLIENTSTARTTIMEATTR       "start-time" //< when did it start
#define XMLCLIENTSMESSAGESRECVATTR   "packets-received" //< how many packets received, optional
#define XMLCLIENTSMESSAGESSENTATTR   "packets-sent" //< how many packets received, optional
#define XMLCLIENTSRECVERRATTR        "receive-errors" //< how many errors receiving, optional
#define XMLCLIENTSSENDERRATTR        "send-errors" //< how many errors sending, optional

#define XMLCLIENTADDRESSATTR         "address" //< client's from address, optional

#define XMLCLIENTSTATEADDR           "state" //< string definining state, optional

//@{{
#define EIBD_LOG_EMERG    "emerg"
#define EIBD_LOG_ALERT    "alert"
#define EIBD_LOG_CRIT     "crit"
#define EIBD_LOG_ERR      "error"
#define EIBD_LOG_WARN     "warn"
#define EIBD_LOG_NOTICE   "note"
#define EIBD_LOG_INFO     "info"
#define EIBD_LOG_DEBUG    "debug"

//@}}

#endif
