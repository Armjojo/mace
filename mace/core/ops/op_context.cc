// Copyright 2018 The MACE Authors. All Rights Reserved.
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

#include "mace/core/ops/op_context.h"

namespace mace {

OpContext::OpContext(Workspace *ws, Runtime *runtime)
    : runtime_(runtime), ws_(ws), future_(nullptr), fake_warmup_(false) {}

OpContext::~OpContext() = default;

void OpContext::set_runtime(Runtime *runtime) {
  runtime_ = runtime;
}

Runtime* OpContext::runtime() const {
  return runtime_;
}

Workspace* OpContext::workspace() const {
  return ws_;
}

void OpContext::set_future(StatsFuture *future) {
  future_ = future;
}

StatsFuture *OpContext::future() const {
  return future_;
}

void OpContext::set_fake_warmup(bool fake_warmup) {
  fake_warmup_ = fake_warmup;
}

bool OpContext::fake_warmup() const {
  return fake_warmup_;
}

}  // namespace mace
