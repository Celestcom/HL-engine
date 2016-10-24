// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_ENGINEPACKET_NULLSPACE_COMMUNICATION_H_
#define FLATBUFFERS_GENERATED_ENGINEPACKET_NULLSPACE_COMMUNICATION_H_

#include "flatbuffers/flatbuffers.h"

#include "SuitStatusUpdate_generated.h"

namespace NullSpace {
namespace Communication {

struct EnginePacket;

enum PacketType {
  PacketType_NONE = 0,
  PacketType_SuitStatusUpdate = 1,
  PacketType_MIN = PacketType_NONE,
  PacketType_MAX = PacketType_SuitStatusUpdate
};

inline const char **EnumNamesPacketType() {
  static const char *names[] = { "NONE", "SuitStatusUpdate", nullptr };
  return names;
}

inline const char *EnumNamePacketType(PacketType e) { return EnumNamesPacketType()[static_cast<int>(e)]; }

template<typename T> struct PacketTypeTraits {
  static const PacketType enum_value = PacketType_NONE;
};

template<> struct PacketTypeTraits<NullSpace::Communication::SuitStatusUpdate> {
  static const PacketType enum_value = PacketType_SuitStatusUpdate;
};

inline bool VerifyPacketType(flatbuffers::Verifier &verifier, const void *union_obj, PacketType type);

struct EnginePacket FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_PACKET_TYPE = 4,
    VT_PACKET = 6
  };
  PacketType packet_type() const { return static_cast<PacketType>(GetField<uint8_t>(VT_PACKET_TYPE, 0)); }
  const void *packet() const { return GetPointer<const void *>(VT_PACKET); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_PACKET_TYPE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_PACKET) &&
           VerifyPacketType(verifier, packet(), packet_type()) &&
           verifier.EndTable();
  }
};

struct EnginePacketBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_packet_type(PacketType packet_type) { fbb_.AddElement<uint8_t>(EnginePacket::VT_PACKET_TYPE, static_cast<uint8_t>(packet_type), 0); }
  void add_packet(flatbuffers::Offset<void> packet) { fbb_.AddOffset(EnginePacket::VT_PACKET, packet); }
  EnginePacketBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  EnginePacketBuilder &operator=(const EnginePacketBuilder &);
  flatbuffers::Offset<EnginePacket> Finish() {
    auto o = flatbuffers::Offset<EnginePacket>(fbb_.EndTable(start_, 2));
    return o;
  }
};

inline flatbuffers::Offset<EnginePacket> CreateEnginePacket(flatbuffers::FlatBufferBuilder &_fbb,
    PacketType packet_type = PacketType_NONE,
    flatbuffers::Offset<void> packet = 0) {
  EnginePacketBuilder builder_(_fbb);
  builder_.add_packet(packet);
  builder_.add_packet_type(packet_type);
  return builder_.Finish();
}

inline bool VerifyPacketType(flatbuffers::Verifier &verifier, const void *union_obj, PacketType type) {
  switch (type) {
    case PacketType_NONE: return true;
    case PacketType_SuitStatusUpdate: return verifier.VerifyTable(reinterpret_cast<const NullSpace::Communication::SuitStatusUpdate *>(union_obj));
    default: return false;
  }
}

inline const NullSpace::Communication::EnginePacket *GetEnginePacket(const void *buf) {
  return flatbuffers::GetRoot<NullSpace::Communication::EnginePacket>(buf);
}

inline bool VerifyEnginePacketBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<NullSpace::Communication::EnginePacket>(nullptr);
}

inline void FinishEnginePacketBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<NullSpace::Communication::EnginePacket> root) {
  fbb.Finish(root);
}

}  // namespace Communication
}  // namespace NullSpace

#endif  // FLATBUFFERS_GENERATED_ENGINEPACKET_NULLSPACE_COMMUNICATION_H_
