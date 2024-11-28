const node = document.createElement("div");
node.setAttribute("data-ea-publisher", "");
const el = document.querySelector(".md-footer");
el.appendChild(node);

hljs.registerLanguage("nest", hljs => {
  const kw = {
    $pattern: /[?..;><#!:=>]+|\w+/,
    keyword: ["?..", "..?", "...", "..", ";", "??", "?!", "?", ":", "=>"],
    built_in: ["_vars_", "_globals_", "_args_", "_debug_", "_debug_arch_", ">>>", "<<<", "!!", "::"],
    literal: ["true", "false", "null"],
    type: ["Int", "Real", "Byte", "Bool", "Map", "Array", "Vector", "Iter", "Func", "Null", "IOFile", "Str", "Type"]
  };
  const string_escape = {
    begin: /\\([abefnrtv\\"']|x[0-9a-fA-F]{2}|u[0-9a-fA-F]{4}|U[0-9a-fA-F]{8}|[0-7]{1,3})/
  };
  const double_quote_string = {
    scope: "string",
    begin: '"',
    end: '"',
    contains: [string_escape]
  };
  const single_quote_string = {
    scope: "string",
    begin: "'",
    end: "'",
    illegal: '\\n',
    contains: [string_escape]
  };
  const line_comment = {
    scope: "comment",
    begin: "--",
    end: "[^\\\\]$",
    illegal: "\\n"
  };
  const multiline_comment = {
    scope: "comment",
    begin: "-/",
    end: "/-"
  };
  const numbers = {
    scope: "number",
    begin: /[+-]?(0[bB][01][01_]*([bB]|(?!\d))|0[oO][0-7][0-7_]*([bB]|(?!\d))|0[xXhH][0-9a-fA-F][0-9a-fA-F_]*(?![g-zG-Z])|\d[\d_]*\.\d[\d_]*([eE][+-]?\d[\d_]*)?|\d[\d_]*[bB]?)/,
    relevance: 0
  };
  const func_def = {
    scope: "keyword",
    begin: /(##|#\s*[A-Za-z_]\w*)/,
    end: /=>|(?=\[)/,
    relevance: 0,
    contains: [{
      begin: /[A-Za-z_]\w*/,
      scope: "params"
    }]
  };
  const func_call = {
    scope: "keyword",
    begin: /(\*|@)?@/,
    end: "[^A-Za-z0-9_.]",
    excludeEnd: true,
    relevance: 10,
    contains: [{
      begin: /[A-Za-z_]\w*(?!A-Za-z0-9_)/,
      scope: "title"
    }]
  };
  const match_statement_kw = {
    begin: "\\|>",
    scope: "keyword"
  };
  const import_op_kw = {
    begin: "\\|#\\|",
    scope: "built_in"
  };
  const identifier = {
    begin: /[A-Za-z_]\w*/,
    relevance: 0
  };
  const fix_kw1 = {
    begin: /true|false|null/,
    scope: "literal"
  };
  const fix_kw2 = {
    begin: /_vars_|_globals_|_args_|_debug_|_debug_arch_/,
    scope: "built_in"
  };
  const fix_kw3 = {
    begin: /Int|Real|Byte|Bool|Map|Array|Vector|Iter|Func|Null|IOFile|Str|Type/,
    scope: "built_in"
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
      fix_kw1,
      fix_kw2,
      fix_kw3,
      identifier,
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
      scope: "title",
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
        scope: "keyword",
        begin: "\\b[a-z\\d_]*_t\\b"
      },
      s = {
        scope: "string",
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
        scope: "number",
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
        scope: "meta",
        begin: /#\s*[a-z]+\b/,
        end: /$/,
        keywords: {
          "meta-keyword": "if else elif endif define undef warning error line pragma _Pragma ifdef ifndef include"
        },
        contains: [{
          begin: /\\\n/,
          relevance: 0
        }, t.inherit(s, {
          scope: "meta-string"
        }), {
          scope: "meta-string",
          begin: /<.*?>/
        }, n, t.C_BLOCK_COMMENT_MODE]
      },
      l = {
        scope: "title",
        begin: e(r) + "[a-zA-Z_]\w*",
        relevance: 0
      },
      d = e(r) + "[a-zA-Z_]\w*" + "\\s*\\(",
      u = {
        keyword: "i8 u8 i16 u16 i32 u32 i64 u64 f32 f64 usize isize int float while private char char8_t char16_t char32_t catch import module export virtual operator sizeof dynamic_cast|10 typedef const_cast|10 const for static_cast|10 union namespace unsigned long volatile static protected bool template mutable if public friend do goto auto void enum else break extern using asm case typeid wchar_t short reinterpret_cast|10 default double register explicit signed typename try this switch continue inline delete alignas alignof constexpr consteval constinit decltype concept co_await co_return co_yield requires noexcept static_assert thread_local restrict final override atomic_bool atomic_char atomic_schar atomic_uchar atomic_short atomic_ushort atomic_int atomic_uint atomic_long atomic_ulong atomic_llong atomic_ullong new throw return and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq",
        built_in: "std stdin stdout stderr stringstream istringstream ostringstream auto_ptr shared_ptr abort terminate abs acos asin atan2 atan calloc ceil cosh cos exit exp fabs floor fmod fprintf fputs free frexp fscanf future isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit tolower toupper labs ldexp log10 log malloc realloc memchr memcmp memcpy memset modf pow printf putchar puts scanf sinh sin snprintf sprintf sqrt sscanf strcat strchr strcmp strcpy strcspn strlen strncat strncmp strncpy strpbrk strrchr strspn strstr tanh tan vfprintf vprintf vsprintf endl initializer_list unique_ptr _Bool complex _Complex imaginary _Imaginary",
        literal: "true false nullptr NULL"
      },
      nest_things = {
        scope: "built_in",
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
        scope: "function",
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
            scope: "title",
            begin: e(r) + "[a-zA-Z_]\w*",
            relevance: 0
          }],
          relevance: 0
        }, {
          scope: "params",
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
        scope: "class",
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
