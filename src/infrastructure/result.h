#ifndef RESULT_H
#define RESULT_H

#include <cstdint>
#include <cstring>

/**
 * Result<T> — Type for Error Handling (Rust-style Result)
 *
 * Ogni operazione ritorna Result<T> che contiene:
 * - Success: valore T
 * - Failure: error code e messaggio
 *
 * Vantaggi:
 * ✅ Elimina bool magic (-1 per errori, ecc)
 * ✅ Carry error message (not just true/false)
 * ✅ Composable (chain operations)
 * ✅ Type-safe error handling
 *
 * Uso:
 *   Result<float> temp = repository.readTemperature();
 *   if (temp.isOk()) {
 *     float value = temp.value();
 *   } else {
 *     logger.error(temp.errorMessage());
 *   }
 */

enum class ErrorCode : uint8_t {
  OK = 0,
  UNKNOWN_ERROR = 1,
  VALIDATION_FAILED = 2,
  REPOSITORY_ERROR = 3,
  TIMEOUT = 4,
  INVALID_PARAMETER = 5,
  NOT_FOUND = 6,
  STORAGE_ERROR = 7,
  COMMUNICATION_ERROR = 8,
};

template <typename T>
class Result {
private:
  bool success;
  T value_;
  ErrorCode errorCode;
  char errorMsg[128];

public:
  // ✅ Success constructor
  static Result<T> Ok(const T &val) {
    Result<T> r;
    r.success = true;
    r.value_ = val;
    r.errorCode = ErrorCode::OK;
    r.errorMsg[0] = '\0';
    return r;
  }

  // ❌ Error constructor
  static Result<T> Err(ErrorCode code, const char *message = "") {
    Result<T> r;
    r.success = false;
    r.errorCode = code;
    strncpy(r.errorMsg, message, sizeof(r.errorMsg) - 1);
    r.errorMsg[sizeof(r.errorMsg) - 1] = '\0';
    return r;
  }

  // Query methods
  bool isOk() const {
    return success;
  }

  bool isErr() const {
    return !success;
  }

  // Get value (only if Ok)
  T value() const {
    return value_;
  }

  // Get error info
  ErrorCode error() const {
    return errorCode;
  }

  const char *errorMessage() const {
    return errorMsg;
  }

  // Utility: get error code as int for logging
  int errorCodeInt() const {
    return (int)errorCode;
  }
};

// Specialization for void (for operations that don't return a value)
template <>
class Result<void> {
private:
  bool success;
  ErrorCode errorCode;
  char errorMsg[128];

public:
  static Result<void> Ok() {
    Result<void> r;
    r.success = true;
    r.errorCode = ErrorCode::OK;
    r.errorMsg[0] = '\0';
    return r;
  }

  static Result<void> Err(ErrorCode code, const char *message = "") {
    Result<void> r;
    r.success = false;
    r.errorCode = code;
    strncpy(r.errorMsg, message, sizeof(r.errorMsg) - 1);
    r.errorMsg[sizeof(r.errorMsg) - 1] = '\0';
    return r;
  }

  bool isOk() const {
    return success;
  }

  bool isErr() const {
    return !success;
  }

  ErrorCode error() const {
    return errorCode;
  }

  const char *errorMessage() const {
    return errorMsg;
  }

  int errorCodeInt() const {
    return (int)errorCode;
  }
};

#endif
