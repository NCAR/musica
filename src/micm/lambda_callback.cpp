// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/lambda_callback.hpp>

namespace musica
{
  namespace
  {
    std::map<std::string, int> g_label_to_id;
    std::function<double(int, double, double, double)> g_dispatcher;
  }  // anonymous namespace

  std::map<std::string, int>& GetLambdaCallbackIds()
  {
    return g_label_to_id;
  }

  void SetLambdaCallbackDispatcher(std::function<double(int, double, double, double)> dispatcher)
  {
    g_dispatcher = std::move(dispatcher);
  }

  double InvokeLambdaCallback(const std::string& label, const micm::Conditions& conditions)
  {
    if (!g_dispatcher)
      return 0.0;

    auto it = g_label_to_id.find(label);
    if (it == g_label_to_id.end())
      return 0.0;

    return g_dispatcher(it->second, conditions.temperature_, conditions.pressure_, conditions.air_density_);
  }

}  // namespace musica
