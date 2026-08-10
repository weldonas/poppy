# Poppy's Type System

Poppy's types are defined according to the context-free grammar below. The starting symbol is $\text{type}$, $\text{num}$ can be substituted for any positive integer, and $\text{str}$ can be substituted for any valid Poppy identifer.
$$
\begin{align*}
\text{type}&\rightarrow \text{returnabletype}\\
\text{returnabletype}&\rightarrow \text{inmemory}\\
\text{inmemory}&\rightarrow \text{int}\\
\text{inmemory}&\rightarrow \text{char}\\
\text{inmemory}&\rightarrow \text{bool}\\
\text{inmemory}&\rightarrow \text{inmemory[num]}\\
\text{inmemory}&\rightarrow \text{\&inmemory}\\
\text{inmemory}&\rightarrow \text{(fields)}\\
\text{fields}&\rightarrow \text{str inmemory, fields}\\
\text{fields}&\rightarrow \text{str inmemory}\\
\text{returnabletype}&\rightarrow \text{void}\\
\text{type}&\rightarrow \text{(optparams) $\mapsto$ returnabletype}\\
\text{optparams}&\rightarrow \varnothing\\
\text{optparams}&\rightarrow \text{params}\\
\text{params}&\rightarrow \text{inmemory, params}\\
\text{params}&\rightarrow \text{inmemory}\\
\end{align*}
$$

We also assume that $\text{str}$ is a built-in record type.

Poppy has the following typing inference rules. We define the sets $R$ and $M$ for the sake of convenience:
$$R:=\{t:\text{returnabletype} \to^* t\}$$
$$M:=\{t:\text{inmemory} \to^* t\}$$

As well, for a record type $\tau$, if the field $\text s$ has the type $\sigma$, we say that $\gamma_\tau(\text s)=\sigma$. Moreover, if it is possible to safely cast from one type $\sigma$ to another type $\tau$, we say that $(\sigma, \tau) \in C$. 

Note that superscript $a$ represents a type being assignable and is only included in type rules where assignability is relevant. For all potentially assignable types $\tau$, we say that $\tau^a = \tau$.

## Program
$$\frac{\forall i \text{ } \overline{\text{defndecls}} \vdash \text{defndecls}_i}{\varnothing \vdash \overline{\text{defndecls}}}$$

## Functions

$$\frac{\Gamma(\text{IDENTIFIER}) = \upsilon \text{ IDENTIFIER}(\overline{\tau \text{ IDENTIFIER}}) \quad \tau \in M, \upsilon \in R}{\Gamma \vdash \text{IDENTIFIER}:(\overline{\tau}) \mapsto \upsilon}$$

$$\frac{\Gamma \vdash E:(\tau_1,\dots,\tau_n)\mapsto \tau' \quad \forall i \in [n], \Gamma \vdash E_i : \tau_i} {
    \Gamma \vdash E(E_1,\dots, E_n): \tau'
}$$

$$\frac{\text{all symbol names in params, $E$ distinct} \quad \Gamma \vdash E: \tau \quad \tau \in R}{\Gamma \vdash \tau \text{ IDENTIFIER }(\overline{\text{params}})\{E\}}$$

## Variables

$$\frac{\Gamma(\text{IDENTIFIER}) = \text{let } \tau \text{ IDENTIFIER}; \quad \tau \in M}{\Gamma \vdash \text{IDENTIFIER} : \tau^a \quad \Gamma \vdash \text{let } \tau \text{ IDENTIFIER};: \text{void}}$$

$$\frac{\Gamma(\text{IDENTIFIER}) = \text{let } \tau \text{ IDENTIFIER} = E; \quad \Gamma \vdash E : \tau \quad \tau \in M}{\Gamma \vdash \text{IDENTIFIER} : \tau^a \quad \Gamma \vdash \text{let } \tau \text{ IDENTIFIER} = E; : \text{void}}$$

$$\frac{\Gamma \vdash E_1 : \tau^a \quad \Gamma \vdash E_2 : \tau}{
    \Gamma \vdash E_1 = E_2 : \text{void}
}$$

## Non-Primitive Data Types
$$\frac{\Gamma \vdash A : \tau[n] \quad \Gamma \vdash i:\text{int}\quad}{ \Gamma \vdash A[i] : \tau}$$

$$\frac{\Gamma \vdash E: \tau^a}{\Gamma \vdash \&E: \&\tau}$$

$$\frac{\Gamma \vdash E: \&\tau}{\Gamma \vdash *E : \tau}$$

$$\frac{\Gamma \vdash E: \tau \quad \gamma_\tau(\text s) = \sigma}{\Gamma \vdash E\text{.s} : \sigma}$$

$$\frac{\Gamma \vdash E:\tau \quad (\tau, \sigma) \in C}{\Gamma \vdash \text{safe }\sigma(E) : \sigma}$$

$$\frac{\Gamma \vdash E:\tau }{\Gamma \vdash \text{unsafe } \sigma(E) : \sigma}$$


## Statements

$$\frac{\Gamma \vdash E_1: \tau_1 \quad \Gamma \vdash E_2: \tau_2 \quad E_1, E_2 \in \text{stmt}}{
    \Gamma \vdash E_1 E_2 : \tau_2
}$$

$$\frac{\Gamma \vdash E : \tau \quad \tau \in M}{
    \Gamma \vdash \text{hop } E; : \tau
}$$

$$\frac{}{
    \Gamma \vdash \text{hop}; : \text{void}
}$$

$$\frac{\Gamma \vdash b : \text{bool} \quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{if (\emph b) \{\emph E\}}: \text{void}}$$

$$\frac{\Gamma \vdash b : \text{bool} \quad \Gamma \vdash E_1: \tau_1 \quad \Gamma \vdash E_2: \tau_2}{\Gamma \vdash \text{if (\emph b) \{} E_1\text{\} else \{} E_2\text{\}}: \tau_1 \text{ if } \tau_1 = \tau_2, \text{ otherwise void}}$$

$$\frac{\Gamma \vdash b: \text{bool} \quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{while }(b)\{E\}: \tau}$$

$$\frac{\Gamma \vdash b: \text{bool} \quad \Gamma \vdash a, c : \text{void}\quad \Gamma \vdash E: \tau}{\Gamma \vdash \text{for }(a;b;c)\{E\}: \tau}$$

$$\frac{}{
    \Gamma\vdash \text{asm}(\text{STRINGLIT}):\text{void}
}$$

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

$$\frac{\Gamma \vdash E : \tau \quad \tau^a \in \{\text{int}, \text{char}\}}{\text{++}E:\tau \quad \text{-}\text{-}E:\tau}$$

$$\frac{\Gamma \vdash E: \text{int}}{\Gamma \vdash -E:\tau}$$

$$\frac{\Gamma \vdash E_1: \text{int}\quad \Gamma \vdash E_2: \text{int}}{
    \Gamma \vdash E_1 \text{ \& } E_2 : \text{int} \quad     \Gamma \vdash E_1 \text{ | }  E_2 : \text{int} \quad     \Gamma \vdash E_1 \text{ \^ } E_2 : \text{int} \quad \Gamma \vdash E_1 \text{ << } E_2 : \text{int} \quad \Gamma \vdash E_1 \text{ >> } E_2 : \text{int}
    \quad \Gamma \vdash \text{\~ } E_1 : \text{int}
}$$

$$\frac{\Gamma \vdash E_1: \tau^a\quad \Gamma \vdash E_2: \tau\quad \tau\in \{\text{int, char}\}}{
    \Gamma \vdash E_1 += E_2 : \text{void} \quad \Gamma \vdash E_1 -= E_2 : \text{void}\quad \Gamma \vdash E_1 *= E_2 : \text{void} \quad \Gamma \vdash E_1 \text{ /= }E_2 : \text{void} \quad \Gamma \vdash E_1 
    \text{ \%= } E_2 : \text{void}
}$$

$$\frac{\Gamma \vdash E_1: \text{int}^a\quad \Gamma \vdash E_2: \text{int}}{
    \Gamma \vdash E_1 \text{ \&= } E_2 : \text{void}\quad     \Gamma \vdash E_1 \text{ |= }  E_2 : \text{void} \quad     \Gamma \vdash E_1 \text{ \^ = } E_2 : \text{void} \quad \Gamma \vdash E_1 \text{ <<= } E_2 : \text{void} \quad \Gamma \vdash E_1 \text{ >>= } E_2 : \text{void}
}$$

## Literals

$$\Gamma \vdash \text{CONSTANT}: \text{int}$$
$$\Gamma \vdash \text{CHARLIT}: \text{char}$$
$$\Gamma \vdash \text{STRINGLIT}: \text{string}$$
$$\Gamma \vdash \text{TRUE}: \text{bool}$$
$$\Gamma \vdash \text{FALSE}: \text{bool}$$
