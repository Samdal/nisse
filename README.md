# nisse, the not intricate serialised s expressions

```
nisse  __
    .-'  |
   /   <\|
  /     \'
  |_.- o-o
  / C  -._)\
 /',        |
|   `-,_,__,'
(,,)====[_]=|
  '.   ____/
   | -|-|_
   |____)_)
```


### Typesystem:
- Entries starting with any of `1234567890-.` are interpreted as numbers
  - And if the number contains a `.` it is considered a float
- Entries starting with \` are a multiline raw string \`like this\`
  - They can contain raw newlines, does not support escaped characters.
- Entries starting with `(` are arrays, and they are ended with a `)`
- Other entries are interpreted as whitespace delimited words


A common behaviour is to have a string as the first element of an array, where that string is the tag or variable name.

### API

The C library exposes the AST directly, to write to a file you must generate
one yourself. There are macros and functions to make this step easier.

A parsed file returns a generated AST. There are functions that help you 
verify formats and for getting tagged arrays/variables.

**View nisse.h for information about the functions.**

### example .nisse file
```lisp
(player
    (hp 2.000000)
    (items
        (sword (type weapon) (damage 10))
        (knife (damage 4))
        (bow (type weapon) (damage 10) (range 15))
        (crossbow
            (type `ranged weapon`)
            (damage 16)
            (range 25)
            (properties poison fire)
        )
        (sling (damage 4) (range 3))
    )
)
(items
    (test)
    ()
    (())
    (sword (damage 10))
)
(`v3 array`
    (1.000000 2.000000 3.000000)
    (4.000000 5.000000 6.000000)
    (7.000000 8.000000 9.000000)
    (10.000000 0.000010 420.690002)
)

```
