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

#include "mace/ops/opencl/image/addn.h"

#include "mace/runtimes/opencl/opencl_runtime.h"

namespace mace {
namespace ops {
namespace opencl {
namespace image {

MaceStatus AddNKernel::Compute(
    OpContext *context,
    const std::vector<const Tensor *> &input_tensors,
    Tensor *output_tensor) {
  size_t size = input_tensors.size();
  MACE_CHECK(size >= 2 && input_tensors[0] != nullptr);

  const index_t batch = input_tensors[0]->dim(0);
  const index_t height = input_tensors[0]->dim(1);
  const index_t width = input_tensors[0]->dim(2);
  const index_t channels = input_tensors[0]->dim(3);

  auto executor = OpenclRuntime::Get(context)->GetOpenclExecutor();
  MACE_OUT_OF_RANGE_DEFINITION;

  for (size_t i = 1; i < size; ++i) {
    MACE_CHECK_NOTNULL(input_tensors[i]);
    MACE_CHECK(batch == input_tensors[i]->dim(0));
    MACE_CHECK(height == input_tensors[i]->dim(1));
    MACE_CHECK(width == input_tensors[i]->dim(2));
    MACE_CHECK(channels == input_tensors[i]->dim(3));
  }

  if (kernel_.get() == nullptr) {
    if (input_tensors.size() > 4) {
      MACE_NOT_IMPLEMENTED;
    }
    std::set<std::string> built_options;
    MACE_OUT_OF_RANGE_CONFIG;
    MACE_NON_UNIFORM_WG_CONFIG;
    std::string kernel_name = MACE_OBFUSCATE_SYMBOL("addn");
    built_options.emplace("-Daddn=" + kernel_name);
    built_options.emplace("-DDATA_TYPE=" + DtToCLDt(DT_FLOAT));
    built_options.emplace("-DCMD_DATA_TYPE=" + DtToCLCMDDt(DT_FLOAT));
    built_options.emplace(MakeString("-DINPUT_NUM=", input_tensors.size()));

    MACE_RETURN_IF_ERROR(executor->BuildKernel("addn", kernel_name,
                                               built_options, &kernel_));

    kwg_size_ =
        static_cast<uint32_t>(executor->GetKernelMaxWorkGroupSize(kernel_));
  }

  std::vector<index_t> output_shape = input_tensors[0]->shape();

  const index_t channel_blocks = RoundUpDiv4(channels);
  const index_t width_pixels = channel_blocks * width;
  const index_t batch_height_pixels = batch * height;

  const uint32_t gws[2] = {static_cast<uint32_t>(width_pixels),
                           static_cast<uint32_t>(batch_height_pixels)};

  MACE_OUT_OF_RANGE_INIT(kernel_);
  if (IsResetArgsNeeded(context, input_shape_, input_tensors[0]->shape())) {
    MACE_RETURN_IF_ERROR(output_tensor->Resize(output_shape));

    uint32_t idx = 0;
    MACE_OUT_OF_RANGE_SET_ARGS(kernel_);
    MACE_SET_2D_GWS_ARGS(kernel_, gws);
    for (auto input : input_tensors) {
      kernel_.setArg(idx++, *(input->memory<cl::Image>()));
    }
    kernel_.setArg(idx++, *(output_tensor->mutable_memory<cl::Image>()));

    input_shape_ = input_tensors[0]->shape();
  }

  const std::vector<uint32_t> lws = {kwg_size_ / 16, 16, 0};
  std::string tuning_key =
      Concat("addn_opencl_kernel", output_tensor->dim(0), output_tensor->dim(1),
             output_tensor->dim(2), output_tensor->dim(3));
  MACE_RETURN_IF_ERROR(TuningOrRun2DKernel(executor, kernel_, tuning_key,
                                           gws, lws, context->future(), context));
  MACE_OUT_OF_RANGE_VALIDATION;
  return MaceStatus::MACE_SUCCESS;
}

}  // namespace image
}  // namespace opencl
}  // namespace ops
}  // namespace mace
