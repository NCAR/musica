// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Mechanism configuration bindings for the Julia interface. Registered with the
// module via the self-registration registry (see registration.hpp).
#include "jlcxx/jlcxx.hpp"
#include "jlcxx/stl.hpp"
#include "registration.hpp"

#include <mechanism_configuration/parse.hpp>

#include <string>
#include <vector>

namespace
{
  void register_mechanism_configuration(jlcxx::Module& mod)
  {
    // Returns the errors as flat (code, message, path) triples. An empty path means no path.
    mod.method(
        "cpp_validate_mechanism_string",
        [](const std::string& config)
        {
          std::vector<std::string> triples;
          for (const auto& [code, message, path] : mechanism_configuration::ParseFromString(config).errors)
          {
            triples.push_back(mechanism_configuration::ErrorCodeToString(code));
            triples.push_back(message);
            triples.push_back(path.value_or(""));
          }
          return triples;
        });
  }

  [[maybe_unused]] const musica_julia::Registrar registrar(register_mechanism_configuration);
}  // namespace
