// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_EXPERIENCE_NULLSPACE_HAPTICFILES_H_
#define FLATBUFFERS_GENERATED_EXPERIENCE_NULLSPACE_HAPTICFILES_H_

#include "flatbuffers/flatbuffers.h"

#include "HapticEffect_generated.h"
#include "HapticFrame_generated.h"
#include "HapticSample_generated.h"
#include "Pattern_generated.h"
#include "Sequence_generated.h"

namespace NullSpace {
namespace HapticFiles {

struct Experience;

struct Experience FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_ITEMS = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>> *items() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>> *>(VT_ITEMS); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_ITEMS) &&
           verifier.Verify(items()) &&
           verifier.VerifyVectorOfTables(items()) &&
           verifier.EndTable();
  }
};

struct ExperienceBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_items(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>>> items) { fbb_.AddOffset(Experience::VT_ITEMS, items); }
  ExperienceBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  ExperienceBuilder &operator=(const ExperienceBuilder &);
  flatbuffers::Offset<Experience> Finish() {
    auto o = flatbuffers::Offset<Experience>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<Experience> CreateExperience(flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>>> items = 0) {
  ExperienceBuilder builder_(_fbb);
  builder_.add_items(items);
  return builder_.Finish();
}

inline flatbuffers::Offset<Experience> CreateExperienceDirect(flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>> *items = nullptr) {
  return CreateExperience(_fbb, items ? _fbb.CreateVector<flatbuffers::Offset<NullSpace::HapticFiles::HapticSample>>(*items) : 0);
}

inline const NullSpace::HapticFiles::Experience *GetExperience(const void *buf) {
  return flatbuffers::GetRoot<NullSpace::HapticFiles::Experience>(buf);
}

inline bool VerifyExperienceBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<NullSpace::HapticFiles::Experience>(nullptr);
}

inline void FinishExperienceBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<NullSpace::HapticFiles::Experience> root) {
  fbb.Finish(root);
}

}  // namespace HapticFiles
}  // namespace NullSpace

#endif  // FLATBUFFERS_GENERATED_EXPERIENCE_NULLSPACE_HAPTICFILES_H_
