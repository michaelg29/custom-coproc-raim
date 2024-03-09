.data
str1: .asciiz "Please enter first operand, A "
str2: .asciiz "Please enter second operand, B "
str3: .asciiz "A - B = "
newline: .asciiz "\n"  #This will cause the screen cursor to move to a newline

float_neg_15_25235: .word 0xC17409A0
float_pos_12041_5582: .word 0x463C263C
float_res: .word 0x0

.text

.globl main
.globl __start

__start:
main:

  la $a3, float_neg_15_25235
  lwc1 $f0, 0($a3)
  lwc1 $f1, 4($a3)
  add.s $f2, $f0, $f1 # 0x463BE93A = 12026.3
  swc1 $f2, 8($a3)

  li $a0, 15
  li $a1, 3
  add $a2 $a0 $a1
  la $a3, str1
  sw $a2, 0($a3)

end:
  addi $v0, $zero, 10
  syscall
  j end
