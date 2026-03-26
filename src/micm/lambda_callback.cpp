// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/lambda_callback.hpp>

#include <map>

namespace musica
{
  namespace
  {
    std::map<std::string, std::function<double(const micm::Conditions&)>> g_callbacks;
  }

  void SetLambdaCallback(const std::string& label, std::function<double(const micm::Conditions&)> fn)
  {
    g_callbacks[label] = std::move(fn);
  }

  double InvokeLambdaCallback(const std::string& label, const micm::Conditions& conditions)
  {
    auto it = g_callbacks.find(label);
    if (it == g_callbacks.end())
      return 0.0;
    return it->second(conditions);
  }

}  // namespace musica
