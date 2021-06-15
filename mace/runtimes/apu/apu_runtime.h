// Copyright 2020 The MACE Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MACE_RUNTIMES_APU_APU_RUNTIME_H_
#define MACE_RUNTIMES_APU_APU_RUNTIME_H_

#include <memory>
#include <string>

#include "mace/core/memory/general_memory_manager.h"
#include "mace/core/runtime/runtime.h"

#if MACE_MTK_APU_VERSION <= 3
#include "mace/runtimes/apu/v1v2v3/apu_wrapper.h"
#else
#include "mace/runtimes/apu/v4/apu_wrapper.h"
#endif


namespace mace {

class ApuRuntime : public Runtime {
 public:
  explicit ApuRuntime(RuntimeContext *runtime_context);
  virtual ~ApuRuntime();
  static ApuRuntime *Get(Runtime *runtime);

  MaceStatus Init(const MaceEngineCfgImpl *engine_config,
                  const MemoryType mem_type) override;

  RuntimeType GetRuntimeType() override;
  std::unique_ptr<Buffer> MakeSliceBuffer(
      const NetDef &net_def, const unsigned char *model_data,
      const index_t model_data_size) override;

  ApuWrapper *GetApuWrapper();
  APUCachePolicy GetCachePolicy();
  const char *GetCacheStorePath();
  const char *GetCacheLoadPath();
  APUPreferenceHint GetPreferenceHint();
  uint8_t GetBoostHint();

 protected:
  virtual std::unique_ptr<Allocator> CreateAllocator();
  MemoryManager *GetMemoryManager(const MemoryType mem_type) override;

 private:
  std::unique_ptr<Allocator> allocator_;
  std::unique_ptr<GeneralMemoryManager> buffer_manager_;

  std::unique_ptr<ApuWrapper> apu_wrapper_;
  APUCachePolicy apu_cache_policy_;
  std::string apu_binary_file_;
  std::string apu_storage_file_;
  uint8_t apu_boost_hint_;
  APUPreferenceHint apu_preference_hint_;
};

}  // namespace mace
#endif  // MACE_RUNTIMES_APU_APU_RUNTIME_H_
