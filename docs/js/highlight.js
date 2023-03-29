console.log(hljs)

hljs.registerLanguage("nest", hljs => {
  const string_escape = {
    begin: "\\\\([abefnrtv\\\\\"']|x[0-9a-fA-F]{2}|u[0-9a-fA-F]{4}|U[0-9a-fA-F]{8}|[0-7]{1,3})"
  }
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
  }
  const multiline_comment = {
    className: "comment",
    begin: "-/",
    end: "/-"
  }
  const numbers = {
    className: "number",
    begin: "[+-]?(0[bB][01][01_]*([bB]|(?!\\d))|0[oO][0-7][0-7_]*([bB]|(?!\\d))|0[xXhH][0-9a-fA-F][0-9a-fA-F_]*(?![g-zG-Z])|\\d[\\d_]*\\.\\d[\\d_]*([eE][+-]?\\d[\\d_]*)?|\\d[\\d_]*[bB]?)"
  }
  const func_def = {
    className: "function",
    begin: "#",
    end: '\\[|=>',
    relevance: 0,
    contains: [{
      begin: "[A-Za-z_][A-Za-z_0-9]*",
      className: "param"
    }]
  }
  return {
    name: "Nest",
    keywords: {
        $pattern: /[\?\.\.;\|><#!]+|\w+/,
        keyword: ["?..", "..?", "...", "..", ";", "??", "?!", "?", ":", "#", "##"],
        built_in: ["_vars_", "_globals_", "_cwd_", "_args_", ">>>", "<<<", "|#|"],
        literal: ["true", "false", "null"],
        type: ["Int", "Real", "Byte", "Map", "Array", "Vector", "Iter", "Func", "Null", "IOFile", "Str", "Type", "Union"]
    },
    contains: [
      line_comment,
      multiline_comment,
      double_quote_string,
      single_quote_string,
      numbers,
      func_def
    ]
  }
});

document$.subscribe(() => {
  hljs.highlightAll()
})
