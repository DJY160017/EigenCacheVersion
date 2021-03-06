/*******************************************************************************
* Copyright 2016-2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn.hpp"
#include "cpu_isa_traits.hpp"

namespace mkldnn {

struct concat_test_params {
    const engine::kind engine_kind;
    size_t concat_dimension;
    std::vector<memory::format> srcs_format;
    memory::format dst_format;
    std::vector<memory::dims> srcs_cds;
    memory::dims dst_cds;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
class concat_test: public ::testing::TestWithParam<concat_test_params> {
    void check_data(const std::vector<memory> &srcs, const memory &dst,
            int concat_dim) {
        const data_t *dst_data = (const data_t *)dst.get_data_handle();
        const auto &dst_d = dst.get_primitive_desc().desc();
        const auto dst_dims = dst_d.data.dims;
        const int* dst_pdims = dst_d.data.layout_desc.blocking.padding_dims;

        int acc_concat_dim = 0;
        const auto ndims = dst_d.data.ndims;

        for (size_t num = 0; num < srcs.size(); num++) {
            const data_t *src_data = (const data_t *)srcs[num].get_data_handle();
            const auto &src_d = srcs[num].get_primitive_desc().desc();
            const int* src_dims = src_d.data.dims;
            const int* src_pdims = src_d.data.layout_desc.blocking.padding_dims;

            auto N = src_dims[0];
            auto C = src_dims[1];
            auto C_PADDED = src_pdims[1];
            auto D = (ndims == 5) ? src_dims[2] : 1;
            auto H = src_dims[ndims-2];
            auto W = src_dims[ndims-1];

            auto DST_C_PADDED = dst_pdims[1];
            auto DST_D = (ndims == 5) ? dst_dims[2] : 1;
            auto DST_H = dst_dims[ndims-2];
            auto DST_W = dst_dims[ndims-1];

            for (auto n = 0; n < N; n++)
            for (auto c = 0; c < C; c++)
            for (auto d = 0; d < D; d++)
            for (auto h = 0; h < H; h++)
            for (auto w = 0; w < W; w++) {
                auto src_idx = w + W*h + H*W*d + D*H*W*c + C_PADDED*D*H*W*n;

                auto adj_dst_dim = [&](int dim, int dim_sz) {
                    if (concat_dim == dim) return dim_sz + acc_concat_dim;
                    return dim_sz;
                };
                auto dst_idx = adj_dst_dim(ndims-1, w)
                    + DST_W*adj_dst_dim(ndims-2, h)
                    + DST_D*DST_H*DST_W*adj_dst_dim(1, c)
                    + DST_C_PADDED*DST_D*DST_H*DST_W*adj_dst_dim(0, n);
                if (ndims == 5) dst_idx += DST_H*DST_W*adj_dst_dim(2, d);
                EXPECT_NEAR(src_data[map_index(src_d, src_idx)],
                            dst_data[map_index(dst_d, dst_idx)],
                            1e-7);
            }

            acc_concat_dim += src_dims[concat_dim];
        }
    }

protected:
    virtual void SetUp() {
        SKIP_IF(data_traits<data_t>::data_type == impl::data_type::bf16
                && !impl::cpu::mayiuse(impl::cpu::avx512_core),
                "current ISA doesn't support bfloat16 data type");
        concat_test_params p
            = ::testing::TestWithParam<decltype(p)>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    virtual void Test() {
        concat_test_params p
            = ::testing::TestWithParam<concat_test_params>::GetParam();

        int src_dim_sum = 0;
        for (size_t i = 0; i < p.srcs_cds.size(); i++) {
            for (size_t dim = 0; dim < p.dst_cds.size(); dim++) {
                if (dim == p.concat_dimension)
                    src_dim_sum += p.srcs_cds[i][dim];
                else if (p.expect_to_fail == false) {
                    ASSERT_TRUE(p.srcs_cds[i][dim] == p.dst_cds[dim]);
                }
            }
        }

        if (p.expect_to_fail == false) {
            ASSERT_TRUE(src_dim_sum == p.dst_cds[p.concat_dimension]);
        }

        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        auto eng = engine(p.engine_kind, 0);
        memory::data_type data_type = data_traits<data_t>::data_type;

        std::vector<memory::primitive_desc> srcs_pd;
        std::vector<memory> srcs;
        for (size_t i = 0; i < p.srcs_cds.size(); i++) {
            auto desc = memory::desc(p.srcs_cds[i], data_type, p.srcs_format[i]);
            auto mpd = memory::primitive_desc(desc, eng);
            auto src_memory = memory(mpd);
            const size_t sz = src_memory.get_primitive_desc().get_size() / sizeof(data_t);
            fill_data<data_t>(sz, (data_t *)src_memory.get_data_handle());
            check_zero_tail<data_t>(1, src_memory);
            srcs_pd.push_back(mpd);
            srcs.push_back(src_memory);
        }

        auto dst_desc = memory::desc(p.dst_cds, data_type, p.dst_format);
        auto concat_pd = concat::primitive_desc(dst_desc, static_cast<int>(p.concat_dimension), srcs_pd);
        auto dst = memory(concat_pd.dst_primitive_desc());
        fill_data<data_t>(dst.get_primitive_desc().get_size() / sizeof(data_t),
            (data_t *)dst.get_data_handle());
        check_zero_tail<data_t>(1, dst);

        std::vector<primitive::at> inputs;
        for (size_t i = 0; i < p.srcs_cds.size(); i++) {
            inputs.push_back(srcs[i]);
        }
        auto c = concat(concat_pd, inputs, dst);

        ASSERT_EQ(concat_pd.dst_primitive_desc().desc().data.format,
                dst_desc.data.format);
        ASSERT_EQ(concat_pd.dst_primitive_desc().desc().data.ndims,
                dst_desc.data.ndims);

        std::vector<primitive> pipeline;
        pipeline.push_back(c);
        auto s = stream(stream::kind::eager);
        s.submit(pipeline).wait();

        check_data(srcs, dst, static_cast<int>(p.concat_dimension));
        check_zero_tail<data_t>(0, dst);
    }
};

using concat_test_float = concat_test<float>;
using concat_test_s8 = concat_test<int8_t>;
using concat_test_bf16 = concat_test<mkldnn_bfloat16_t>;

TEST_P(concat_test_float, TestsConcat) {}
TEST_P(concat_test_s8, TestsConcat) {}
TEST_P(concat_test_bf16, TestsConcat) {}

using fmt = memory::format;

INSTANTIATE_TEST_CASE_P(TestConcat_ZeroDim, concat_test_float, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{4, 0, 5, 5}, {4, 5, 5, 5}}, {4, 5, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{4, 4, 5, 5}, {4, 0, 5, 5}}, {4, 4, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw8c}, fmt::nChw8c, {{4, 0, 5, 5}, {4, 5, 5, 5}}, {4, 5, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw8c}, fmt::nChw8c, {{4, 4, 5, 5}, {4, 0, 5, 5}}, {4, 4, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{0, 4, 5, 5}, {0, 2, 5, 5}}, {0, 6, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{2, 4, 0, 5}, {2, 2, 0, 5}}, {2, 6, 0, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nhwc, fmt::nhwc}, fmt::nhwc,  {{0, 4, 5, 5}, {0, 2, 5, 5}}, {0, 6, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nchw, fmt::nchw}, fmt::nchw,  {{0, 4, 5, 5}, {0, 2, 5, 5}}, {0, 6, 5, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nhwc, fmt::nhwc}, fmt::nhwc,  {{2, 4, 0, 5}, {2, 2, 0, 5}}, {2, 6, 0, 5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nchw, fmt::nchw}, fmt::nchw,  {{2, 4, 0, 5}, {2, 2, 0, 5}}, {2, 6, 0, 5}}
));

INSTANTIATE_TEST_CASE_P(TestConcat_EF, concat_test_float, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{4, 2, 5, 5}, {4, 5, 5, 5}}, {4, 5, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 2, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{4, 2, 5, 5}, {4, 3, 5, 5}}, {4, 5, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 5, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{4, 4, 5, 5}, {4, 0, 5, 5}}, {4, 4, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw8c}, fmt::nChw8c, {{4, -1, 5, 5}, {4, 5, 5, 5}}, {4, 5, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw8c}, fmt::nChw8c, {{4, 4, 5, 5}, {4, 4, 5, 5}}, {4, 4, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{0, 4, 5, 5}, {0, 4, 5, 5}}, {0, 6, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c, fmt::nChw16c}, fmt::nchw,  {{2, 4, 2, 5}, {2, 2, 1, 5}}, {2, 6, 2, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nhwc, fmt::nhwc}, fmt::nhwc,  {{1, 4, 5, 5}, {1, 2, 5, 5}}, {1, 7, 5, 5}, true, mkldnn_invalid_arguments},
    concat_test_params{engine::kind::cpu, 1, {fmt::nchw, fmt::nchw}, fmt::nchw,  {{1, 4, 5, 5}, {1, 2, 5, 5}}, {1, 6, 6, 5}, true, mkldnn_invalid_arguments}
));

INSTANTIATE_TEST_CASE_P(TestConcat_padded, concat_test_float, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nChw16c, {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}, true, mkldnn_unimplemented},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nchw,    {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c,  fmt::nChw8c},  fmt::nchw,    {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw8c},  fmt::nchw,    {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c,  fmt::nChw16c}, fmt::nchw,    {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nChw16c, {{4,  4, 5, 5}, {4,  6, 5, 5}}, {4, 10,  5,  5}, true, mkldnn_unimplemented},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nchw,    {{4,  4, 5, 5}, {4,  6, 5, 5}}, {4, 10,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nchw,    fmt::nChw16c}, fmt::nChw16c, {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}, true, mkldnn_unimplemented},
    concat_test_params{engine::kind::cpu, 1, {fmt::nchw,    fmt::nChw16c}, fmt::nchw,    {{4, 25, 5, 5}, {4, 45, 5, 5}}, {4, 70,  5,  5}},
    // right border
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nChw16c, {{4, 16, 5, 5}, {4,  3, 5, 5}}, {4, 19,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw16c, fmt::nChw16c}, fmt::nChw8c, {{4, 16, 5, 5}, {4,  3, 5, 5}}, {4, 19,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c,  fmt::nChw8c},  fmt::nChw8c, {{4, 8, 5, 5}, {4,  3, 5, 5}}, {4, 11,  5,  5}},
    concat_test_params{engine::kind::cpu, 1, {fmt::nChw8c,  fmt::nChw16c}, fmt::nChw16c, {{4, 8, 5, 5}, {4,  3, 5, 5}}, {4, 11,  5,  5}},
    // not over channels
    concat_test_params{engine::kind::cpu, 2, {fmt::nChw16c, fmt::nChw16c}, fmt::nchw,    {{4, 25, 5, 5}, {4, 25, 5, 5}}, {4, 25, 10,  5}},
    concat_test_params{engine::kind::cpu, 2, {fmt::nChw8c,  fmt::nChw8c},  fmt::nchw,    {{4, 25, 5, 5}, {4, 25, 5, 5}}, {4, 25, 10,  5}},
    concat_test_params{engine::kind::cpu, 2, {fmt::nChw8c,  fmt::nChw16c}, fmt::nchw,    {{4, 25, 5, 5}, {4, 25, 5, 5}}, {4, 25, 10,  5}}
));

INSTANTIATE_TEST_CASE_P(TestConcat3D, concat_test_float, ::testing::Values(
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {4, 8, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 2,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 6, 4, 5}},
    concat_test_params{engine::kind::cpu, 3,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 8, 5}},
    concat_test_params{engine::kind::cpu, 4,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 4, 10}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {4, 8, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nCdhw8c, memory::format::ncdhw}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 2,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 6, 4, 5}},
    concat_test_params{engine::kind::cpu, 3,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 8, 5}},
    concat_test_params{engine::kind::cpu, 4,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 4, 10}}
));

INSTANTIATE_TEST_CASE_P(TestConcat, concat_test_float, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nchw, memory::format::nchw}, memory::format::nchw,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {2, 16, 3, 4}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nchw, memory::format::nchw}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nhwc, memory::format::nhwc}, memory::format::nhwc,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nchw,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},

    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nchw, memory::format::nchw}, memory::format::nchw,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {4, 8, 3, 4}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nchw, memory::format::nchw}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nchw,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},

    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 8, 1, 1}, {2, 8, 1, 1}}, {2, 16, 1, 1}},

    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw16c}, memory::format::nChw8c,
    {{2, 8, 1, 1}, {2, 16, 1, 1}}, {2, 24, 1, 1}}
));

INSTANTIATE_TEST_CASE_P(TestConcat, concat_test_s8, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nhwc, memory::format::nhwc}, memory::format::nhwc,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {2, 16, 3, 4}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nchw, memory::format::nchw}, memory::format::nchw,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {2, 16, 3, 4}}
    ));

INSTANTIATE_TEST_CASE_P(
        TestConcat_ZeroDim, concat_test_bf16,
        ::testing::Values(
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nChw8c,
                        { { 4, 0, 5, 5 }, { 4, 5, 5, 5 } }, { 4, 5, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nChw8c,
                        { { 4, 4, 5, 5 }, { 4, 0, 5, 5 } }, { 4, 4, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nhwc, fmt::nhwc }, fmt::nhwc,
                        { { 0, 4, 5, 5 }, { 0, 2, 5, 5 } }, { 0, 6, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nhwc, fmt::nhwc }, fmt::nhwc,
                        { { 2, 4, 0, 5 }, { 2, 2, 0, 5 } }, { 2, 6, 0, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nchw, fmt::nchw }, fmt::nchw,
                        { { 0, 4, 5, 5 }, { 0, 2, 5, 5 } }, { 0, 6, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nchw, fmt::nchw }, fmt::nchw,
                        { { 2, 4, 0, 5 }, { 2, 2, 0, 5 } }, { 2, 6, 0, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 0, 5, 5 }, { 4, 5, 5, 5 } }, { 4, 5, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 4, 5, 5 }, { 4, 0, 5, 5 } }, { 4, 4, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 0, 4, 5, 5 }, { 0, 2, 5, 5 } }, { 0, 6, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 2, 4, 0, 5 }, { 2, 2, 0, 5 } }, { 2, 6, 0, 5 } }
));

INSTANTIATE_TEST_CASE_P(
        TestConcat_EF, concat_test_bf16,
        ::testing::Values(
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 2, 5, 5 }, { 4, 5, 5, 5 } }, { 4, 5, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 2,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 2, 5, 5 }, { 4, 3, 5, 5 } }, { 4, 5, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 5,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 4, 5, 5 }, { 4, 0, 5, 5 } }, { 4, 4, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nChw8c,
                        { { 4, -1, 5, 5 }, { 4, 5, 5, 5 } }, { 4, 5, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nChw8c,
                        { { 4, 4, 5, 5 }, { 4, 4, 5, 5 } }, { 4, 4, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 0, 4, 5, 5 }, { 0, 4, 5, 5 } }, { 0, 6, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 2, 4, 2, 5 }, { 2, 2, 1, 5 } }, { 2, 6, 2, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nhwc, fmt::nhwc }, fmt::nhwc,
                        { { 1, 4, 5, 5 }, { 1, 2, 5, 5 } }, { 1, 7, 5, 5 },
                        true, mkldnn_invalid_arguments },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nchw, fmt::nchw }, fmt::nchw,
                        { { 1, 4, 5, 5 }, { 1, 2, 5, 5 } }, { 1, 6, 6, 5 },
                        true, mkldnn_invalid_arguments }
));

INSTANTIATE_TEST_CASE_P(
        TestConcat_padded, concat_test_bf16,
        ::testing::Values(
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nChw16c,
                        { { 4, 4, 5, 5 }, { 4, 6, 5, 5 } }, { 4, 10, 5, 5 },
                        true, mkldnn_unimplemented },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nChw16c,
                        { { 4, 16, 5, 5 }, { 4, 3, 5, 5 } }, { 4, 19, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nChw8c,
                        { { 4, 8, 5, 5 }, { 4, 3, 5, 5 } }, { 4, 11, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nChw16c,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 },
                        true, mkldnn_unimplemented },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw8c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 4, 5, 5 }, { 4, 6, 5, 5 } }, { 4, 10, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nchw, fmt::nChw16c }, fmt::nChw16c,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 },
                        true, mkldnn_unimplemented },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nchw, fmt::nChw16c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 45, 5, 5 } }, { 4, 70, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nChw8c,
                        { { 4, 16, 5, 5 }, { 4, 3, 5, 5 } }, { 4, 19, 5, 5 } },
                concat_test_params{ engine::kind::cpu, 1,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nChw16c,
                        { { 4, 8, 5, 5 }, { 4, 3, 5, 5 } }, { 4, 11, 5, 5 } },
                // not over channels
                concat_test_params{ engine::kind::cpu, 2,
                        { fmt::nChw16c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 25, 5, 5 } },
                        { 4, 25, 10, 5 } },
                concat_test_params{ engine::kind::cpu, 2,
                        { fmt::nChw8c, fmt::nChw8c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 25, 5, 5 } },
                        { 4, 25, 10, 5 } },
                concat_test_params{ engine::kind::cpu, 2,
                        { fmt::nChw8c, fmt::nChw16c }, fmt::nchw,
                        { { 4, 25, 5, 5 }, { 4, 25, 5, 5 } },
                        { 4, 25, 10, 5 } }
));

INSTANTIATE_TEST_CASE_P(TestConcat3D, concat_test_bf16, ::testing::Values(
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {4, 8, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 2,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 6, 4, 5}},
    concat_test_params{engine::kind::cpu, 3,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 8, 5}},
    concat_test_params{engine::kind::cpu, 4,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::ncdhw,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 4, 10}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {4, 8, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 2,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 6, 4, 5}},
    concat_test_params{engine::kind::cpu, 3,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 8, 5}},
    concat_test_params{engine::kind::cpu, 4,
    {memory::format::nCdhw8c, memory::format::nCdhw8c}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 8, 3, 4, 10}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nCdhw8c, memory::format::ncdhw}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::ncdhw, memory::format::ncdhw}, memory::format::nCdhw8c,
    {{2, 8, 3, 4, 5}, {2, 8, 3, 4, 5}}, {2, 16, 3, 4, 5}}
));

INSTANTIATE_TEST_CASE_P(TestConcat, concat_test_bf16, ::testing::Values(
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nchw, memory::format::nchw}, memory::format::nchw,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {2, 16, 3, 4}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nhwc, memory::format::nhwc}, memory::format::nhwc,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nchw, memory::format::nchw}, memory::format::nchw,
    {{2, 8, 3, 4}, {2, 8, 3, 4}}, {4, 8, 3, 4}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nChw8c,
    {{2, 8, 1, 1}, {2, 8, 1, 1}}, {2, 16, 1, 1}},

    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nchw, memory::format::nchw}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},
    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nchw,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {2, 32, 1, 1}},

    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nchw, memory::format::nchw}, memory::format::nChw8c,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},
    concat_test_params{engine::kind::cpu, 0,
    {memory::format::nChw8c, memory::format::nChw8c}, memory::format::nchw,
    {{2, 16, 1, 1}, {2, 16, 1, 1}}, {4, 16, 1, 1}},

    concat_test_params{engine::kind::cpu, 1,
    {memory::format::nChw8c, memory::format::nChw16c}, memory::format::nChw8c,
    {{2, 8, 1, 1}, {2, 16, 1, 1}}, {2, 24, 1, 1}}
));
}
