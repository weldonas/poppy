# Context-Free Grammar for Poppy

Terminal symbols: all tokens for the language.

Nonterminal symbols: $\{\text{program, defndecls, defn, type, optparams, stmts, params, param, stmt, semistmt, expr, cond, andcond, orcond, uncond, optelse, addexpr, multexpr, unexpr, optargs, args, vardec, varasst, ret, call, ifstmt, whilestmt, forstmt, body, signature, defndecl, decl, addressable, fields, field}\}$

Start symbol: $\text{program}$

Generation rules:
$$\begin{align*}
\text{program} &\rightarrow \text{defndecls END}\\
\text{defndecls} &\rightarrow \text{defndecl}\\
\text{defndecls} &\rightarrow \text{defndecl defndecls}\\
\text{defndecl} &\rightarrow \text{fndefn}\\
\text{defndecl} &\rightarrow \text{recdefn}\\
\text{defndecl} &\rightarrow \text{decl}\\
\text{fndefn} &\rightarrow \text{signature LBRACE body RBRACE}\\
\text{recdefn} &\rightarrow \text{RECORD IDENTIFIER LPAREN fields RPAREN} \\
\text{decl} &\rightarrow \text{DECLARE signature}\\
\text{signature} &\rightarrow \text{type IDENTIFIER LPAREN optparams RPAREN}\\
\text{type} &\rightarrow \text{INT}\\
\text{type} &\rightarrow \text{VOID}\\
\text{type} &\rightarrow \text{CHAR}\\
\text{type} &\rightarrow \text{BOOL}\\
\text{type} &\rightarrow \text{RECORD IDENTIFIER}\\
\text{type} &\rightarrow \text{type LBRACKET CONSTANT RBRACKET}\\
\text{optparams} &\rightarrow \varnothing \\
\text{optparams} &\rightarrow \text{params} \\
\text{params} &\rightarrow \text{param COMMA params}  \\
\text{params} &\rightarrow \text{param}  \\
\text{param} &\rightarrow \text{type IDENTIFIER}  \\
\text{fields} &\rightarrow \text{field COMMA fields}  \\
\text{fields} &\rightarrow \text{field}  \\
\text{field} &\rightarrow \text{type IDENTIFIER} \\
\text{body} &\rightarrow \text{stmts}  \\
\text{stmts} &\rightarrow \text{stmt}  \\
\text{stmts} &\rightarrow \text{stmt stmts}  \\
\text{stmt} &\rightarrow \text{semistmt SEMICOLON}  \\
\text{semistmt} &\rightarrow \text{ASM LPAREN STRINGLIT RPAREN}\\
\text{semistmt} &\rightarrow \text{vardec}  \\
\text{vardec} &\rightarrow \text{LET type IDENTIFIER ASSIGN expr}  \\
\text{vardec} &\rightarrow \text{LET type IDENTIFIER}  \\
\text{semistmt} &\rightarrow \text{varasst}  \\
\text{varasst} &\rightarrow \text{addressable ASSIGN expr}  \\
\text{addressable} &\rightarrow \text{IDENTIFIER}  \\
\text{addressable} &\rightarrow \text{LPAREN addressable RPAREN}  \\
\text{addressable} &\rightarrow \text{addressable LBRACKET expr RBRACKET}  \\
\text{addressable} &\rightarrow \text{addressable DOT IDENTIFIER} \\
\text{semistmt} &\rightarrow \text{ret}  \\
\text{ret} &\rightarrow \text{HOP expr}  \\
\text{ret} &\rightarrow \text{HOP}  \\
\text{ret} &\rightarrow \text{HOP type} \\
\text{semistmt} &\rightarrow \text{expr} \\
\text{stmt} &\rightarrow \text{ifstmt}  \\
\text{ifstmt} &\rightarrow \text{IF LPAREN expr RPAREN LBRACE body RBRACE optelse}  \\
\text{optelse} &\rightarrow \varnothing  \\
\text{optelse} &\rightarrow \text{ELSE LBRACE stmts RBRACE}  \\
\text{stmt} &\rightarrow \text{whilestmt}  \\
\text{whilestmt} &\rightarrow \text{WHILE LPAREN expr RPAREN LBRACE body RBRACE}  \\
\text{stmt} &\rightarrow \text{forstmt}  \\
\text{forstmt} &\rightarrow \text{FOR LPAREN semistmt SEMICOLON expr SEMICOLON semistmt RPAREN LBRACE stmts RBRACE}  \\
\text{expr} &\rightarrow \text{orcond}  \\
\text{orcond} &\rightarrow \text{orcond OR andcond}  \\
\text{orcond} &\rightarrow \text{andcond}  \\
\text{andcond} &\rightarrow \text{andcond AND uncond}  \\
\text{andcond} &\rightarrow \text{uncond}  \\
\text{uncond} &\rightarrow \text{NOT expr}  \\
\text{uncond} &\rightarrow \text{LPAREN expr RPAREN}  \\
\text{uncond} &\rightarrow \text{expr LT expr}  \\
\text{uncond} &\rightarrow \text{expr GT expr}  \\
\text{uncond} &\rightarrow \text{expr LE expr}  \\
\text{uncond} &\rightarrow \text{expr GE expr}  \\
\text{uncond} &\rightarrow \text{expr EQ expr}  \\
\text{uncond} &\rightarrow \text{expr NE expr}  \\
\text{uncond} &\rightarrow \text{TRUE}  \\
\text{uncond} &\rightarrow \text{FALSE}  \\
\text{uncond} &\rightarrow \text{expr}\\
\text{expr} &\rightarrow \text{addexpr}  \\
\text{addexpr} &\rightarrow \text{addexpr PLUS multexpr}  \\
\text{addexpr} &\rightarrow \text{addexpr MINUS multexpr}  \\
\text{addexpr} &\rightarrow \text{multexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr TIMES unexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr DIVIDE unexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr MOD unexpr}  \\
\text{multexpr} &\rightarrow \text{unexpr}  \\
\text{unexpr} &\rightarrow \text{MINUS unexpr} \\
\text{unexpr} &\rightarrow \text{addressable} \\
\text{unexpr} &\rightarrow \text{LPAREN expr RPAREN}  \\
\text{unexpr} &\rightarrow \text{call}\\
\text{call} &\rightarrow \text{IDENTIFIER LPAREN optargs RPAREN}  \\
\text{optargs} &\rightarrow \varnothing  \\
\text{optargs} &\rightarrow \text{args} \\
\text{args} &\rightarrow \text{expr} \\
\text{args} &\rightarrow \text{expr COMMA args} \\
\text{unexpr} &\rightarrow \text{INC addressable} \\
\text{unexpr} &\rightarrow \text{DEC addressable} \\
\text{unexpr} &\rightarrow \text{CONSTANT}  \\
\text{unexpr} &\rightarrow \text{CHARLIT}
\end{align*}
$$

We also expand all of the children derived by the following symbols as if they were derived by one rule in a postprocessing step: $\text{defndecls, params, stmts, args}$. That is, the rules containing these symbols are effectively replaced by the following rules in this step:
$$\begin{align*}
\text{defndecls} &\rightarrow \text{defndecl}^+\\
\text{stmts} &\rightarrow \text{stmt}^+\\
\text{params} &\rightarrow \text{ param (COMMA param)}^+\\
\text{args} &\rightarrow \text{expr (COMMA expr)}^+\\
\text{fields} &\rightarrow \text{field (COMMA field)}^+
\end{align*}$$
