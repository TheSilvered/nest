# Nest tests

Index:
- 🟢: Well tested
- 🟡: Needs improvement or checking
- 🔴: Poorly tested or missing

The current state of tests:

- 🟢 Lexer
- 🟢 Parser
- 🟢 Optimizer
- 🔴 Runtime
  - 🟢 Runtime Errors
  - 🔴 Runtime Evaluation
- 🟡 Objects
  - 🟡 `Array`
  - 🟢 `Bool`
  - 🟢 `Byte`
  - 🟡 `Func`
  - 🟢 `Int`
  - 🟢 `IOFile`
  - 🟢 `Iter`
  - 🟢 `Map`
  - 🟢 `Null`
  - 🟡 `Real`
  - 🟡 `Str`
  - 🟢 `Type`
  - 🟡 `Vector`
- 🟡 Standard library
  - 🟢 `stdco.nest`
  - 🟡 `stdcodecs.nest`
  - 🟡 `stderr.nest`
  - 🟢 `stdfs.nest`
  - 🟢 `stdio.nest`
  - 🟡 `stditutil.nest`
  - 🟢 `stdjson.nest`
  - 🟡 `stdmath.nest`
  - 🟡 `stdrand.nest`
  - 🟢 `stdsequtil.nest`
  - 🟡 `stdsutil.nest`
  - 🟡 `stdsys.nest`
  - 🟢 `stdtime.nest`
- 🔴 C tests
