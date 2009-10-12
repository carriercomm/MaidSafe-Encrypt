/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Description:  Manages buffer packet messages to the maidsafe client
* Version:      1.0
* Created:      2009-01-28-23.10.42
* Revision:     none
* Compiler:     gcc
* Author:       Fraser Hutchison (fh), fraser.hutchison@maidsafe.net
* Company:      maidsafe.net limited
*
* The following source code is property of maidsafe.net limited and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of maidsafe.net.
*
* ============================================================================
*/

#ifndef MAIDSAFE_CLIENT_CLIENTBUFFERPACKETHANDLER_H_
#define MAIDSAFE_CLIENT_CLIENTBUFFERPACKETHANDLER_H_

#include <boost/thread/mutex.hpp>
#include <maidsafe/crypto.h>

#include <list>
#include <set>
#include <string>

#include "maidsafe/maidsafe.h"
#include "maidsafe/client/sessionsingleton.h"
#include "maidsafe/client/storemanager.h"
#include "protobuf/packet.pb.h"
#include "protobuf/datamaps.pb.h"

namespace maidsafe {

class ClientBufferPacketHandler {
 public:
  explicit ClientBufferPacketHandler(maidsafe::StoreManagerInterface *sm);
  int CreateBufferPacket(const std::string &owner_id,
                         const std::string &public_key,
                         const std::string &private_key);
  int ChangeStatus(const int &status, const PacketType &type);
  int AddUsers(const std::set<std::string> &users, const PacketType &type);
  int DeleteUsers(const std::set<std::string> &users, const PacketType &type);
  int GetMessages(const PacketType &type,
                  std::list<ValidatedBufferPacketMessage> *valid_messages);
  int GetBufferPacketInfo(const PacketType &type);
//  int ClearMessages(const PacketType &type);
 private:
  ClientBufferPacketHandler &operator=(const ClientBufferPacketHandler);
  ClientBufferPacketHandler(const ClientBufferPacketHandler&);
  bool UserList(const PacketType &type, std::set<std::string> *list);
  bool SetUserList(const PacketType &type, const std::set<std::string> &list);
  crypto::Crypto crypto_obj_;
  maidsafe::SessionSingleton *ss_;
  maidsafe::StoreManagerInterface *sm_;
};

}  // namespace maidsafe

#endif  // MAIDSAFE_CLIENT_CLIENTBUFFERPACKETHANDLER_H_
