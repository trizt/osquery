/**
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>

#include <osquery/error.h>

#include <boost/blank.hpp>
#include <boost/variant.hpp>

/**
 * Utility class that should be used in function that return
 * either error or value. Expected enforce developer to test for success and
 * check error if any.
 *
 * enum class TestError { SomeError = 1, AnotherError = 2 };
 * Expected<std::string, TestError> function() {
 *   if (test) {
 *    return "ok";
 *   } else {
 *    if (first_error) {
 *      return Error<TestError>(TestError::SomeError, "some error message");
 *    } else {
 *      return createError(TestError::SomeError, "one more error message");
 *    }
 *   }
 * }
 *
 * Expected:
 * ExpectedUnique<PlatformProcess, TestError> function() {
 *   if (test) {
 *    return std::make_unique<PlatformProcess>(pid);
 *   } else {
 *    return createError(TestError::AnotherError, "something wrong");
 *   }
 * }
 *
 * auto result = function();
 * if (result) {
 *   ...use *result
 * } else {
 *   switch (result.getErrorCode()) {
 *     case TestError::SomeError:
 *        ...do something with it
 *        break;
 *     case TestError::AnotherError:
 *        ...do something with it
 *        break;
 *   }
 * }
 * @see osquery/core/tests/exptected_tests.cpp for more examples
 *
 * Rvalue ref-qualified methods of unconditional access value or error are
 * explicitly deleted. As far as `osquery` does not have an exceptions we
 * definitely would like to avoid using unsafe way of getting either value or
 * error without a proper check in advance.
 */

namespace osquery {

template <typename ValueType_, typename ErrorCodeEnumType>
class Expected final {
 public:
  using ValueType = ValueType_;
  using ErrorType = Error<ErrorCodeEnumType>;
  using SelfType = Expected<ValueType, ErrorCodeEnumType>;

 public:
  Expected(ValueType value) : object_{std::move(value)} {}

  Expected(ErrorType error) : object_{std::move(error)} {}

  explicit Expected(ErrorCodeEnumType code, std::string message)
      : object_{ErrorType(code, message)} {}

  Expected(Expected&& other) = default;

  Expected() = delete;
  Expected(const Expected&) = delete;
  Expected(ErrorBase* error) = delete;

  Expected& operator=(Expected&& other) = default;
  Expected& operator=(const Expected& other) = delete;

  ~Expected() {
    assert(errorChecked_ && "Error was not checked");
  }

  static SelfType success(ValueType value) {
    return SelfType{std::move(value)};
  }

  static SelfType failure(std::string message) {
    auto defaultCode = ErrorCodeEnumType{};
    return SelfType(defaultCode, std::move(message));
  }

  static SelfType failure(ErrorCodeEnumType code, std::string message) {
    return SelfType(code, std::move(message));
  }

  ErrorType takeError() && = delete;
  ErrorType takeError() & {
    return std::move(boost::get<ErrorType>(object_));
  }

  const ErrorType& getError() const&& = delete;
  const ErrorType& getError() const& {
    return boost::get<ErrorType>(object_);
  }

  ErrorCodeEnumType getErrorCode() const&& = delete;
  ErrorCodeEnumType getErrorCode() const& {
    return getError().getErrorCode();
  }

  bool isError() const noexcept {
#ifndef NDEBUG
    errorChecked_ = true;
#endif
    return object_.which() == kErrorType_;
  }

  explicit operator bool() const noexcept {
    return !isError();
  }

  ValueType& get() && = delete;
  ValueType& get() & {
#ifndef NDEBUG
    assert(object_.which() == kValueType_ &&
           "Do not try to get value from Expected with error");
#endif
    return boost::get<ValueType>(object_);
  }

  const ValueType& get() const&& = delete;
  const ValueType& get() const& {
#ifndef NDEBUG
    assert(object_.which() == kValueType_ &&
           "Do not try to get value from Expected with error");
#endif
    return boost::get<ValueType>(object_);
  }

  const ValueType& getOr(const ValueType& defaultValue) const {
    if (isError()) {
      return defaultValue;
    }
    return boost::get<ValueType>(object_);
  }

  ValueType take() && = delete;
  ValueType take() & {
    return std::move(get());
  }

  template <typename ValueTypeUniversal = ValueType>
  typename std::enable_if<std::is_same<ValueTypeUniversal, ValueType>::value,
                          ValueType>::type
  takeOr(ValueTypeUniversal&& defaultValue) {
    if (isError()) {
      return std::forward<ValueTypeUniversal>(defaultValue);
    }
    return std::move(get());
  }

  ValueType* operator->() && = delete;
  ValueType* operator->() & {
    return &get();
  }

  const ValueType* operator->() const&& = delete;
  const ValueType* operator->() const& {
    return &get();
  }

  ValueType& operator*() && = delete;
  ValueType& operator*() & {
    return get();
  }

  const ValueType& operator*() const&& = delete;
  const ValueType& operator*() const& {
    return get();
  }

 private:
  static_assert(
      !std::is_pointer<ValueType>::value,
      "Please do not use raw pointers as expected value, "
      "use smart pointers instead. See CppCoreGuidelines for explanation. "
      "https://github.com/isocpp/CppCoreGuidelines/blob/master/"
      "CppCoreGuidelines.md#Rf-unique_ptr");
  static_assert(std::is_enum<ErrorCodeEnumType>::value,
                "ErrorCodeEnumType template parameter must be enum");

  boost::variant<ValueType, ErrorType> object_;
  enum ETypeId {
    kValueType_ = 0,
    kErrorType_ = 1,
  };
#ifndef NDEBUG
  mutable bool errorChecked_ = false;
#endif
};

template <typename ValueType, typename ErrorCodeEnumType>
using ExpectedShared = Expected<std::shared_ptr<ValueType>, ErrorCodeEnumType>;

template <typename ValueType, typename ErrorCodeEnumType>
using ExpectedUnique = Expected<std::unique_ptr<ValueType>, ErrorCodeEnumType>;

using Success = boost::blank;

template <typename ErrorCodeEnumType>
using ExpectedSuccess = Expected<Success, ErrorCodeEnumType>;

} // namespace osquery
