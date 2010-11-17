/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Unit tests for Passport class
* Version:      1.0
* Created:      2010-10-19-23.59.27
* Revision:     none
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

#include <boost/lexical_cast.hpp>
#include <gtest/gtest.h>
#include <maidsafe/base/utils.h>

#include "maidsafe/passport/passport.h"

namespace maidsafe {

namespace passport {

namespace test {

const boost::uint16_t kRsaKeySize(4096);
const boost::uint8_t kMaxThreadCount(5);

class PassportTest : public testing::Test {
 public:
  PassportTest()
      : passport_(kRsaKeySize, kMaxThreadCount),
        kUsername_(base::RandomAlphaNumericString(15)),
        kPin_(boost::lexical_cast<std::string>(base::RandomUint32())),
        kPassword_(base::RandomAlphaNumericString(20)),
        kPlainTextMasterData_(base::RandomString(10000)),
        mid_name_(),
        smid_name_() {}
 protected:
  typedef std::tr1::shared_ptr<pki::Packet> PacketPtr;
  typedef std::tr1::shared_ptr<MidPacket> MidPtr;
  typedef std::tr1::shared_ptr<TmidPacket> TmidPtr;
  typedef std::tr1::shared_ptr<SignaturePacket> SignaturePtr;
  void SetUp() {
    passport_.Init();
  }
  void TearDown() {}
  bool CreateUser(MidPtr mid, MidPtr smid, TmidPtr tmid) {
    if (!mid || !smid || !tmid)
      return false;
    SignaturePtr sig_packet(new SignaturePacket);
    bool result =
        passport_.InitialiseSignaturePacket(ANMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANSMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess &&
        passport_.InitialiseSignaturePacket(ANTMID, sig_packet) == kSuccess &&
        passport_.ConfirmSignaturePacket(sig_packet) == kSuccess;
    if (!result)
      return false;
    if (passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_, &smid_name_)
        != kSuccess)
      return false;
    if (passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid, smid,
                                 tmid) != kSuccess)
      return false;
    if (passport_.ConfirmNewUserData(mid, smid, tmid) != kSuccess)
      return false;
    return passport_.GetPacket(MID, true) && passport_.GetPacket(SMID, true) &&
           passport_.GetPacket(TMID, true);
  }
  Passport passport_;
  const std::string kUsername_, kPin_, kPassword_, kPlainTextMasterData_;
  std::string mid_name_, smid_name_;
};

TEST_F(PassportTest, BEH_PASSPORT_SignaturePacketFunctions) {
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseSignaturePacket(ANMID, SignaturePtr()));

  SignaturePtr signature_packet(new SignaturePacket);
  EXPECT_EQ(kPassportError,
            passport_.InitialiseSignaturePacket(MID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));

  EXPECT_EQ(kNoSigningPacket,
            passport_.InitialiseSignaturePacket(MAID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MAID, false));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  SignaturePtr anmaid1(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid1));
  EXPECT_FALSE(anmaid1->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid1.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));

  SignaturePtr anmaid2(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid2));
  EXPECT_FALSE(anmaid2->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid2.get()));
  EXPECT_FALSE(anmaid1->Equals(anmaid2.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));

  EXPECT_EQ(kNoSigningPacket,
            passport_.InitialiseSignaturePacket(MAID, signature_packet));
  EXPECT_TRUE(signature_packet->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));
  EXPECT_FALSE(passport_.GetPacket(MAID, false));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  EXPECT_EQ(kPassportError, passport_.ConfirmSignaturePacket(SignaturePtr()));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));

  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmSignaturePacket(anmaid1));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));

  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmaid2));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  SignaturePtr anmaid3(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_FALSE(anmaid3->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false)->Equals(anmaid3.get()));
  EXPECT_FALSE(anmaid2->Equals(anmaid3.get()));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  EXPECT_TRUE(passport_.SignaturePacketName(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MAID, false).empty());
  EXPECT_TRUE(passport_.SignaturePacketName(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKey(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPrivateKey(MAID, true).empty());
  EXPECT_TRUE(passport_.SignaturePacketPublicKeySignature(MAID, true).empty());
  EXPECT_EQ(anmaid3->name(), passport_.SignaturePacketName(ANMAID, false));
  EXPECT_EQ(anmaid3->value(),
            passport_.SignaturePacketPublicKey(ANMAID, false));
  EXPECT_EQ(anmaid3->private_key(),
            passport_.SignaturePacketPrivateKey(ANMAID, false));
  EXPECT_EQ(anmaid3->public_key_signature(),
            passport_.SignaturePacketPublicKeySignature(ANMAID, false));
  EXPECT_EQ(anmaid2->name(), passport_.SignaturePacketName(ANMAID, true));
  EXPECT_EQ(anmaid2->value(), passport_.SignaturePacketPublicKey(ANMAID, true));
  EXPECT_EQ(anmaid2->private_key(),
            passport_.SignaturePacketPrivateKey(ANMAID, true));
  EXPECT_EQ(anmaid2->public_key_signature(),
            passport_.SignaturePacketPublicKeySignature(ANMAID, true));

  EXPECT_EQ(kPassportError, passport_.RevertSignaturePacket(MID));
  EXPECT_EQ(kPassportError, passport_.RevertSignaturePacket(MAID));
  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(ANMAID));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));

  SignaturePtr maid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  std::string original_maid_name(maid->name());
  EXPECT_FALSE(original_maid_name.empty());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));
  EXPECT_TRUE(passport_.GetPacket(MAID, false));
  EXPECT_TRUE(passport_.GetPacket(MAID, false)->Equals(maid.get()));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(MAID));
  EXPECT_FALSE(passport_.GetPacket(MAID, false));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_FALSE(maid->name().empty());
  EXPECT_NE(original_maid_name, maid->name());
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true)->Equals(anmaid2.get()));
  EXPECT_TRUE(passport_.GetPacket(MAID, false));
  EXPECT_TRUE(passport_.GetPacket(MAID, false)->Equals(maid.get()));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  EXPECT_EQ(kNoPacket, passport_.DeletePacket(MID));
  EXPECT_EQ(kSuccess, passport_.DeletePacket(MAID));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_FALSE(passport_.GetPacket(MAID, false));
  EXPECT_FALSE(passport_.GetPacket(MAID, true));

  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(MAID, false));

  passport_.Clear();
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));
  EXPECT_FALSE(passport_.GetPacket(MAID, false));

  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmaid3));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMAID, anmaid3));
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(MAID, maid));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMAID, true));
  EXPECT_TRUE(passport_.GetPacket(MAID, false));

  passport_.ClearKeyring();
  EXPECT_FALSE(passport_.GetPacket(ANMAID, false));
  EXPECT_FALSE(passport_.GetPacket(ANMAID, true));
  EXPECT_FALSE(passport_.GetPacket(MAID, false));
}

TEST_F(PassportTest, BEH_PASSPORT_MpidFunctions) {
  const std::string kPublicName(base::RandomAlphaNumericString(10));
  EXPECT_EQ(kNullPointer,
            passport_.InitialiseMpid(kPublicName, SignaturePtr()));

  SignaturePtr mpid(new SignaturePacket);
  EXPECT_EQ(kNoSigningPacket, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_TRUE(mpid->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  SignaturePtr anmpid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseSignaturePacket(ANMPID, anmpid));
  EXPECT_FALSE(anmpid->name().empty());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, false)->Equals(anmpid.get()));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, true));

  EXPECT_EQ(kNoSigningPacket, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_TRUE(mpid->name().empty());
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(anmpid));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));

  EXPECT_EQ(kPassportError, passport_.InitialiseSignaturePacket(MPID, mpid));
  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  std::string original_mpid_name(mpid->name());
  EXPECT_FALSE(original_mpid_name.empty());
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  EXPECT_TRUE(passport_.GetPacket(MPID, false));
  EXPECT_TRUE(passport_.GetPacket(MPID, false)->Equals(mpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.RevertSignaturePacket(MPID));
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_FALSE(mpid->name().empty());
  EXPECT_EQ(original_mpid_name, mpid->name());
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  EXPECT_TRUE(passport_.GetPacket(MPID, false));
  EXPECT_TRUE(passport_.GetPacket(MPID, false)->Equals(mpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.DeletePacket(MPID));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  SignaturePtr other_mpid(new SignaturePacket);
  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName + "a", other_mpid));
  EXPECT_FALSE(other_mpid->name().empty());
  EXPECT_NE(original_mpid_name, other_mpid->name());
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmSignaturePacket(mpid));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(MPID, false));
  EXPECT_FALSE(passport_.GetPacket(MPID, true));

  EXPECT_EQ(kSuccess, passport_.InitialiseMpid(kPublicName, mpid));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(mpid));
  EXPECT_FALSE(passport_.GetPacket(ANMPID, false));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true));
  EXPECT_TRUE(passport_.GetPacket(ANMPID, true)->Equals(anmpid.get()));
  EXPECT_FALSE(passport_.GetPacket(MPID, false));
  EXPECT_TRUE(passport_.GetPacket(MPID, true));
  EXPECT_TRUE(passport_.GetPacket(MPID, true)->Equals(mpid.get()));
  EXPECT_EQ(original_mpid_name, mpid->name());
}

TEST_F(PassportTest, BEH_PASSPORT_SetInitialDetails) {
  // Invalid data and null pointers
  std::string invalid_pin("Non-numerical");
  mid_name_ = smid_name_ = "a";
  EXPECT_EQ(kNullPointer, passport_.SetInitialDetails(kUsername_, kPin_, NULL,
                                                      &smid_name_));
  EXPECT_EQ(kNullPointer, passport_.SetInitialDetails(kUsername_, kPin_,
                                                      &mid_name_, NULL));

  EXPECT_EQ(kPassportError,
            passport_.SetInitialDetails(kUsername_, invalid_pin, &mid_name_,
                                        &smid_name_));
  EXPECT_TRUE(mid_name_.empty());
  EXPECT_TRUE(smid_name_.empty());

  mid_name_ = smid_name_ = "a";
  EXPECT_EQ(kPassportError, passport_.SetInitialDetails("", kPin_, &mid_name_,
                                                        &smid_name_));
  EXPECT_TRUE(mid_name_.empty());
  EXPECT_TRUE(smid_name_.empty());

  // Good initial data
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_FALSE(mid_name_.empty());
  EXPECT_FALSE(smid_name_.empty());
  EXPECT_NE(mid_name_, smid_name_);
  PacketPtr pending_mid(passport_.GetPacket(MID, false));
  PacketPtr pending_smid(passport_.GetPacket(SMID, false));
  PacketPtr confirmed_mid(passport_.GetPacket(MID, true));
  PacketPtr confirmed_smid(passport_.GetPacket(SMID, true));
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_EQ(mid_name_, pending_mid->name());
  EXPECT_EQ(smid_name_, pending_smid->name());

  // Different username should generate different mid and smid
  std::string different_username(kUsername_ + "a");
  std::string different_username_mid_name, different_username_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(different_username, kPin_,
                                        &different_username_mid_name,
                                        &different_username_smid_name));
  EXPECT_FALSE(different_username_mid_name.empty());
  EXPECT_FALSE(different_username_smid_name.empty());
  EXPECT_NE(different_username_mid_name, different_username_smid_name);
  EXPECT_NE(mid_name_, different_username_mid_name);
  EXPECT_NE(smid_name_, different_username_mid_name);
  EXPECT_NE(mid_name_, different_username_smid_name);
  EXPECT_NE(smid_name_, different_username_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_EQ(different_username_mid_name, pending_mid->name());
  EXPECT_EQ(different_username_smid_name, pending_smid->name());

  // Different pin should generate different mid and smid
  std::string different_pin(boost::lexical_cast<std::string>(
                            boost::lexical_cast<boost::uint32_t>(kPin_) + 1));
  std::string different_pin_mid_name, different_pin_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(kUsername_, different_pin,
                                        &different_pin_mid_name,
                                        &different_pin_smid_name));
  EXPECT_FALSE(different_pin_mid_name.empty());
  EXPECT_FALSE(different_pin_smid_name.empty());
  EXPECT_NE(different_pin_mid_name, different_pin_smid_name);
  EXPECT_NE(mid_name_, different_pin_mid_name);
  EXPECT_NE(smid_name_, different_pin_mid_name);
  EXPECT_NE(mid_name_, different_pin_smid_name);
  EXPECT_NE(smid_name_, different_pin_smid_name);
  EXPECT_NE(different_username_mid_name, different_pin_mid_name);
  EXPECT_NE(different_username_smid_name, different_pin_mid_name);
  EXPECT_NE(different_username_mid_name, different_pin_smid_name);
  EXPECT_NE(different_username_smid_name, different_pin_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_EQ(different_pin_mid_name, pending_mid->name());
  EXPECT_EQ(different_pin_smid_name, pending_smid->name());

  // Different username & pin should generate different mid and smid
  std::string different_both_mid_name, different_both_smid_name;
  EXPECT_EQ(kSuccess,
            passport_.SetInitialDetails(different_username, different_pin,
                                        &different_both_mid_name,
                                        &different_both_smid_name));
  EXPECT_FALSE(different_both_mid_name.empty());
  EXPECT_FALSE(different_both_smid_name.empty());
  EXPECT_NE(different_both_mid_name, different_both_smid_name);
  EXPECT_NE(mid_name_, different_both_mid_name);
  EXPECT_NE(smid_name_, different_both_mid_name);
  EXPECT_NE(mid_name_, different_both_smid_name);
  EXPECT_NE(smid_name_, different_both_smid_name);
  EXPECT_NE(different_username_mid_name, different_both_mid_name);
  EXPECT_NE(different_username_smid_name, different_both_mid_name);
  EXPECT_NE(different_username_mid_name, different_both_smid_name);
  EXPECT_NE(different_username_smid_name, different_both_smid_name);
  EXPECT_NE(different_pin_mid_name, different_both_mid_name);
  EXPECT_NE(different_pin_smid_name, different_both_mid_name);
  EXPECT_NE(different_pin_mid_name, different_both_smid_name);
  EXPECT_NE(different_pin_smid_name, different_both_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_EQ(different_both_mid_name, pending_mid->name());
  EXPECT_EQ(different_both_smid_name, pending_smid->name());

  // Original username & pin should generate original mid and smid
  std::string original_mid_name, original_smid_name;
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_,
                                                  &original_mid_name,
                                                  &original_smid_name));
  EXPECT_EQ(mid_name_, original_mid_name);
  EXPECT_EQ(smid_name_, original_smid_name);
  pending_mid = passport_.GetPacket(MID, false);
  pending_smid = passport_.GetPacket(SMID, false);
  confirmed_mid = passport_.GetPacket(MID, true);
  confirmed_smid = passport_.GetPacket(SMID, true);
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_EQ(mid_name_, pending_mid->name());
  EXPECT_EQ(smid_name_, pending_smid->name());
}

TEST_F(PassportTest, BEH_PASSPORT_SetNewUserData) {
  // Invalid data and null pointers
  MidPtr null_mid, mid(new MidPacket), null_smid, smid(new MidPacket);
  TmidPtr null_tmid, tmid(new TmidPacket);

  EXPECT_EQ(kNoMid,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());

  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kSuccess, passport_.packet_handler_.DeletePacket(SMID));
  EXPECT_EQ(kNoSmid,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());

  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     null_mid, smid, tmid));
  EXPECT_TRUE(smid->name().empty());
  EXPECT_TRUE(tmid->name().empty());
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     mid, null_smid, tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(tmid->name().empty());
  EXPECT_EQ(kNullPointer,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     mid, smid, null_tmid));
  EXPECT_TRUE(mid->name().empty());
  EXPECT_TRUE(smid->name().empty());

  // Good initial data
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  MidPtr pending_mid(std::tr1::static_pointer_cast<MidPacket>(
                     passport_.GetPacket(MID, false)));
  MidPtr pending_smid(std::tr1::static_pointer_cast<MidPacket>(
                      passport_.GetPacket(SMID, false)));
  TmidPtr pending_tmid(std::tr1::static_pointer_cast<TmidPacket>(
                       passport_.GetPacket(TMID, false)));
  MidPtr confirmed_mid(std::tr1::static_pointer_cast<MidPacket>(
                       passport_.GetPacket(MID, true)));
  MidPtr confirmed_smid(std::tr1::static_pointer_cast<MidPacket>(
                        passport_.GetPacket(SMID, true)));
  TmidPtr confirmed_tmid(std::tr1::static_pointer_cast<TmidPacket>(
                         passport_.GetPacket(TMID, true)));
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_TRUE(pending_tmid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_FALSE(confirmed_tmid);
  std::string mid_name(pending_mid->name()), smid_name(pending_smid->name());
  std::string tmid_name(pending_tmid->name());
  EXPECT_FALSE(mid_name.empty());
  EXPECT_FALSE(smid_name.empty());
  EXPECT_FALSE(tmid_name.empty());
  EXPECT_TRUE(pending_mid->Equals(mid.get()));
  EXPECT_TRUE(pending_smid->Equals(smid.get()));
  EXPECT_TRUE(pending_tmid->Equals(tmid.get()));
  EXPECT_EQ(kUsername_, pending_mid->username());
  EXPECT_EQ(kUsername_, pending_smid->username());
  EXPECT_EQ(kUsername_, pending_tmid->username());
  EXPECT_EQ(kPin_, pending_mid->pin());
  EXPECT_EQ(kPin_, pending_smid->pin());
  EXPECT_EQ(kPin_, pending_tmid->pin());
  boost::uint32_t rid(pending_mid->rid());
  EXPECT_NE(0U, rid);
  EXPECT_EQ(rid, pending_smid->rid());
  EXPECT_EQ(kPassword_, pending_tmid->password());
  // Check *copies* of pointers are returned
  EXPECT_EQ(1UL, mid.use_count());
  EXPECT_EQ(1UL, smid.use_count());
  EXPECT_EQ(1UL, tmid.use_count());

  // Check retry with same data generates new rid and hence new tmid name
  MidPtr retry_mid(new MidPacket), retry_smid(new MidPacket);
  TmidPtr retry_tmid(new TmidPacket);
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_,
                                     retry_mid, retry_smid, retry_tmid));
  pending_mid = std::tr1::static_pointer_cast<MidPacket>(
                passport_.GetPacket(MID, false));
  pending_smid = std::tr1::static_pointer_cast<MidPacket>(
                 passport_.GetPacket(SMID, false));
  pending_tmid = std::tr1::static_pointer_cast<TmidPacket>(
                 passport_.GetPacket(TMID, false));
  confirmed_mid = std::tr1::static_pointer_cast<MidPacket>(
                  passport_.GetPacket(MID, true));
  confirmed_smid = std::tr1::static_pointer_cast<MidPacket>(
                   passport_.GetPacket(SMID, true));
  confirmed_tmid = std::tr1::static_pointer_cast<TmidPacket>(
                   passport_.GetPacket(TMID, true));
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_TRUE(pending_tmid);
  EXPECT_FALSE(confirmed_mid);
  EXPECT_FALSE(confirmed_smid);
  EXPECT_FALSE(confirmed_tmid);
  EXPECT_EQ(mid_name, pending_mid->name());
  EXPECT_EQ(smid_name, pending_smid->name());
  EXPECT_NE(tmid_name, pending_tmid->name());
  EXPECT_FALSE(pending_tmid->name().empty());
  EXPECT_TRUE(pending_mid->Equals(retry_mid.get()));
  EXPECT_TRUE(pending_smid->Equals(retry_smid.get()));
  EXPECT_TRUE(pending_tmid->Equals(retry_tmid.get()));
  EXPECT_FALSE(pending_mid->Equals(mid.get()));
  EXPECT_FALSE(pending_smid->Equals(smid.get()));
  EXPECT_FALSE(pending_tmid->Equals(tmid.get()));
  EXPECT_EQ(kUsername_, pending_mid->username());
  EXPECT_EQ(kUsername_, pending_smid->username());
  EXPECT_EQ(kUsername_, pending_tmid->username());
  EXPECT_EQ(kPin_, pending_mid->pin());
  EXPECT_EQ(kPin_, pending_smid->pin());
  EXPECT_EQ(kPin_, pending_tmid->pin());
  EXPECT_NE(0U, pending_mid->rid());
  EXPECT_NE(rid, pending_mid->rid());
  EXPECT_EQ(pending_mid->rid(), pending_smid->rid());
  EXPECT_EQ(kPassword_, pending_tmid->password());
}

TEST_F(PassportTest, BEH_PASSPORT_ConfirmNewUserData) {
  MidPtr null_mid, different_username_mid(new MidPacket);
  MidPtr null_smid, different_username_smid(new MidPacket);
  TmidPtr null_tmid, different_username_tmid(new TmidPacket);
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails("Different", kPin_,
                                                  &mid_name_, &smid_name_));
  EXPECT_EQ(kSuccess, passport_.SetNewUserData(kPassword_,
                      kPlainTextMasterData_, different_username_mid,
                      different_username_smid, different_username_tmid));
  MidPtr mid(new MidPacket), smid(new MidPacket);
  TmidPtr tmid(new TmidPacket);
  EXPECT_EQ(kSuccess, passport_.SetInitialDetails(kUsername_, kPin_, &mid_name_,
                                                  &smid_name_));
  EXPECT_EQ(kSuccess,
            passport_.SetNewUserData(kPassword_, kPlainTextMasterData_, mid,
                                     smid, tmid));
  MidPtr pending_mid(std::tr1::static_pointer_cast<MidPacket>(
                     passport_.GetPacket(MID, false)));
  MidPtr pending_smid(std::tr1::static_pointer_cast<MidPacket>(
                      passport_.GetPacket(SMID, false)));
  TmidPtr pending_tmid(std::tr1::static_pointer_cast<TmidPacket>(
                       passport_.GetPacket(TMID, false)));
  EXPECT_TRUE(pending_mid);
  EXPECT_TRUE(pending_smid);
  EXPECT_TRUE(pending_tmid);
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(null_mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(mid, null_smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kNullPointer, passport_.ConfirmNewUserData(mid, smid, null_tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kMissingDependentPackets,
            passport_.ConfirmNewUserData(mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  SignaturePtr signature_packet(new SignaturePacket);
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANSMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));
  EXPECT_EQ(kSuccess,
            passport_.InitialiseSignaturePacket(ANTMID, signature_packet));
  EXPECT_EQ(kSuccess, passport_.ConfirmSignaturePacket(signature_packet));

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(different_username_mid, smid, tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(mid, different_username_smid, tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(MID, true));
  EXPECT_FALSE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kPacketsNotEqual,
            passport_.ConfirmNewUserData(mid, smid, different_username_tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_FALSE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(MID, true));
  EXPECT_TRUE(passport_.GetPacket(SMID, true));
  EXPECT_FALSE(passport_.GetPacket(TMID, true));

  EXPECT_EQ(kSuccess, passport_.ConfirmNewUserData(mid, smid, tmid));
  MidPtr confirmed_mid(std::tr1::static_pointer_cast<MidPacket>(
                       passport_.GetPacket(MID, true)));
  MidPtr confirmed_smid(std::tr1::static_pointer_cast<MidPacket>(
                        passport_.GetPacket(SMID, true)));
  TmidPtr confirmed_tmid(std::tr1::static_pointer_cast<TmidPacket>(
                         passport_.GetPacket(TMID, true)));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_FALSE(passport_.GetPacket(SMID, false));
  EXPECT_FALSE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(confirmed_mid);
  EXPECT_TRUE(confirmed_smid);
  EXPECT_TRUE(confirmed_tmid);

  EXPECT_TRUE(mid->Equals(pending_mid.get()));
  EXPECT_TRUE(smid->Equals(pending_smid.get()));
  EXPECT_TRUE(tmid->Equals(pending_tmid.get()));
  EXPECT_TRUE(mid->Equals(confirmed_mid.get()));
  EXPECT_TRUE(smid->Equals(confirmed_smid.get()));
  EXPECT_TRUE(tmid->Equals(confirmed_tmid.get()));

  EXPECT_EQ(kSuccess, passport_.ConfirmNewUserData(mid, smid, tmid));
}

TEST_F(PassportTest, BEH_PASSPORT_UpdateMasterData) {
  // Setup
  MidPtr original_mid(new MidPacket), original_smid(new MidPacket);
  TmidPtr original_tmid(new TmidPacket);
  MidPtr null_mid, different_mid(new MidPacket(kUsername_ + "a", kPin_, ""));
  MidPtr different_smid(new MidPacket(kUsername_ + "a", kPin_, "1"));
  TmidPtr null_tmid, different_tmid(new TmidPacket(kUsername_ + "a", kPin_, 99,
                                                   false, kPassword_,
                                                   kPlainTextMasterData_));
  std::string updated_master_data1(base::RandomString(10000));
  std::string mid_old_value, smid_old_value;
  MidPtr updated_mid1(new MidPacket), updated_smid1(new MidPacket);
  TmidPtr new_tmid1(new TmidPacket), tmid_for_deletion1(new TmidPacket);

  // Invalid data and null pointers
  ASSERT_TRUE(CreateUser(original_mid, original_smid, original_tmid));
  ASSERT_EQ(kSuccess, passport_.DeletePacket(TMID));
  EXPECT_EQ(kNoTmid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  ASSERT_EQ(kSuccess, passport_.DeletePacket(SMID));
  EXPECT_EQ(kNoSmid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  passport_.Clear();
  EXPECT_EQ(kNoMid, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));

  ASSERT_TRUE(CreateUser(original_mid, original_smid, original_tmid));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1, NULL,
            &smid_old_value, updated_mid1, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, NULL, updated_mid1, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, null_mid, updated_smid1, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, null_mid, new_tmid1,
            tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            null_tmid, tmid_for_deletion1));
  EXPECT_EQ(kNullPointer, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, null_tmid));
  ASSERT_TRUE(passport_.GetPacket(MID, true));
  ASSERT_TRUE(passport_.GetPacket(SMID, true));
  ASSERT_TRUE(passport_.GetPacket(TMID, true));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(original_mid.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(original_smid.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(original_tmid.get()));

  // Good initial data
  *tmid_for_deletion1 = *original_tmid;
  EXPECT_FALSE(tmid_for_deletion1->name().empty());
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(updated_master_data1,
            &mid_old_value, &smid_old_value, updated_mid1, updated_smid1,
            new_tmid1, tmid_for_deletion1));
  EXPECT_EQ(original_mid->value(), mid_old_value);
  EXPECT_EQ(original_smid->value(), smid_old_value);
  ASSERT_TRUE(updated_mid1);
  ASSERT_TRUE(updated_smid1);
  ASSERT_TRUE(new_tmid1);
  ASSERT_TRUE(tmid_for_deletion1);
  EXPECT_EQ(1UL, updated_mid1.use_count());
  EXPECT_EQ(1UL, updated_smid1.use_count());
  EXPECT_EQ(1UL, new_tmid1.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion1.use_count());
  EXPECT_TRUE(tmid_for_deletion1->name().empty());
  ASSERT_TRUE(passport_.GetPacket(MID, false));
  ASSERT_TRUE(passport_.GetPacket(SMID, false));
  ASSERT_TRUE(passport_.GetPacket(TMID, false));
  ASSERT_TRUE(passport_.GetPacket(STMID, false));
  EXPECT_TRUE(passport_.GetPacket(MID, false)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, false)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, false)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, false)->value(), original_tmid->value());
  ASSERT_TRUE(passport_.GetPacket(MID, true));
  ASSERT_TRUE(passport_.GetPacket(SMID, true));
  ASSERT_TRUE(passport_.GetPacket(TMID, true));
  EXPECT_FALSE(passport_.GetPacket(STMID, true));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(original_mid.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(original_smid.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(original_tmid.get()));
  // As first ever update for this user, original SMID == updated SMID
  EXPECT_TRUE(original_smid->Equals(updated_smid1.get()));

  // Bad confirm
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(null_mid,
                          updated_smid1, new_tmid1));
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(updated_mid1,
                          null_mid, new_tmid1));
  EXPECT_EQ(kNullPointer, passport_.ConfirmMasterDataUpdate(updated_mid1,
                          updated_smid1, null_tmid));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(STMID, false));
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(original_mid,
                              updated_smid1, new_tmid1));
  EXPECT_TRUE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(STMID, false));
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(updated_mid1,
                              different_smid, new_tmid1));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_TRUE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(STMID, false));
  EXPECT_EQ(kPacketsNotEqual, passport_.ConfirmMasterDataUpdate(updated_mid1,
                              updated_smid1, original_tmid));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_FALSE(passport_.GetPacket(SMID, false));
  EXPECT_TRUE(passport_.GetPacket(TMID, false));
  EXPECT_TRUE(passport_.GetPacket(STMID, false));

  // Confirm to populate STMID
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid1,
                      updated_smid1, new_tmid1));
  EXPECT_EQ(kSuccess, passport_.ConfirmMasterDataUpdate(updated_mid1,
                      updated_smid1, new_tmid1));
  EXPECT_FALSE(passport_.GetPacket(MID, false));
  EXPECT_FALSE(passport_.GetPacket(SMID, false));
  EXPECT_FALSE(passport_.GetPacket(TMID, false));
  EXPECT_FALSE(passport_.GetPacket(STMID, false));
  ASSERT_TRUE(passport_.GetPacket(MID, true));
  ASSERT_TRUE(passport_.GetPacket(SMID, true));
  ASSERT_TRUE(passport_.GetPacket(TMID, true));
  ASSERT_TRUE(passport_.GetPacket(STMID, true));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());

  // Retry with same data
  std::string updated_master_data2(base::RandomString(10000));
  MidPtr updated_mid2(new MidPacket), updated_smid2(new MidPacket);
  TmidPtr new_tmid2(new TmidPacket), tmid_for_deletion2(new TmidPacket);
  *tmid_for_deletion2 = *original_tmid;
  EXPECT_FALSE(tmid_for_deletion2->name().empty());
  EXPECT_EQ(kSuccess, passport_.UpdateMasterData(updated_master_data2,
            &mid_old_value, &smid_old_value, updated_mid2, updated_smid2,
            new_tmid2, tmid_for_deletion2));
  EXPECT_EQ(updated_mid1->value(), mid_old_value);
  EXPECT_EQ(updated_smid1->value(), smid_old_value);
  EXPECT_NE(original_mid->value(), mid_old_value);
  EXPECT_EQ(original_smid->value(), smid_old_value);
  ASSERT_TRUE(updated_mid2);
  ASSERT_TRUE(updated_smid2);
  ASSERT_TRUE(new_tmid2);
  ASSERT_TRUE(tmid_for_deletion2);
  EXPECT_EQ(1UL, updated_mid2.use_count());
  EXPECT_EQ(1UL, updated_smid2.use_count());
  EXPECT_EQ(1UL, new_tmid2.use_count());
  EXPECT_EQ(1UL, tmid_for_deletion2.use_count());
  EXPECT_EQ(original_tmid->name(), tmid_for_deletion2->name());
  EXPECT_EQ(original_tmid->value(), tmid_for_deletion2->value());
  ASSERT_TRUE(passport_.GetPacket(MID, false));
  ASSERT_TRUE(passport_.GetPacket(SMID, false));
  ASSERT_TRUE(passport_.GetPacket(TMID, false));
  ASSERT_TRUE(passport_.GetPacket(STMID, false));
  EXPECT_TRUE(passport_.GetPacket(MID, false)->Equals(updated_mid2.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, false)->Equals(updated_smid2.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, false)->Equals(new_tmid2.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, false)->name(), new_tmid1->name());
  EXPECT_EQ(passport_.GetPacket(STMID, false)->value(), new_tmid1->value());
  ASSERT_TRUE(passport_.GetPacket(MID, true));
  ASSERT_TRUE(passport_.GetPacket(SMID, true));
  ASSERT_TRUE(passport_.GetPacket(TMID, true));
  ASSERT_TRUE(passport_.GetPacket(STMID, true));
  EXPECT_TRUE(passport_.GetPacket(MID, true)->Equals(updated_mid1.get()));
  EXPECT_TRUE(passport_.GetPacket(SMID, true)->Equals(updated_smid1.get()));
  EXPECT_TRUE(passport_.GetPacket(TMID, true)->Equals(new_tmid1.get()));
  EXPECT_EQ(passport_.GetPacket(STMID, true)->name(), original_tmid->name());
  EXPECT_EQ(passport_.GetPacket(STMID, true)->value(), original_tmid->value());
  EXPECT_FALSE(updated_smid1->Equals(updated_smid2.get()));

  // Retry with same data - shouldn't return tmid_for_deletion this time

  // Revert

}

TEST_F(PassportTest, BEH_PASSPORT_ClearKeyring) {
  FAIL() << "ClearKeyring to be tested";
}

TEST_F(PassportTest, BEH_PASSPORT_SerialiseParseKeyring) {
  FAIL() << "SerialiseParseKeyring to be tested";
}

}  // namespace test

}  // namespace passport

}  // namespace maidsafe
