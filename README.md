# KISS - Knowledge in Security protocolS

This repository contains the source code and example inputs for the
tool KISS, which I have developed as part of my PhD thesis at LSV, ENS
Cachan.

Please contact me at stefan.ciobaca@gmail.com for any questions.

This readme file contain the information that I retrieved from my old
webpage at LSV, edited to be more up to date.

## Abstract

KISS (Knowledge In Security protocolS) is an implementation of the
 procedure for deduction and static equivalence presented in in our <a
 href="http://www.lsv.ens-cachan.fr/Publis/RAPPORTS_LSV/PDF/rr-lsv-2009-05.pdf">
 research report</a>. The procedure is able to handle cryptographic
 primitives modeled by any convergent term rewriting system. KISS is
 known to terminate on a wide range of rewriting systems, such as
 subterm convergent rewriting systems, blind signatures and trapdoor
 commitment.

## Installation

Assuming you are using a Unix-based system, run the following
commands:

   make                                   ;; compile kiss
   ./kiss < running-example.in            ;; test the tool on the

## Tutorial

KISS reads queries from the standard input and outputs answers to
standard output. The input must corespond to the following BNF-like
grammar:

_Number_ ::= \[0-9\]\*

_Identifier_ ::= (\[a-z\] | \[A-Z\] | \_ | \[0-9\])\*

_Term_ ::= _Identifier_ | _Identifier(Term#)_

_Sign_ ::= **signature** (_Identifier_ **/** _Number_)#

_Vars_ ::= **variables** _Identifier_#

_Names_ ::= **names** _Identifier_#

_Rewrite_ ::= **rewrite** (_Term_ **\->** _Term_)#

_Frame_ ::= **new** Identifier# **.{** (_Identifier_ = _Term_)# **}**

_Frames_ ::= **frames** (_Identifier_ = _Frame_)#

_Question_ ::=

**deducible** _Term_ _Identifier_ |

**equiv** _Identifier_ _Identifier_ |

**knowledgebase** _Identifier_

_Questions_ ::= **questions** _Question_#

_Program_ ::= _Sign_ **;** _Vars_ **;** _Names_ **;** _Rewrite_ **;** _Frames_ **;** _Questions_ **;**

where keywords appear in **bold**, where _Program_ is the starting
symbol and where _Something_\# is a shortcut for _Something_ (**,**
_Something_)\*. Here is an example input:

    signature pair/2, fst/1, snd/1, enc/2, dec/2;
    variables x, y, z;
    names a, b, c, k, w1, w2, w3;
    rewrite
            fst(pair(x, y)) -> x,
            snd(pair(x, y)) -> y,
            dec(enc(x, y), y) -> x;
    frames
            phi1 = new a, k.{w1 = enc(a, k), w2 = a},
            phi2 = new b, k.{w1 = enc(b, k), w2 = c};
    questions
            deducible a phi1,
            equiv phi1 phi2,
            knowledgebase phi1;

The first line declares 5 function symbols, together with their
arity. The following lines declares that the identifiers x, y and z
will be used as variables. The following line declares that the
identifiers a, b, c, k, w1, w2 and w3 will be used as names.

The declaration of term rewriting system starts on the fourth line and
it contains three rewrite rules.

Next follow the frame declarations. We declare two frames, phi1 and
phi2. Each of the two frames contains two bound names: a and k for
phi1 and resp. b and k for phi2. The frames each contain two messages
identified by the names w1 and resp. w2.

On the fourth to last line start the questions. The first question is
to determine if the term a is deducible from the frame phi1. The next
question asks if the two frames are statically equivalent and the last
question asks for the knowledge base associated to the frame phi1.

For reference, here is the output of KISS when run on the above
example:

    Saturating frame phi1...
    We have 7 deduction facts and 3 eq. facts.
    Saturating frame phi2...
    We have 8 deduction facts and 4 eq. facts.



    Answers to questions:


    Yupii, phi1 |- a with recipe w2

    Sorry, phi1 is not statically equivalent to phi2
    Here's an equation that holds in phi2 but not in phi1
    \[ c ~ w2 |  \]

    Voila the saturated knowledge base of phi1:
    Deduction facts:
       \[ dec(\\X0,\\X1) > dec(\\x0,\\x1) | \\X0 > \\x0, \\X1 > \\x1 \]
       \[ enc(\\X0,\\X1) > enc(\\x0,\\x1) | \\X0 > \\x0, \\X1 > \\x1 \]
       \[ fst(\\X0) > fst(\\x0) | \\X0 > \\x0 \]
       \[ pair(\\X0,\\X1) > pair(\\x0,\\x1) | \\X0 > \\x0, \\X1 > \\x1 \]
       \[ snd(\\X0) > snd(\\x0) | \\X0 > \\x0 \]
       \[ w1 > enc(a,k) |  \]
       \[ w2 > a |  \]
    Equational facts:
       \[ dec(enc(\\X0,\\X1),\\X1) ~ \\X0 |  \]
       \[ fst(pair(\\X0,\\X1)) ~ \\X0 |  \]
       \[ snd(pair(\\X0,\\X1)) ~ \\X1 |  \]

The program starts by applying the saturation procedure on phi1 and
phi2. In general, because the underlying problem is undecidable, the
saturation process does not terminate. For this particular term
rewriting system, saturation is guaranteed to terminate (in polynomial
time).

After having saturated all frames, KISS starts answering
questions. KISS is glad to see that the term a is indeed deducible
from phi1 with recipe w2. It is sorry to inform you that phi1 and phi2
are not statically equivalent and it gives an example of an equation
that holds in phi2 but not in phi1 (the equation being c ~ w2). Had
there been an equation that would hold in phi1 but not in phi2, KISS
would have informed you of that as well. Finally, KISS joyfully shows
you the knowledgebase associated to phi1.

## Command line arguments

In order to see the intermediate deduction and equational facts
(i.e. the unsolved ones), you can increase the verbosity to 2 (or even
to 4):

    ./kiss -v 2 < example.in

    Saturating frame phi1...
    Solving \[ dec(\\Y0,\\Y1) > \\y0 | \\Y0 > enc(\\y0,\\y1), \\Y1 > \\y1 \]
    Solving \[ dec(enc(\\Y0,\\Y1),\\Y2) > \\y0 | \\Y0 > \\y0, \\Y1 > \\y1, \\Y2 > \\y1 \]
    Solving \[ dec(w1,\\Y0) > a | \\Y0 > k \]
    ...

If you do not like to see KISS happy, you can shut it up using the --sober option:

    ./kiss --sober < example.in

    ...
    Answers to questions:
    
    
    phi1 |- a with recipe w2
    
    phi1 is not statically equivalent to phi2
    Here's an equation that holds in phi2 but not in phi1
    \[ c ~ w2 |  \]
    ...

