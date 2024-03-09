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

  li $t4, 4
  li $t5, 8
  la $a3, float_neg_15_25235

  # COP1X | $a3 | $zero | 0_5 | $f0 | LWXC1
  # 010011 | 00111 | 00000 | 00000 | 00000 | 000000 = 0x4ce00000
  #lwxc1 $f0, $zero($a3)
  lwc1 $f0, 0($a3)

  # COP1X | $a3 | $t4 | 0_5 | $f1 | LWXC1
  # 010011 | 00111 | 01100 | 00000 | 00001 | 000000 = 0x4cec0040
  #lwxc1 $f1, $t4($a3)
  lwc1 $f1, 4($a3)

  add.s $f2, $f0, $f1 # 0x463BE93A = 12026.3

  # COP1X | $a3 | $t5 | $f2 | 0_5 | SWXC1
  # 010011 | 00111 | 01101 | 00010 | 00000 | 001000 = 0x4ced1008
  #swxc1 $f2, $t5($a3)
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
