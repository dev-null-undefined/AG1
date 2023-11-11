#ifndef __PROGTEST__

#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>

#endif

#define __PROGTEST__

#include "main.cpp"

#undef __PROGTEST__


#ifndef __PROGTEST__
// MARK: "Dark magic"


// region Dark magic
////////////////// Dark magic, ignore ////////////////////////

template<typename T>
auto quote(const T &t) { return t; }

std::string quote(const std::string &s) {
    std::string ret = "\"";
    for (char c: s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")"\
        << " (rhs: " << quote(_b) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

#define CHECK_ALL(expr, ...) do { \
    std::array _arr = { __VA_ARGS__ }; \
    for (size_t _i = 0; _i < _arr.size(); _i++) \
      CHECK_((expr)(_i), _arr[_i], STR(expr) "(" << _i << ")", _arr[_i]); \
  } while (0)

#define CHECK_EX(expr, ex) do { \
    try { \
      (expr); \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but no exception was raised." << std::endl; \
    } catch (const ex&) { ok++; \
    } catch (...) { \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but got different exception." << std::endl; \
    } \
  } while (0)


#define CHECK_EX_C(expr, ex, context) do { \
    try { \
      (expr); \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but no exception was raised." << std::endl; \
    } catch (const ex&) { ok++; \
    } catch (...) { \
      fail++;           \
      std::cout << "Line " << __LINE__ << ": Expected " << STR(expr) << (context) << \
        " to throw " #ex " but got different exception." << std::endl; \
    } \
  } while (0)

////////////////// End of dark magic ////////////////////////
//endregion
#include <csignal>

struct X {
    X operator++(int) {
        auto cpy = X(*this);
        // stop on each fail
        raise(SIGTRAP);
        value++;
        return cpy;
    }

    operator int() {
        return value;
    }

    int value = 0;
};

namespace reference {

    struct TextEditorBackend {
        void assertIndex(size_t i, size_t max) const {
            if (i > max) {
                throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                        std::to_string(max - 1));
            }
        }

        void assertIndex(size_t i) const {
            assertIndex(i, size());
        }


        void assertStrictIndex(size_t i, size_t max) const {
            if (i >= max) {
                throw std::out_of_range("Index out of range " + std::to_string(i) + " maximum is " +
                                        std::to_string(size() - 1));
            }
        }

        void assertStrictIndex(size_t i) const {
            assertStrictIndex(i, size());
        }

        TextEditorBackend(const std::string &text) {
            s = text;
        }

        size_t size() const {
            return s.size();
        }

        size_t lines() const {
            size_t lines = 1;
            for (char c: s) {
                if (c == '\n') {
                    ++lines;
                }
            }
            return lines;
        }

        char at(size_t i) const {
            assertStrictIndex(i);
            return s[i];
        }

        void edit(size_t i, char c) {
            assertStrictIndex(i);
            s[i] = c;
        }

        void insert(size_t i, char c) {
            assertIndex(i);
            s.insert(i, 1, c);
        }

        void erase(size_t i) {
            assertStrictIndex(i);
            s.erase(i, 1);
        }

        size_t line_start(size_t r) const {
            assertIndex(r, lines() - 1);
            if (r == 0) {
                return 0;
            }
            size_t i = 0;
            for (size_t j = 0; j < r; ++j) {
                i = s.find('\n', i) + 1;
            }
            return i;
        }

        size_t line_length(size_t r) const {
            assertIndex(r, lines() - 1);
            if (r == lines() - 1) {
                return size() - line_start(r);
            }
            return line_start(r + 1) - line_start(r);
        }

        size_t char_to_line(size_t i) const {
            assertIndex(i);
            size_t r = 0;
            for (size_t j = 0; j < i; ++j) {
                if (s[j] == '\n') {
                    ++r;
                }
            }
            return r;
        }

        std::string s;
    };
}

#include <functional>

std::string text(const auto &t) {
    std::string ret;
    for (size_t i = 0; i < t.size(); i++) ret.push_back(t.at(i));
    return ret;
}

void test1(int &ok, X &fail) {
    TextEditorBackend s("123\n456\n789");
    CHECK(s.size(), 11);
    CHECK(text(s), "123\n456\n789");
    CHECK(s.lines(), 3);
    CHECK_ALL(s.char_to_line, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2);
    CHECK_ALL(s.line_start, 0, 4, 8);
    CHECK_ALL(s.line_length, 4, 4, 3);
}

void test2(int &ok, X &fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK(t.size(), 12);
    CHECK(text(t), "123\n456\n789\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.char_to_line, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2);
    CHECK_ALL(t.line_start, 0, 4, 8, 12);
    CHECK_ALL(t.line_length, 4, 4, 4, 0);
}

void test3(int &ok, X &fail) {
    TextEditorBackend t("asdfasdfasdf");

    CHECK(t.size(), 12);
    CHECK(text(t), "asdfasdfasdf");
    CHECK(t.lines(), 1);
    CHECK_ALL(t.char_to_line, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CHECK(t.line_start(0), 0);
    CHECK(t.line_length(0), 12);


    TextEditorBackend t2("\nasdfasdfasdf");

    CHECK(t2.size(), 13);
    CHECK(text(t2), "\nasdfasdfasdf");
    CHECK(t2.lines(), 2);
    CHECK_ALL(t2.char_to_line, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    CHECK(t2.line_start(0), 0);
    CHECK(t2.line_length(0), 1);
    CHECK(t2.line_start(1), 1);
    CHECK(t2.line_length(1), 12);

    TextEditorBackend t3("");

    CHECK(t3.size(), 0);
    CHECK(t3.lines(), 1);
    CHECK_ALL(t3.char_to_line, 0);
    CHECK(t3.line_start(0), 0);
    CHECK(t3.line_length(0), 0);

    t.insert(0, '\n');
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasdfasdf");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);

    t.insert(4, '\n');
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasdfasdf");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.insert(t.size(), '\n');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdf\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.line_start, 0, 1, 5, 15);

    t.edit(t.size() - 1, 'H');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(8);
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(4);
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasfasdfH");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);
}

void test_ex(int &ok, X &fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK_EX(t.at(12), std::out_of_range);

    CHECK_EX(t.insert(13, 'a'), std::out_of_range);
    CHECK_EX(t.edit(12, 'x'), std::out_of_range);
    CHECK_EX(t.erase(12), std::out_of_range);

    CHECK_EX(t.line_start(4), std::out_of_range);
    CHECK_EX(t.line_start(40), std::out_of_range);
    CHECK_EX(t.line_length(4), std::out_of_range);
    CHECK_EX(t.line_length(6), std::out_of_range);
    CHECK_EX(t.char_to_line(12), std::out_of_range);
    CHECK_EX(t.char_to_line(25), std::out_of_range);


    TextEditorBackend t3("");
    CHECK_EX(t3.line_start(1), std::out_of_range);
    CHECK_EX(t3.line_length(1), std::out_of_range);
}

int RNG() {
    return rand();
}

// region TEST_ALL makro
#define TEST_ALL()\
CHECK(text(sol), text(ref));\
CHECK(sol.size(), ref.size());\
CHECK(sol.lines(), ref.lines());\
CHECK_EX(sol.at(sol.size()), std::out_of_range);\
CHECK_EX(sol.at(sol.size() + 1), std::out_of_range);\
\
for(size_t i = 0; i < ref.lines() + 10; i ++) {\
try {\
auto line_length_ref = ref.line_length(i);\
try {\
auto line_length = sol.line_length(i);\
if(line_length != line_length_ref)     std::cout << "Index: " << i << " failed line length" << std::endl;              \
CHECK(line_length, line_length_ref);            \
} catch (const std::out_of_range &e) {\
fail++;\
std::cout << "Line " << __LINE__ << ": Throwed after line_lenght (" << i << ") but should not throw." << std::endl;\
}\
} catch (const std::out_of_range &e) {\
CHECK_EX(sol.line_length(i), std::out_of_range);\
return;\
}\
try {\
auto line_start_ref = ref.line_start(i);\
try {\
auto line_start = sol.line_start(i);\
if(line_start != line_start_ref)     std::cout << "Index: " << i << " failed line start" << std::endl;              \
CHECK(line_start, line_start_ref);\
} catch (const std::out_of_range &e) {\
fail++;\
std::cout << "Line " << __LINE__ << ": Throwed after line_start (" << i << ") but should not throw." << std::endl;\
}\
} catch (const std::out_of_range &e) {\
CHECK_EX(sol.line_start(i), std::out_of_range);\
return;\
}\
}\
\
for(size_t i = 0; i < ref.size() + 10; i ++) {\
try {\
auto char_to_line_ref = ref.char_to_line(i);\
try {\
auto char_to_line = sol.char_to_line(i);\
CHECK(char_to_line_ref, char_to_line);\
} catch (const std::out_of_range &e) {\
fail++;\
std::cout << "Line " << __LINE__ << ": Throwed after char_to_line (" << i << ") but should not throw." << std::endl;\
}\
} catch (const std::out_of_range &e) {\
CHECK_EX(sol.char_to_line(i), std::out_of_range);\
return;\
}\
try {\
auto at_ref = ref.at(i);\
try {\
auto at = sol.at(i);\
CHECK(at_ref, at);\
} catch (const std::out_of_range &e) {\
fail++;\
std::cout << "Line " << __LINE__ << ": Throwed after at (" << i << ") but should not throw." << std::endl;\
}\
} catch (const std::out_of_range &e) {\
CHECK_EX(sol.at(i), std::out_of_range);\
return;\
}\
}                 \

// endregion



int rngPos(int max, int overShoot = 20) {
    return RNG() % (max + overShoot);
}

char rngChar() {
    return char(RNG() % 10 == 0 ? '\n' : ('a' + char(RNG() % 26)));
}

#define CHECK_OP_(expr, expr_str, ctx) do { \
try {\
ref.expr;\
try {\
sol.expr;\
} catch (const std::out_of_range &e) {\
fail++;\
std::cout << "Line " << __LINE__ << ": Throwed after " expr_str "" << ctx\
<< " but should not throw." << std::endl;\
}\
} catch (const std::out_of_range &e) {\
CHECK_EX_C(sol.expr, std::out_of_range, ctx);\
return;\
}                                          \
} while (0);

#define CHECK_OP(expr, ctx) CHECK_OP_(expr, #expr,ctx)

void print(const TextEditorBackend &t) {
    std::string s = text(t);
    std::replace(s.begin(), s.end(), '\n', '*');
    std::cout << s << std::endl;
}

void insert(int &ok, X &fail, TextEditorBackend &sol, reference::TextEditorBackend &ref) {
    int pos = rngPos(ref.size(), 20);
    char c = rngChar();
    CHECK_OP(insert(pos, c), " (" + std::to_string(pos) + ", " + quote(c) + ")")
    print(sol);
    TEST_ALL()
}

void erase(int &ok, X &fail, TextEditorBackend &sol, reference::TextEditorBackend &ref) {
    int pos = rngPos(ref.size(), 20);
    CHECK_OP(erase(pos), " (" + std::to_string(pos) + ")")
    print(sol);
    TEST_ALL()
}

void edit(int &ok, X &fail, TextEditorBackend &sol, reference::TextEditorBackend &ref) {
    int pos = rngPos(ref.size(), 20);
    char c = rngChar();
    CHECK_OP(edit(pos, c), " (" + std::to_string(pos) + ", " + quote(c) + ")")
    print(sol);
    TEST_ALL()
}

std::vector<std::function<void(int &, X &, TextEditorBackend &,
                               reference::TextEditorBackend &)>> referenceOperation = {
        insert, erase, edit
};

void test_vs_ref(int &ok, X &fail) {
    for (int i = 0; i < 100; ++i) {
        std::string s;
        for (int j = 0; j < RNG() % 20; ++j) {
            if (RNG() % 3 == 0)
                s.push_back('\n');
            else
                s.push_back(char('a' + (RNG() % 26)));
        }

        TextEditorBackend t(s);
        reference::TextEditorBackend t2(s);

        for (int j = 0; j < 100; ++j) {
            referenceOperation[RNG() % referenceOperation.size()](ok, fail, t, t2);
        }
    }
}


std::vector<std::function<void(int &, X &)>> tests = {
        test1,
        test2,
        test3,
        test_ex,
        test_vs_ref
};

int main() {
    int ok = 0;
    X fail;
    for (auto &test: tests) test(ok, fail);

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


