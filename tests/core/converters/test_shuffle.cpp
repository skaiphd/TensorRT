#include <string>
#include "gtest/gtest.h"
#include "torch/csrc/jit/ir/irparser.h"
#include "tests/util/util.h"
#include "core/compiler.h"

TEST(Converters, ATenReshapeConvertsCorrectly) {
    const auto graph = R"IR(
      graph(%0 : Tensor):
        %1 : int = prim::Constant[value=3]()
        %2 : int = prim::Constant[value=2]()
        %3 : int[] = prim::ListConstruct(%1, %2)
        %4 : Tensor = aten::reshape(%0, %3)
        return (%4))IR";

    auto g = std::make_shared<torch::jit::Graph>();
    torch::jit::parseIR(graph, &*g);

    auto in = at::randint(0, 5, {2, 3}, {at::kCUDA});
    auto params = trtorch::core::conversion::get_named_params(g->inputs(), {});
    auto jit_results = trtorch::tests::util::RunGraph(g, params, {in});

    in = at::clone(in);
    params = trtorch::core::conversion::get_named_params(g->inputs(), {});
    auto trt_results = trtorch::tests::util::RunGraphEngine(g, params, {in});
    auto trt = trt_results[0].reshape_as(jit_results[0]);

    ASSERT_TRUE(trtorch::tests::util::almostEqual(jit_results[0], trt, 2e-6));
}