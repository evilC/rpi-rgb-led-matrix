---
description: "Use when contributing C/C++ code in this repo."
applyTo: "include/**/*.{h},lib/**/*.{cc,c,h},examples-api-use/**/*.{cc,c},utils/**/*.{cc,c}"
---

# C/C++ Contribution Guidelines

This repository's implementation is primarily C++ with some C interfaces. These rules are for AI-assisted and manual contributions to native code.

## Core Rules

- Follow local style in neighboring files before introducing new patterns.
- Keep interface and implementation separated: headers for declarations, source files for logic.
- Minimize public API churn and preserve backward compatibility unless intentionally versioned.
- Prefer additive changes over behavior-changing rewrites when extending existing features.
- Keep performance-sensitive operations in native code paths.

## Header and Source Separation

- Put exported declarations in include/ headers.
- Keep implementation details and helpers in lib/ source files.
- Avoid introducing public API only in .cc files.
- If another component must call it, declare it in a header first.

## Native API Design

- Keep naming consistent with existing repo conventions.
- Keep ownership and lifetime clear at API boundaries.
- Prefer simple, explicit signatures over implicit behavior.
- Document non-obvious constraints near declarations.

## Error Handling and Stability

- Return predictable error values from low-level APIs.
- Keep failure modes explicit and avoid hidden side effects.
- Validate inputs at boundary points where malformed data can crash.
- Preserve existing on-disk and wire-format compatibility unless intentionally introducing a new versioned format.

## Performance Boundaries

- Keep heavy pixel and frame processing in C/C++.
- Prefer bulk operations and preprocessed pathways over per-pixel high-level loops.
- Avoid avoidable allocations or conversions in frame-hot paths.

## Style and Formatting

- Follow .editorconfig: 2-space indentation for .h and .cc files, LF line endings.
- Keep include style and ordering consistent with nearby files.
- Keep comments concise and focused on non-obvious behavior.

## PR Checklist

- Headers and sources are separated cleanly.
- New public native APIs are declared in headers.
- Ownership and lifetime are explicit and symmetric.
- Existing behavior and file format compatibility are preserved unless intentionally versioned.
