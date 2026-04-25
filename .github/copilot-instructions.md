# Code Review Instructions for Copilot

## Review Priorities

1. **Critical Issues** (Block merge):
   - Memory leaks or resource leaks
   - Security vulnerabilities (SQL injection, hardcoded secrets, unvalidated input)
   - Null/undefined pointer dereferences
   - Unhandled API errors
   - Race conditions

2. **High Priority** (Require fixes):
   - Memory safety issues (missing deletes, double frees)
   - Unchecked return values from system/API calls
   - Missing error handling for file I/O operations
   - Resource cleanup in all code paths (RAII pattern)
   - Proper error propagation up the call stack

3. **Medium Priority** (Consider fixes):
   - Cyclomatic complexity > 20 in single functions
   - Magic numbers without named constants
   - Missing const qualifiers where applicable
   - Inconsistent naming with codebase conventions

## Code Style Requirements

- Use RAII for all resource management (no manual new/delete pairs)
- Every function that can fail must return an error code or use exceptions
- Raw pointers should only be used for object lifetime management (owner transfers via std::unique_ptr)
- All public API functions require documentation comments
- Add assertions to validate preconditions and contract violations
- Use fixed-width integer types (qint32, quint64, etc.) for serialization boundaries

## Qt Framework Specific

- Use Qt types (QString, QByteArray, QVariant) for Qt API boundaries
- Prefer QCoreApplication::exit() over std::exit()
- Connect signals/slots using queued connections for cross-thread communication
- Always use qUtf8Printable() for logging Qt strings
- Use QMetaMethod::invoke() for dynamic calls

## Test Coverage

- New public functions require corresponding unit tests
- Focus review on error paths and edge cases in tests
- Mock external dependencies (file system, network, database)