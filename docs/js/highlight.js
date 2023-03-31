console.log(hljs)

hljs.registerLanguage("nest", hljs => {
  const kw = {
    $pattern: /[?..;><#!:=>]+|\w+/,
    keyword: ["?..", "..?", "...", "..", ";", "??", "?!", "?", ":", "=>"],
    built_in: ["_vars_", "_globals_", "_cwd_", "_args_", ">>>", "<<<", "!!", "::"],
    literal: ["true", "false", "null"],
    type: ["Int", "Real", "Byte", "Map", "Array", "Vector", "Iter", "Func", "Null", "IOFile", "Str", "Type"]
  };
  const string_escape = {
    begin: "\\\\([abefnrtv\\\\\"']|x[0-9a-fA-F]{2}|u[0-9a-fA-F]{4}|U[0-9a-fA-F]{8}|[0-7]{1,3})"
  };
  const double_quote_string = {
    className: "string",
    begin: '"',
    end: '"',
    contains: [string_escape]
  };
  const single_quote_string = {
    className: "string",
    begin: "'",
    end: "'",
    illegal: '\\n',
    contains: [string_escape]
  };
  const line_comment = {
    className: "comment",
    begin: "--",
    end: "[^\\\\]$",
    illegal: "\\n"
  };
  const multiline_comment = {
    className: "comment",
    begin: "-/",
    end: "/-"
  };
  const numbers = {
    className: "number",
    begin: "[+-]?(0[bB][01][01_]*([bB]|(?!\\d))|0[oO][0-7][0-7_]*([bB]|(?!\\d))|0[xXhH][0-9a-fA-F][0-9a-fA-F_]*(?![g-zG-Z])|\\d[\\d_]*\\.\\d[\\d_]*([eE][+-]?\\d[\\d_]*)?|\\d[\\d_]*[bB]?)",
    relevance: 0
  };
  const func_def = {
    className: "keyword",
    begin: "(##|#\\s*[A-Za-z_]\\w*)",
    end: "=>|(?=\\[)",
    relevance: 0,
    contains: [{
      begin: "[A-Za-z_]\\w*",
      className: "params"
    }]
  };
  const func_call = {
    className: "keyword",
    begin: "@|@@",
    end: "[^A-Za-z0-9_.]",
    excludeEnd: !0,
    relevance: 10,
    contains: [{
      begin: "[A-Za-z_]\\w*",
      end: "\\.|\\s|[^A-Za-z0-9_]",
      excludeEnd: !0,
      className: "title"
    }]
  };
  const match_statement_kw = {
    begin: "\\|>",
    className: "keyword"
  };
  const import_op_kw = {
    begin: "\\|#\\|",
    className: "built_in"
  };
  return {
    name: "Nest",
    keywords: kw,
    contains: [
      func_def,
      line_comment,
      match_statement_kw,
      func_call,
      import_op_kw,
      multiline_comment,
      double_quote_string,
      single_quote_string,
      numbers
    ]
  }
});

hljs.registerLanguage("better-c", (() => {
    "use strict";

    function e(e) {
        return ((...e) => e.map((e => (e => e ? "string" == typeof e ? e : e.source : null)(e))).join(""))("(", e, ")?")
    }
    return t => {
        const title_mode = {
          begin: "[A-Za-z_]\\w*",
          className: "title",
          relevance: 0
        }
        const n = t.COMMENT("//", "$", {
                contains: [{
                    begin: /\\\n/
                }]
            }),
            r = "[a-zA-Z_]\\w*::",
            a = "(decltype\\(auto\\)|" + e(r) + "[a-zA-Z_]\\w*" + e("<[^<>]+>") + ")",
            i = {
                className: "keyword",
                begin: "\\b[a-z\\d_]*_t\\b"
            },
            s = {
                className: "string",
                variants: [{
                    begin: '(u8?|U|L)?"',
                    end: '"',
                    illegal: "\\n",
                    contains: [t.BACKSLASH_ESCAPE]
                }, {
                    begin: "(u8?|U|L)?'(\\\\(x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4,8}|[0-7]{3}|\\S)|.)",
                    end: "'",
                    illegal: "."
                }, t.END_SAME_AS_BEGIN({
                    begin: /(?:u8?|U|L)?R"([^()\\ ]{0,16})\(/,
                    end: /\)([^()\\ ]{0,16})"/
                })]
            },
            o = {
                className: "number",
                variants: [{
                    begin: "\\b(0b[01']+)"
                }, {
                    begin: "(-?)\\b([\\d']+(\\.[\\d']*)?|\\.[\\d']+)((ll|LL|l|L)(u|U)?|(u|U)(ll|LL|l|L)?|f|F|b|B)"
                }, {
                    begin: "(-?)(\\b0[xX][a-fA-F0-9']+|(\\b[\\d']+(\\.[\\d']*)?|\\.[\\d']+)([eE][-+]?[\\d']+)?)"
                }],
                relevance: 0
            },
            c = {
                className: "meta",
                begin: /#\s*[a-z]+\b/,
                end: /$/,
                keywords: {
                    "meta-keyword": "if else elif endif define undef warning error line pragma _Pragma ifdef ifndef include"
                },
                contains: [{
                    begin: /\\\n/,
                    relevance: 0
                }, t.inherit(s, {
                    className: "meta-string"
                }), {
                    className: "meta-string",
                    begin: /<.*?>/
                }, n, t.C_BLOCK_COMMENT_MODE]
            },
            l = {
                className: "title",
                begin: e(r) + "[a-zA-Z_]\w*",
                relevance: 0
            },
            d = e(r) + "[a-zA-Z_]\w*" + "\\s*\\(",
            u = {
                keyword: "i8 u8 i16 u16 i32 u32 i64 u64 f32 f64 usize isize int float while private char char8_t char16_t char32_t catch import module export virtual operator sizeof dynamic_cast|10 typedef const_cast|10 const for static_cast|10 union namespace unsigned long volatile static protected bool template mutable if public friend do goto auto void enum else break extern using asm case typeid wchar_t short reinterpret_cast|10 default double register explicit signed typename try this switch continue inline delete alignas alignof constexpr consteval constinit decltype concept co_await co_return co_yield requires noexcept static_assert thread_local restrict final override atomic_bool atomic_char atomic_schar atomic_uchar atomic_short atomic_ushort atomic_int atomic_uint atomic_long atomic_ulong atomic_llong atomic_ullong new throw return and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq",
                built_in: "std string wstring cin cout cerr clog stdin stdout stderr stringstream istringstream ostringstream auto_ptr deque list queue stack vector map set pair bitset multiset multimap unordered_set unordered_map unordered_multiset unordered_multimap priority_queue make_pair array shared_ptr abort terminate abs acos asin atan2 atan calloc ceil cosh cos exit exp fabs floor fmod fprintf fputs free frexp fscanf future isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit tolower toupper labs ldexp log10 log malloc realloc memchr memcmp memcpy memset modf pow printf putchar puts scanf sinh sin snprintf sprintf sqrt sscanf strcat strchr strcmp strcpy strcspn strlen strncat strncmp strncpy strpbrk strrchr strspn strstr tanh tan vfprintf vprintf vsprintf endl initializer_list unique_ptr _Bool complex _Complex imaginary _Imaginary",
                literal: "true false nullptr NULL"
            },
            nest_things = {
              className: "built_in",
              begin: "_?(Nst|nst|NST)_\\w+",
              relevance: 0
            },
            m = [c, i, n, t.C_BLOCK_COMMENT_MODE, o, s],
            p = {
                variants: [{
                    begin: /=/,
                    end: /;/
                }, {
                    begin: /\(/,
                    end: /\)/
                }, {
                    beginKeywords: "new throw return else",
                    end: /;/
                }],
                keywords: u,
                contains: m.concat([nest_things, {
                    begin: /\(/,
                    end: /\)/,
                    keywords: u,
                    contains: m.concat(["self"]),
                    relevance: 0
                }]),
                relevance: 0
            },
            _ = {
                className: "function",
                begin: "(" + a + "[\\*&\\s]+)+" + d,
                returnBegin: !0,
                end: /[{;=]/,
                excludeEnd: !0,
                keywords: u,
                illegal: /[^\w\s\*&:<>.]/,
                contains: [nest_things, {
                    begin: "decltype\\(auto\\)",
                    keywords: u,
                    relevance: 0,
                    contains: [nest_things]
                }, {
                    begin: d,
                    returnBegin: !0,
                    contains: [{
                        className: "title",
                        begin: e(r) + "[a-zA-Z_]\w*",
                        relevance: 0
                    }],
                    relevance: 0
                }, {
                    className: "params",
                    begin: /\(/,
                    end: /\)/,
                    keywords: u,
                    relevance: 0,
                    contains: [nest_things, n, t.C_BLOCK_COMMENT_MODE, s, o, i, {
                        begin: /\(/,
                        end: /\)/,
                        keywords: u,
                        relevance: 0,
                        contains: [nest_things, "self", n, t.C_BLOCK_COMMENT_MODE, s, o, i]
                    }]
                }, i, n, t.C_BLOCK_COMMENT_MODE, c]
            };
        return {
            name: "C",
            aliases: ["h"],
            keywords: u,
            disableAutodetect: !0,
            illegal: "</",
            contains: [].concat(nest_things, p, _, m, [c, {
                begin: "\\b(deque|list|queue|priority_queue|pair|stack|vector|map|set|bitset|multiset|multimap|unordered_map|unordered_set|unordered_multiset|unordered_multimap|array)\\s*<",
                end: ">",
                keywords: u,
                contains: [nest_things, "self", i]
            }, {
                begin: "[a-zA-Z_]\w*" + "::",
                keywords: u,
                contains: [nest_things]
            }, {
                className: "class",
                beginKeywords: "enum class struct union",
                end: /[{;:<>=]/,
                contains: [{
                    beginKeywords: "final class struct"
                }, title_mode]
            }]),
            exports: {
                preprocessor: c,
                strings: s,
                keywords: u,
                contains: [nest_things]
            }
        }
    }
})());

document$.subscribe(() => {
  hljs.highlightAll()
})
