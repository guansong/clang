// RUN: %clang_cc1 -fsyntax-only -verify -Wno-array-bounds %s -fpascal-strings
// RUN: %clang_cc1 -fdiagnostics-parseable-fixits -x c++ %s 2>&1 -Wno-array-bounds -fpascal-strings | FileCheck %s

void consume(const char* c) {}
void consume(const unsigned char* c) {}
void consume(const wchar_t* c) {}
void consumeChar(char c) {}

enum MyEnum {
  kMySmallEnum = 1,
  kMyEnum = 5
};

enum OperatorOverloadEnum {
  kMyOperatorOverloadedEnum = 5
};

const char* operator+(const char* c, OperatorOverloadEnum e) {
  return "yo";
}

const char* operator+(OperatorOverloadEnum e, const char* c) {
  return "yo";
}

void f(int index) {
  // Should warn.
  // CHECK: fix-it:"{{.*}}":{31:11-31:11}:"&"
  // CHECK: fix-it:"{{.*}}":{31:17-31:18}:"["
  // CHECK: fix-it:"{{.*}}":{31:20-31:20}:"]"
  consume("foo" + 5);  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}
  consume("foo" + index);  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}
  consume("foo" + kMyEnum);  // expected-warning {{adding 'MyEnum' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}

  consume(5 + "foo");  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}
  consume(index + "foo");  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}
  consume(kMyEnum + "foo");  // expected-warning {{adding 'MyEnum' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}

  // FIXME: suggest replacing with "foo"[5]
  consumeChar(*("foo" + 5));  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}
  consumeChar(*(5 + "foo"));  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}

  consume(L"foo" + 5);  // expected-warning {{adding 'int' to a string does not append to the string}} expected-note {{use array indexing to silence this warning}}

  // Should not warn.
  consume(&("foo"[3]));
  consume(&("foo"[index]));
  consume(&("foo"[kMyEnum]));
  consume("foo" + kMySmallEnum);
  consume(kMySmallEnum + "foo");

  consume(L"foo" + 2);

  consume("foo" + 3);  // Points at the \0
  consume("foo" + 4);  // Points 1 past the \0, which is legal too.
  consume("\pfoo" + 4);  // Pascal strings don't have a trailing \0, but they
                         // have a leading length byte, so this is fine too.

  consume("foo" + kMyOperatorOverloadedEnum);
  consume(kMyOperatorOverloadedEnum + "foo");

  #define A "foo"
  #define B "bar"
  consume(A B + sizeof(A) - 1);
}

