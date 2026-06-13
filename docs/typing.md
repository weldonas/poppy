# Poppy's Type System

Poppy's types are defined according to the context-free grammar below. The starting symbol is $\text{type}$ and $\text{num}$ can be substituted for any positive integer.
$$
\begin{align*}
\text{type}&\rightarrow \text{returnabletype}\\
\text{returnabletype}&\rightarrow \text{int}\\
\text{returnabletype}&\rightarrow \text{char}\\
\text{returnabletype}&\rightarrow \text{bool}\\
\text{returnabletype}&\rightarrow \text{void}\\
\text{type}&\rightarrow \text{(optparams) $\mapsto$ returnabletype}\\
\text{optparams}&\rightarrow \varnothing\\
\text{optparams}&\rightarrow \text{params}\\
\text{params}&\rightarrow \text{param, params}\\
\text{param}&\rightarrow \text{int}\\
\text{param}&\rightarrow \text{char}\\
\text{param}&\rightarrow \text{bool}\\
\text{param}&\rightarrow \text{array}\\
\text{array}&\rightarrow \text{param[num]}\\
\end{align*}
$$


Poppy has the following typing inference rules. We define the sets $R$ and $P$ for the sake of convenience:
$$R:=\{t:\text{returnabletype} \to^* t\}$$
$$P:=\{t:\text{param} \to^* t\}$$

## Program
$$\frac{\forall i \text{ } \overline{\text{defns}} \vdash \text{defns}_i}{\varnothing \vdash \overline{\text{defns}}}$$

## Functions

$$\frac{\Gamma(\text{IDENTIFIER}) = \upsilon \text{ IDENTIFIER}(\overline{\tau \text{ IDENTIFIER}}) \quad \tau \in P, \upsilon \in R}{\Gamma \vdash \text{IDENTIFIER}:(\overline{\tau}) \mapsto \upsilon}$$

$$\frac{\Gamma \vdash E:(\tau_1,\dots,\tau_n)\mapsto \tau' \quad \forall i \in [n], \Gamma \vdash E_i : \tau_i} {
    \Gamma \vdash E(E_1,\dots, E_n): \tau'
}$$

$$\frac{\text{all symbol names in params, $E$ distinct} \quad \Gamma \vdash E: \tau \quad \tau \in R}{\Gamma \vdash \tau \text{ IDENTIFIER }(\overline{\text{params}})\{E\}}$$

## Variables

$$\frac{\Gamma(\text{IDENTIFIER}) = \text{let } \tau \text{ IDENTIFIER}; \quad \tau \in P}{\Gamma \vdash \text{IDENTIFIER} : \tau \quad \Gamma \vdash \text{let } \tau \text{ IDENTIFIER};: \text{void}}$$

$$\frac{\Gamma(\text{IDENTIFIER}) = \tau \text{ IDENTIFIER} = E; \quad \Gamma \vdash E : \tau \quad \tau \in P}{\Gamma \vdash \text{IDENTIFIER} : \tau \quad \Gamma \vdash \text{IDENTIFIER} = E; : \text{void}}$$

## Arrays
$$\frac{\Gamma(\text{IDENTIFIER}) = \text{let } \tau[n] \text{ IDENTIFIER}; \quad \tau \in P\quad n \in \mathbb N^+}{\Gamma \vdash \text{IDENTIFIER} : \tau[n] \quad \Gamma \vdash \text{let } \tau[n] \text{ IDENTIFIER};: \text{void}}$$

$$\frac{\Gamma \vdash A : \tau[n] \quad \Gamma \vdash i:\text{int}\quad \Gamma \vdash E:\tau}{ \Gamma \vdash A[i] = E;: \text{void}}$$

$$\frac{\Gamma \vdash A : \tau[n] \quad \Gamma \vdash i:\text{int}\quad}{ \Gamma \vdash A[i] : \tau}$$

## Statements

$$\frac{\Gamma \vdash E_1: \tau_1 \quad \Gamma \vdash E_2: \tau_2 \quad E_1, E_2 \in \text{stmt}}{
    \Gamma \vdash E_1 E_2 : \tau_2
}$$

$$\frac{\Gamma \vdash E : \tau \quad \tau \in P}{
    \Gamma \vdash \text{hop } E; : \tau
}$$

$$\frac{\Gamma \vdash b : \text{bool} \quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{if (\emph b) \{\emph E\}}: \text{void}}$$

$$\frac{\Gamma \vdash b : \text{bool} \quad \Gamma \vdash E_1: \tau_1 \quad \Gamma \vdash E_2: \tau_2}{\Gamma \vdash \text{if (\emph b) \{} E_1\text{\} else \{} E_2\text{\}}: \tau_1 \text{ if } \tau_1 = \tau_2, \text{ otherwise void}}$$

$$\frac{\Gamma \vdash b: \text{bool} \quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{while }(b)\{E\}: \tau}$$

$$\frac{\Gamma \vdash b: \text{bool} \quad \Gamma \vdash a, c : \text{void}\quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{for }(a;b;c)\{E\}: \tau}$$

$$\Gamma\vdash \text{asm}(\text{STRINGLIT}):\text{void}$$

## Predicates
$$\frac{\Gamma \vdash E_1:\tau \quad \Gamma \vdash E_2:\tau\quad \tau \in \{\text{int, char, bool}\}}{
    \Gamma \vdash E_1 == E_2 : \text{bool} \quad \Gamma \vdash E_1 \text{ != } E_2 : \text{bool}
}$$

$$\frac{\Gamma \vdash E_1:\tau \quad \Gamma \vdash E_2:\tau \quad \tau \in \{\text{int, char}\}}{
    \Gamma \vdash E_1 < E_2 : \text{bool} \quad \Gamma \vdash E_1 > E_2 : \text{bool} \quad
    \Gamma \vdash E_1 <= E_2 : \text{bool} \quad \Gamma \vdash E_1 >= E_2 : \text{bool}
}$$


$$\frac{\Gamma \vdash E : \text{bool}}{
    \Gamma \vdash \text{ !}E : \text{bool}
}$$

$$\frac{\Gamma \vdash E_1 : \text{bool} \quad \Gamma \vdash E_2 : \text{bool}}{
    \Gamma \vdash E_1 \text{ \&\& } E_2 : \text{bool} \quad     \Gamma \vdash E_1 \text{ || } E_2 : \text{bool}
}$$

## Arithmetic
$$\frac{\Gamma \vdash E_1: \tau_1\quad \Gamma \vdash E_2: \tau_2\quad \tau_1,\tau_2 \in \{\text{int, char}\}}{
    \Gamma \vdash E_1 + E_2 : \tau_1 \quad \Gamma \vdash E_1 - E_2 : \tau_1 \quad \Gamma \vdash E_1 * E_2 : \tau_1 \quad \Gamma \vdash E_1 \text{ / }E_2 : \tau_1 \quad \Gamma \vdash E_1 
    \text{ \% } E_2 : \tau_1
}$$

$$\frac{\Gamma \vdash E: \tau \quad \tau \in \{\text{int, char}\}}{\Gamma \vdash (E):\tau}$$


## Literals

$$\Gamma \vdash \text{CONSTANT}: \text{int}$$
$$\Gamma \vdash \text{CHARLIT}: \text{char}$$
$$\Gamma \vdash \text{TRUE}: \text{bool}$$
$$\Gamma \vdash \text{FALSE}: \text{bool}$$
