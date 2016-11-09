// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_HAPTICEFFECT_NULLSPACE_HAPTICFILES_H_
#define FLATBUFFERS_GENERATED_HAPTICEFFECT_NULLSPACE_HAPTICFILES_H_

#include "flatbuffers/flatbuffers.h"

namespace NullSpace {
namespace HapticFiles {

struct HapticEffect;

struct HapticEffect FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_TIME = 4,
    VT_EFFECT = 6,
    VT_STRENGTH = 8,
    VT_DURATION = 10,
    VT_REPEAT = 12
  };
  float time() const { return GetField<float>(VT_TIME, 0.0f); }
  const flatbuffers::String *effect() const { return GetPointer<const flatbuffers::String *>(VT_EFFECT); }
  float strength() const { return GetField<float>(VT_STRENGTH, 0.0f); }
  float duration() const { return GetField<float>(VT_DURATION, 0.0f); }
  int32_t repeat() const { return GetField<int32_t>(VT_REPEAT, 0); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_TIME) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_EFFECT) &&
           verifier.Verify(effect()) &&
           VerifyField<float>(verifier, VT_STRENGTH) &&
           VerifyField<float>(verifier, VT_DURATION) &&
           VerifyField<int32_t>(verifier, VT_REPEAT) &&
           verifier.EndTable();
  }
};

struct HapticEffectBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_time(float time) { fbb_.AddElement<float>(HapticEffect::VT_TIME, time, 0.0f); }
  void add_effect(flatbuffers::Offset<flatbuffers::String> effect) { fbb_.AddOffset(HapticEffect::VT_EFFECT, effect); }
  void add_strength(float strength) { fbb_.AddElement<float>(HapticEffect::VT_STRENGTH, strength, 0.0f); }
  void add_duration(float duration) { fbb_.AddElement<float>(HapticEffect::VT_DURATION, duration, 0.0f); }
  void add_repeat(int32_t repeat) { fbb_.AddElement<int32_t>(HapticEffect::VT_REPEAT, repeat, 0); }
  HapticEffectBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  HapticEffectBuilder &operator=(const HapticEffectBuilder &);
  flatbuffers::Offset<HapticEffect> Finish() {
    auto o = flatbuffers::Offset<HapticEffect>(fbb_.EndTable(start_, 5));
    return o;
  }
};

inline flatbuffers::Offset<HapticEffect> CreateHapticEffect(flatbuffers::FlatBufferBuilder &_fbb,
    float time = 0.0f,
    flatbuffers::Offset<flatbuffers::String> effect = 0,
    float strength = 0.0f,
    float duration = 0.0f,
    int32_t repeat = 0) {
  HapticEffectBuilder builder_(_fbb);
  builder_.add_repeat(repeat);
  builder_.add_duration(duration);
  builder_.add_strength(strength);
  builder_.add_effect(effect);
  builder_.add_time(time);
  return builder_.Finish();
}

inline flatbuffers::Offset<HapticEffect> CreateHapticEffectDirect(flatbuffers::FlatBufferBuilder &_fbb,
    float time = 0.0f,
    const char *effect = nullptr,
    float strength = 0.0f,
    float duration = 0.0f,
    int32_t repeat = 0) {
  return CreateHapticEffect(_fbb, time, effect ? _fbb.CreateString(effect) : 0, strength, duration, repeat);
}

inline const NullSpace::HapticFiles::HapticEffect *GetHapticEffect(const void *buf) {
  return flatbuffers::GetRoot<NullSpace::HapticFiles::HapticEffect>(buf);
}

inline bool VerifyHapticEffectBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<NullSpace::HapticFiles::HapticEffect>(nullptr);
}

inline void FinishHapticEffectBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<NullSpace::HapticFiles::HapticEffect> root) {
  fbb.Finish(root);
}

}  // namespace HapticFiles
}  // namespace NullSpace

#endif  // FLATBUFFERS_GENERATED_HAPTICEFFECT_NULLSPACE_HAPTICFILES_H_
