// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_NODE_NULLSPACE_HAPTICFILES_H_
#define FLATBUFFERS_GENERATED_NODE_NULLSPACE_HAPTICFILES_H_

#include "flatbuffers/flatbuffers.h"

namespace NullSpace {
namespace HapticFiles {

struct Node;
struct NodeT;

enum NodeType {
  NodeType_Effect = 0,
  NodeType_Sequence = 1,
  NodeType_Pattern = 2,
  NodeType_Experience = 3,
  NodeType_MIN = NodeType_Effect,
  NodeType_MAX = NodeType_Experience
};

inline const char **EnumNamesNodeType() {
  static const char *names[] = { "Effect", "Sequence", "Pattern", "Experience", nullptr };
  return names;
}

inline const char *EnumNameNodeType(NodeType e) { return EnumNamesNodeType()[static_cast<int>(e)]; }

struct NodeT : public flatbuffers::NativeTable {
  NodeType type;
  std::vector<std::unique_ptr<NodeT>> children;
  float time;
  std::string effect;
  float strength;
  float duration;
  uint32_t area;
};

struct Node FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TYPE = 4,
    VT_CHILDREN = 6,
    VT_TIME = 8,
    VT_EFFECT = 10,
    VT_STRENGTH = 12,
    VT_DURATION = 14,
    VT_AREA = 16
  };
  NodeType type() const { return static_cast<NodeType>(GetField<int16_t>(VT_TYPE, 0)); }
  const flatbuffers::Vector<flatbuffers::Offset<Node>> *children() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Node>> *>(VT_CHILDREN); }
  float time() const { return GetField<float>(VT_TIME, 0.0f); }
  const flatbuffers::String *effect() const { return GetPointer<const flatbuffers::String *>(VT_EFFECT); }
  float strength() const { return GetField<float>(VT_STRENGTH, 1.0f); }
  float duration() const { return GetField<float>(VT_DURATION, 0.0f); }
  uint32_t area() const { return GetField<uint32_t>(VT_AREA, 0); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int16_t>(verifier, VT_TYPE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_CHILDREN) &&
           verifier.Verify(children()) &&
           verifier.VerifyVectorOfTables(children()) &&
           VerifyField<float>(verifier, VT_TIME) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_EFFECT) &&
           verifier.Verify(effect()) &&
           VerifyField<float>(verifier, VT_STRENGTH) &&
           VerifyField<float>(verifier, VT_DURATION) &&
           VerifyField<uint32_t>(verifier, VT_AREA) &&
           verifier.EndTable();
  }
  NodeT *UnPack(const flatbuffers::resolver_function_t *resolver = nullptr) const;
};

struct NodeBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_type(NodeType type) { fbb_.AddElement<int16_t>(Node::VT_TYPE, static_cast<int16_t>(type), 0); }
  void add_children(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Node>>> children) { fbb_.AddOffset(Node::VT_CHILDREN, children); }
  void add_time(float time) { fbb_.AddElement<float>(Node::VT_TIME, time, 0.0f); }
  void add_effect(flatbuffers::Offset<flatbuffers::String> effect) { fbb_.AddOffset(Node::VT_EFFECT, effect); }
  void add_strength(float strength) { fbb_.AddElement<float>(Node::VT_STRENGTH, strength, 1.0f); }
  void add_duration(float duration) { fbb_.AddElement<float>(Node::VT_DURATION, duration, 0.0f); }
  void add_area(uint32_t area) { fbb_.AddElement<uint32_t>(Node::VT_AREA, area, 0); }
  NodeBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  NodeBuilder &operator=(const NodeBuilder &);
  flatbuffers::Offset<Node> Finish() {
    auto o = flatbuffers::Offset<Node>(fbb_.EndTable(start_, 7));
    return o;
  }
};

inline flatbuffers::Offset<Node> CreateNode(flatbuffers::FlatBufferBuilder &_fbb,
    NodeType type = NodeType_Effect,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Node>>> children = 0,
    float time = 0.0f,
    flatbuffers::Offset<flatbuffers::String> effect = 0,
    float strength = 1.0f,
    float duration = 0.0f,
    uint32_t area = 0) {
  NodeBuilder builder_(_fbb);
  builder_.add_area(area);
  builder_.add_duration(duration);
  builder_.add_strength(strength);
  builder_.add_effect(effect);
  builder_.add_time(time);
  builder_.add_children(children);
  builder_.add_type(type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Node> CreateNodeDirect(flatbuffers::FlatBufferBuilder &_fbb,
    NodeType type = NodeType_Effect,
    const std::vector<flatbuffers::Offset<Node>> *children = nullptr,
    float time = 0.0f,
    const char *effect = nullptr,
    float strength = 1.0f,
    float duration = 0.0f,
    uint32_t area = 0) {
  return CreateNode(_fbb, type, children ? _fbb.CreateVector<flatbuffers::Offset<Node>>(*children) : 0, time, effect ? _fbb.CreateString(effect) : 0, strength, duration, area);
}

inline flatbuffers::Offset<Node> CreateNode(flatbuffers::FlatBufferBuilder &_fbb, const NodeT *_o, const flatbuffers::rehasher_function_t *rehasher = nullptr);

inline NodeT *Node::UnPack(const flatbuffers::resolver_function_t *resolver) const {
  (void)resolver;
  auto _o = new NodeT();
  { auto _e = type(); _o->type = _e; };
  { auto _e = children(); if (_e) { for (flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) { _o->children.push_back(std::unique_ptr<NodeT>(_e->Get(_i)->UnPack(resolver))); } } };
  { auto _e = time(); _o->time = _e; };
  { auto _e = effect(); if (_e) _o->effect = _e->str(); };
  { auto _e = strength(); _o->strength = _e; };
  { auto _e = duration(); _o->duration = _e; };
  { auto _e = area(); _o->area = _e; };
  return _o;
}

inline flatbuffers::Offset<Node> CreateNode(flatbuffers::FlatBufferBuilder &_fbb, const NodeT *_o, const flatbuffers::rehasher_function_t *rehasher) {
  (void)rehasher;
  return CreateNode(_fbb,
    _o->type,
    _o->children.size() ? _fbb.CreateVector<flatbuffers::Offset<Node>>(_o->children.size(), [&](size_t i) { return CreateNode(_fbb, _o->children[i].get(), rehasher); }) : 0,
    _o->time,
    _o->effect.size() ? _fbb.CreateString(_o->effect) : 0,
    _o->strength,
    _o->duration,
    _o->area);
}

inline const NullSpace::HapticFiles::Node *GetNode(const void *buf) {
  return flatbuffers::GetRoot<NullSpace::HapticFiles::Node>(buf);
}

inline bool VerifyNodeBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<NullSpace::HapticFiles::Node>(nullptr);
}

inline void FinishNodeBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<NullSpace::HapticFiles::Node> root) {
  fbb.Finish(root);
}

inline std::unique_ptr<NodeT> UnPackNode(const void *buf, const flatbuffers::resolver_function_t *resolver = nullptr) {
  return std::unique_ptr<NodeT>(GetNode(buf)->UnPack(resolver));
}

}  // namespace HapticFiles
}  // namespace NullSpace

#endif  // FLATBUFFERS_GENERATED_NODE_NULLSPACE_HAPTICFILES_H_
