arch snes.cpu
output "test.bin", create

origin $0000; fill $10000
origin $0000; base $8000

function @strlen text, offset
  if {@get {#text}, {#offset}} == 0
    return {#offset}
  else
    return {@strlen {#text}, {={#offset} + 1}}
  endif
endfunction

function @strlen text
  return {@strlen {#text}, 0}
endfunction

function @uppercase text, offset
  if {@get {#text}, {#offset}} == 0
    return {#text}
  else
    if {@get {#text}, {#offset}} >= 'a' && {@get {#text}, {#offset}} <= 'z'
      define #text {@set {#text}, {#offset}, {@get {#text}, {#offset}} + 'A' - 'a'}
    endif
    return {@uppercase {#text}, {={#offset} + 1}}
  endif
endfunction

function @uppercase text
  return {@uppercase {#text}, 0}
endfunction

function @lowercase text
  return {@transform {#text}, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "abcdefghijklmnopqrstuvwxyz"}
endfunction

function repeat times, function
  if {#times}
    {{#function}}
    {repeat {={#times} - 1}, {#function}}
  endif
endfunction

function increment variable, by
  eval {#variable} {{#variable}} + {#by}
endfunction

function factorial n
  if {#n} == 1
    return 1
  else
    return {={#n} * {factorial {={#n} - 1}}}
  endif
endfunction

function fact n
  eval #result {#n}
  while {#n} > 1
    eval #n {#n} - 1
    eval #result {#result} * {#n}
  endwhile
  return {#result}
endfunction

function loop times
  while {#times}
    eval #repeat {#times}
    print "{factorial {#times}}: "
    while {#repeat}
      print "hello"
      eval #repeat {#repeat} - 1
    endwhile
    print "\n"
    eval #times {#times} - 1
  endwhile
endfunction

main:
  print "{@strlen "hello, world!"}\n"
  print {@uppercase "hello, world!"}, "\n"
  print {@lowercase "Hello, World!"}, "\n"
  define counter 10
  {repeat 5, "increment counter, 2"}
  print "{counter}\n"
  print "{factorial 10}\n"
  {loop 5}
  print "{fact 10}\n"
