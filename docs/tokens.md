# Tokens
_token_:

- _keyword_
- _identifier_
- _constant_
- _charlit_
- _strlit_
- _punctuator_

_keyword_:
- One of __asm__, __bool__, __char__, __declare__, __else__, __false__, __for__, __hop__, __if__, __int__, __let__, __record__, __true__, __unsafe__, __void__, __while__

_identifier_:
- any string matching the following regex that is not part of the above list: [A-Za-z_][A-Za-z0-9_]*

_constant_:
- any string matching the following regex: [1-9][0-9]*

_charlit_:
- any single character

_strlit_:
- any sequence of characters

_punctuator_:
- One of __(__, __)__, __{__, __}__, __++__, __--__, __&&__, __||__, __!__, __+__, __-__, __*__, __/__, __%__, __<__, __>__, __<=__, __>=__, __==__, __!=__, __=__, __,__, __;__, __'__, __"__, __[__, __]__, __.__, __&__, __|__, __^__, __<<__, __>>__, __~__
