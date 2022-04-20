/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 SIGNET Lab, Department of Information Engineering,
 * University of Padova
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BURST_SINK_H
#define BURST_SINK_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/seq-ts-size-frag-header.h"
#include <unordered_map>
#include <map>

namespace ns3 {

class Address;
class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup burstsink BurstSink
 *
 * This application was written to complement BurstyApplication.
 */

/**
 * \ingroup burstsink
 * 
 * \brief Receives and consume traffic generated from a BurstyApplication
 * to an IP address and port
 *
 * This application was based on PacketSink, and modified to accept traffic
 * generated by BurstyApplication.
 * The application assumes to be on top of a UDP socket, thus receiving UDP
 * datagrams rather than bytestreams.
 *
 * The sink tries to aggregate single packets (fragments) into a single packet
 * burst. To do so, it gathers information from SeqTsSizeFragHeader, which all
 * received packets should have.
 * It then makes the following assumptions:
 * - Being based on a UDP socket, packets might arrive out-of-order. Within a
 * burst, BurstSink will reorder the received packets.
 * - While receiving burst n, if a fragment from burst k<n is received, the
 * fragment is discarded
 * - While receiving burst n, if a fragment from burst k>n is received,
 * burst n is discarded and burst k will start being buffered.
 * - If all fragments from a burst are received, the burst is successfully
 * received.
 * 
 * Traces are sent when a fragment is received and when a whole burst is
 * successfully received.
 * 
 */
class BurstSink : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  BurstSink ();
  virtual ~BurstSink ();

  /**
   * \return the total bytes received in this sink app
   */
  uint64_t GetTotalRxBytes () const;

  /**
   * \return the total fragments received in this sink app
   */
  uint64_t GetTotalRxFragments () const;

  /**
   * \return the total bursts received in this sink app
   */
  uint64_t GetTotalRxBursts () const;

  /**
   * \return pointer to listening socket
   */
  Ptr<Socket> GetListeningSocket (void) const;

  /**
   * \return list of pointers to accepted sockets
   */
  std::list<Ptr<Socket>> GetAcceptedSockets (void) const;

  /**
   * TracedCallback signature for a reception with addresses and SeqTsSizeFragHeader
   *
   * \param f The fragment received (without the SeqTsSize header)
   * \param from From address
   * \param to Local address
   * \param header The SeqTsSize header
   */
  typedef void (*SeqTsSizeFragCallback) (Ptr<const Packet> f, const Address &from,
                                         const Address &to, const SeqTsSizeFragHeader &header);

protected:
  virtual void DoDispose (void);

private:
  // inherited from Application base class.
  virtual void StartApplication (void); // Called at time specified by Start
  virtual void StopApplication (void); // Called at time specified by Stop

  /**
   * \brief Handle a fragment received by the application
   * \param socket the receiving socket
   */
  void HandleRead (Ptr<Socket> socket);
  /**
   * \brief Handle an incoming connection
   * \param socket the incoming connection socket
   * \param from the address the connection is from
   */
  void HandleAccept (Ptr<Socket> socket, const Address &from);
  /**
   * \brief Handle a connection close
   * \param socket the connected socket
   */
  void HandlePeerClose (Ptr<Socket> socket);
  /**
   * \brief Handle a connection error
   * \param socket the connected socket
   */
  void HandlePeerError (Ptr<Socket> socket);

  /**
   * \brief Simple burst handler
   * Contains information regarding the current burst sequence number
   */
  struct BurstHandler
  {
    uint64_t m_currentBurstSeq{0}; //!< Current burst sequence number
    uint16_t m_fragmentsMerged{0}; //!< Number of ordered fragments received and merged for the current burst
    std::map<uint16_t, const Ptr<Packet>> m_unorderedFragments; //!< The fragments received out-of-order, still to be merged
    Ptr<Packet> m_burstBuffer{Create<Packet> (0)}; //!< The buffer containing the ordered received fragments
  };

  /**
   * \brief Fragment received: assemble byte stream to extract SeqTsSizeFragHeader
   * \param f received fragment
   * \param from from address
   * \param localAddress local address
   *
   * The method assembles a received byte stream and extracts SeqTsSizeFragHeader
   * instances from the stream to export in a trace source.
   */
  void FragmentReceived (BurstHandler &burstHandler, const Ptr<Packet> &f, const Address &from,
                         const Address &localAddress);

  /**
   * \brief Hashing for the Address class
   * Needed to make Address the key of a map.
   */
  struct AddressHash
  {
    /**
     * \brief operator ()
     * \param x the address of which calculate the hash
     * \return the hash of x
     *
     * Should this method go in address.h?
     *
     * It calculates the hash taking the uint32_t hash value of the ipv4 address.
     * It works only for InetSocketAddresses (Ipv4 version)
     */
    size_t
    operator() (const Address &x) const
    {
      NS_ABORT_IF (!InetSocketAddress::IsMatchingType (x));
      InetSocketAddress a = InetSocketAddress::ConvertFrom (x);
      return std::hash<uint32_t> () (a.GetIpv4 ().Get ());
    }
  };

  std::unordered_map<Address, BurstHandler, AddressHash> m_burstHandlerMap; //!< Map of BurstHandlers, assuming a user only has one data stream

  // In the case of TCP, each socket accept returns a new socket, so the
  // listening socket is stored separately from the accepted sockets
  Ptr<Socket> m_socket{0}; //!< Listening socket
  std::list<Ptr<Socket>> m_socketList; //!< the accepted sockets
  Address m_local; //!< Local address to bind to
  TypeId m_tid; //!< Protocol TypeId
  uint64_t m_totRxBursts{0}; //!< Total bursts received
  uint64_t m_totRxFragments{0}; //!< Total fragments received
  uint64_t m_totRxBytes{0}; //!< Total bytes received

  // Traced Callback
  /// Callback for tracing the fragment Rx events, includes source, destination addresses, and headers
  TracedCallback<Ptr<const Packet>, const Address &, const Address &, const SeqTsSizeFragHeader &>
      m_rxFragmentTrace;
  /// Callbacks for tracing the burst Rx events, includes source, destination addresses, and headers
  TracedCallback<Ptr<const Packet>, const Address &, const Address &, const SeqTsSizeFragHeader &>
      m_rxBurstTrace;
};

} // namespace ns3

#endif /* BURST_SINK_H */
