\LoadClass[xcolor=x11names,hyperref={colorlinks=true,urlcolor=DodgerBlue4}]{beamer}
\RequirePackage[ngerman,shorthands=off]{babel} % Language specific settings
\RequirePackage{hyphenat} % Hyphenation of words containing analphabetic characters
\RequirePackage[babel=true,tracking=true]{microtype} % Micro-typographic refinements
\RequirePackage{listings} % Source code
\usepackage{xcolor,colortbl} % Colors

% Default settings
\lstset{
    numberstyle = \tiny\color{Gray0},
    numbersep = 1em,
    tabsize = 4,
    breaklines = true,
    breakatwhitespace = true,
    basicstyle = \ttfamily\small,
    keywordstyle = \color{Firebrick3},
    keywordstyle = [2]{\color{SpringGreen4}},
    keywordstyle = [3]{\color{DodgerBlue4}},
    keywordstyle = [4]{\color{DarkOrange3}},
    stringstyle = \color{OliveDrab4},
    commentstyle = \itshape\color{Snow4}}

% Style with context information
\lstdefinestyle{context}{
    basicstyle = \ttfamily\scriptsize,
    backgroundcolor = \color{gray!10},
    numbers = left,
    frame = lines}

\newcommand*{\monospaced}[1]{\texttt{\small #1}}

% Theming
\usetheme{Madrid}
\usecolortheme{beaver}

\definecolor{red}{rgb}{0.8,0,0}
\definecolor{green}{rgb}{0,0.8,0}
\definecolor{blue}{rgb}{0,0,0.8}

% Information
\title{Code-Generierung und -Optimierung für Deep-Learning-Anwendungen auf modernen Prozessoren}
\author{Gabriel Borrelli}
\date{\today}

% vi: ft=tex tw=100
