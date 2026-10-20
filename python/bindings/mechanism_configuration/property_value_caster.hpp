// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 type caster for mechanism_configuration::types::PropertyValue, the
// JSON-like value that holds an unknown (`__`-prefixed) property. Python None,
// bool, int, float, str, list/tuple, and dict convert in both directions, so
// `other_properties` is a plain dict whose values keep their type and structure.
//
// Include this header in every translation unit that binds a type with
// `unknown_properties`, before any use of the type.
#pragma once

#include <mechanism_configuration/types/property_value.hpp>

#include <pybind11/pybind11.h>

#include <cstdint>
#include <optional>

namespace pybind11::detail
{
  template<>
  struct type_caster<mechanism_configuration::types::PropertyValue>
  {
    using PropertyValue = mechanism_configuration::types::PropertyValue;

    PYBIND11_TYPE_CASTER(PropertyValue, const_name("object"));

    bool load(handle src, bool /*convert*/)
    {
      auto converted = ToPropertyValue(src);
      if (!converted)
        return false;
      value = std::move(*converted);
      return true;
    }

    static handle cast(const PropertyValue& src, return_value_policy /*policy*/, handle /*parent*/)
    {
      return ToPython(src).release();
    }

   private:
    static std::optional<PropertyValue> ToPropertyValue(pybind11::handle src)
    {
      if (src.is_none())
        return PropertyValue(nullptr);
      // bool is a subclass of int in Python, so test it first
      if (pybind11::isinstance<pybind11::bool_>(src))
        return PropertyValue(src.cast<bool>());
      if (pybind11::isinstance<pybind11::int_>(src))
      {
        try
        {
          return PropertyValue(src.cast<std::int64_t>());
        }
        catch (const pybind11::cast_error&)
        {
          // Too large for a 64-bit integer
          return PropertyValue(src.cast<double>());
        }
      }
      if (pybind11::isinstance<pybind11::float_>(src))
        return PropertyValue(src.cast<double>());
      if (pybind11::isinstance<pybind11::str>(src))
        return PropertyValue(src.cast<std::string>());
      if (pybind11::isinstance<pybind11::list>(src) || pybind11::isinstance<pybind11::tuple>(src))
      {
        PropertyValue::Array array;
        for (pybind11::handle item : pybind11::reinterpret_borrow<pybind11::sequence>(src))
        {
          auto converted = ToPropertyValue(item);
          if (!converted)
            return std::nullopt;
          array.push_back(std::move(*converted));
        }
        return PropertyValue(std::move(array));
      }
      if (pybind11::isinstance<pybind11::dict>(src))
      {
        PropertyValue::Object map;
        for (auto [key, item] : pybind11::reinterpret_borrow<pybind11::dict>(src))
        {
          auto converted = ToPropertyValue(item);
          if (!converted)
            return std::nullopt;
          map.emplace_back(pybind11::str(key).cast<std::string>(), std::move(*converted));
        }
        return PropertyValue(std::move(map));
      }
      return std::nullopt;
    }

    static pybind11::object ToPython(const PropertyValue& v)
    {
      if (v.IsNull())
        return pybind11::none();
      if (v.IsBool())
        return pybind11::bool_(v.AsBool());
      if (v.IsInt())
        return pybind11::int_(v.AsInt());
      if (v.IsDouble())
        return pybind11::float_(v.AsDouble());
      if (v.IsString())
        return pybind11::str(v.AsString());
      if (v.IsArray())
      {
        pybind11::list result;
        for (const auto& item : v.AsArray())
          result.append(ToPython(item));
        return std::move(result);
      }
      pybind11::dict result;
      for (const auto& [key, item] : v.AsObject())
        result[pybind11::str(key)] = ToPython(item);
      return std::move(result);
    }
  };
}  // namespace pybind11::detail
