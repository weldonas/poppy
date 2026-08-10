# Context-Free Grammar for Poppy

Terminal symbols: all tokens for the language.

Nonterminal symbols: $\{\text{program, defndecls, defn, type, optparams, stmts, params, param, stmt, semistmt, expr, orexpr, andexpr, eqexpr, compexpr, borexpr, bxorexpr, bandexpr, bshiftexpr, addexpr, multexpr, unaryexpr, memberexpr, baseexpr, optelse, optargs, args, vardec, varasst, ret, call, ifstmt, whilestmt, forstmt, body, signature, defndecl, decl, addressable, fields, field, cast}\}$

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
\text{type} &\rightarrow \text{AMP type}\\
\text{type} &\rightarrow \text{LPAREN type RPAREN}\\
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
\text{semistmt} &\rightarrow \text{vardec}  \\
\text{vardec} &\rightarrow \text{LET type IDENTIFIER ASSIGN expr}  \\
\text{vardec} &\rightarrow \text{LET type IDENTIFIER}  \\
\text{semistmt} &\rightarrow \text{varasst}  \\
\text{varasst} &\rightarrow \text{expr ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr PLUS ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr MINUS ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr STAR ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr DIVIDE ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr MOD ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr BLEFT ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr BRIGHT ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr AMP ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr BXOR ASSIGN expr}  \\
\text{varasst} &\rightarrow \text{expr BOR ASSIGN expr}  \\
\text{semistmt} &\rightarrow \text{ret}  \\
\text{ret} &\rightarrow \text{HOP expr}  \\
\text{ret} &\rightarrow \text{HOP}  \\
\text{semistmt} &\rightarrow \text{expr} \\
\text{stmt} &\rightarrow \text{ifstmt}  \\
\text{ifstmt} &\rightarrow \text{IF LPAREN expr RPAREN LBRACE body RBRACE optelse}  \\
\text{optelse} &\rightarrow \varnothing  \\
\text{optelse} &\rightarrow \text{ELSE LBRACE stmts RBRACE}  \\
\text{stmt} &\rightarrow \text{whilestmt}  \\
\text{whilestmt} &\rightarrow \text{WHILE LPAREN expr RPAREN LBRACE body RBRACE}  \\
\text{stmt} &\rightarrow \text{forstmt}  \\
\text{forstmt} &\rightarrow \text{FOR LPAREN semistmt SEMICOLON expr SEMICOLON semistmt RPAREN LBRACE stmts RBRACE}  \\
\text{expr} &\rightarrow \text{orexpr}  \\
\text{orexpr} &\rightarrow \text{orexpr OR andexpr}  \\
\text{orexpr} &\rightarrow \text{andexpr}  \\
\text{andexpr} &\rightarrow \text{andexpr AND eqexpr}  \\
\text{andexpr} &\rightarrow \text{eqexpr}  \\
\text{eqexpr} &\rightarrow \text{eqexpr EQ compexpr}  \\
\text{eqexpr} &\rightarrow \text{eqexpr NE compexpr}  \\
\text{eqexpr} &\rightarrow \text{compexpr}  \\
\text{compexpr} &\rightarrow \text{compexpr LT borexpr}  \\
\text{compexpr} &\rightarrow \text{compexpr GT borexpr}  \\
\text{compexpr} &\rightarrow \text{compexpr LE borexpr}  \\
\text{compexpr} &\rightarrow \text{compexpr GE borexpr}  \\
\text{compexpr} &\rightarrow \text{borexpr}  \\
\text{borexpr} &\rightarrow \text{borexpr BOR bxorexpr}  \\
\text{borexpr} &\rightarrow \text{bxorexpr}  \\
\text{bxorexpr} &\rightarrow \text{bxorexpr BXOR bandexpr}  \\
\text{bxorexpr} &\rightarrow \text{bandexpr}  \\
\text{bandexpr} &\rightarrow \text{bandexpr AMP bshiftexpr}  \\
\text{bandexpr} &\rightarrow \text{bshiftexpr}  \\
\text{bshiftexpr} &\rightarrow \text{bshift BLEFT addexpr}  \\
\text{bshiftexpr} &\rightarrow \text{bshift BRIGHT addexpr}  \\
\text{bshiftexpr} &\rightarrow \text{addexpr}  \\
\text{addexpr} &\rightarrow \text{addexpr PLUS multexpr}  \\
\text{addexpr} &\rightarrow \text{addexpr MINUS multexpr}  \\
\text{addexpr} &\rightarrow \text{multexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr STAR unaryexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr DIVIDE unaryexpr}  \\
\text{multexpr} &\rightarrow \text{multexpr MOD unaryexpr}  \\
\text{multexpr} &\rightarrow \text{unaryexpr}  \\
\text{unaryexpr} &\rightarrow \text{INC memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{DEC memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{MINUS memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{NOT memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{BNOT memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{STAR memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{AMP memberexpr}  \\
\text{unaryexpr} &\rightarrow \text{memberexpr}  \\
\text{memberexpr} &\rightarrow \text{memberexpr LBRACKET expr RBRACKET}  \\
\text{memberexpr} &\rightarrow \text{memberexpr DOT IDENTIFIER} \\
\text{memberexpr} &\rightarrow \text{baseexpr} \\
\text{baseexpr} &\rightarrow \text{LPAREN expr RPAREN}  \\
\text{baseexpr} &\rightarrow \text{TRUE}  \\
\text{baseexpr} &\rightarrow \text{FALSE}  \\
\text{baseexpr} &\rightarrow \text{call}\\
\text{baseexpr} &\rightarrow \text{cast} \\
\text{baseexpr} &\rightarrow \text{IDENTIFIER}  \\
\text{baseexpr} &\rightarrow \text{CONSTANT}  \\
\text{baseexpr} &\rightarrow \text{CHARLIT} \\
\text{baseexpr} &\rightarrow \text{STRINGLIT} \\
\text{baseexpr} &\rightarrow \text{ASM LPAREN STRINGLIT RPAREN} \\
\text{call} &\rightarrow \text{IDENTIFIER LPAREN optargs RPAREN}  \\
\text{optargs} &\rightarrow \varnothing  \\
\text{optargs} &\rightarrow \text{args} \\
\text{args} &\rightarrow \text{expr} \\
\text{args} &\rightarrow \text{expr COMMA args} \\
\text{cast} &\rightarrow \text{type LPAREN expr RPAREN} \\
\text{cast} &\rightarrow \text{UNSAFE type LPAREN expr RPAREN} \\
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

Note that the rules above yield the following operator precedence classes, which are mostly modelled off of those for C

Precedence 1 ($\text{memberexpr}$): `[]`, `.`

Precedence 2 ($\text{unaryexpr}$): `++`, `--`, `-` (as negation operator), `!`, `~`, `*` (as dereference operator), `&` (as reference operator)

Precedence 3 ($\text{multexpr}$): `*` (as multiplication operator), `/`, `%`

Precedence 4 ($\text{addexpr}$): `+`, `-` (as subtraction operator)

Precedence 5 ($\text{bshiftexpr}$): `<<`, `>>`

Precedence 6 ($\text{bandexpr}$): `&` (as bitwise AND)

Precedence 7 ($\text{bxorexpr}$): `^`

Precedence 8 ($\text{borexpr}$): `|`

Precedence 9 ($\text{compexpr}$): `<`, `<=`, `>`, `>=`

Precedence 10 ($\text{eqexpr}$): `==`, `!=`

Precedence 11 ($\text{andexpr}$): `&&`

Precedence 12 ($\text{orexpr}$): `||`

Precedence 13 ($\text{varasst}$ and $\text{vardec}$): any assignment operator 
