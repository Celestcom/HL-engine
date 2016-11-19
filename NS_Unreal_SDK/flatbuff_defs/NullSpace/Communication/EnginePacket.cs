// automatically generated by the FlatBuffers compiler, do not modify

namespace NullSpace.Communication
{

using System;
using FlatBuffers;

public struct EnginePacket : IFlatbufferObject
{
  private Table __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public static EnginePacket GetRootAsEnginePacket(ByteBuffer _bb) { return GetRootAsEnginePacket(_bb, new EnginePacket()); }
  public static EnginePacket GetRootAsEnginePacket(ByteBuffer _bb, EnginePacket obj) { return (obj.__assign(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __p.bb_pos = _i; __p.bb = _bb; }
  public EnginePacket __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public PacketType PacketType { get { int o = __p.__offset(4); return o != 0 ? (PacketType)__p.bb.Get(o + __p.bb_pos) : PacketType.NONE; } }
  public TTable? Packet<TTable>() where TTable : struct, IFlatbufferObject { int o = __p.__offset(6); return o != 0 ? (TTable?)__p.__union<TTable>(o) : null; }

  public static Offset<EnginePacket> CreateEnginePacket(FlatBufferBuilder builder,
      PacketType packet_type = PacketType.NONE,
      int packetOffset = 0) {
    builder.StartObject(2);
    EnginePacket.AddPacket(builder, packetOffset);
    EnginePacket.AddPacketType(builder, packet_type);
    return EnginePacket.EndEnginePacket(builder);
  }

  public static void StartEnginePacket(FlatBufferBuilder builder) { builder.StartObject(2); }
  public static void AddPacketType(FlatBufferBuilder builder, PacketType packetType) { builder.AddByte(0, (byte)packetType, 0); }
  public static void AddPacket(FlatBufferBuilder builder, int packetOffset) { builder.AddOffset(1, packetOffset, 0); }
  public static Offset<EnginePacket> EndEnginePacket(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<EnginePacket>(o);
  }
  public static void FinishEnginePacketBuffer(FlatBufferBuilder builder, Offset<EnginePacket> offset) { builder.Finish(offset.Value); }
};


}
