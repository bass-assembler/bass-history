arch snes.cpu
output "test.bin", create

origin $0000; fill $8000
origin $0000; base $8000

main:
  db "test"
  -; bra {-}
