---
description: "Use when exposing native APIs to language bindings (C#, Python, or other languages)."
applyTo: "include/*-c.h,lib/*-c.cc,bindings/**/*.cs,bindings/**/README.md"
---

# Binding Interop Addendum

Use these rules in addition to the base C/C++ conventions.

## C ABI Rules

- Add or update declarations in include/*-c.h.
- Implement wrappers in lib/*-c.cc using extern "C".
- Use opaque handles for C-facing object ownership where appropriate.
- Keep C signatures C-friendly: plain integers, pointers, structs, and C strings.
- Do not expose C++-only types in C headers, such as std::string, references, templates, or exceptions.

## Ownership and Lifetime

- Every create/open function needs a matching destroy/close function.
- Document ownership transfer at the API boundary.
- Do null checks in wrappers where a bad pointer would otherwise crash.
- Keep object lifetime rules simple and consistent across wrappers.

## C# Interop Notes

- C# extern symbols should map 1:1 to native C wrapper symbols.
- Use blittable types and explicit marshaling where needed.
- Keep managed wrappers lightweight and move expensive loops into native code.

## PR Checklist

- Binding-facing APIs use C ABI wrappers under include/*-c.h and lib/*-c.cc.
- C API declarations and implementations are kept in sync.
- C# extern signatures and marshaling match native signatures.
- Backward compatibility is preserved unless intentionally versioned.
