.data
str1: .asciiz "Please enter first operand, A "
str2: .asciiz "Please enter second operand, B "
str3: .asciiz "A - B = "
newline: .asciiz "\n"  #This will cause the screen cursor to move to a newline

.text

.globl main

main:

  li $a0, 15
  li $a1, 3
  add $a2 $a0 $a1
  la $a3, str1
  sw $a2, 0($a3)
  lwc1 $a2, 0($a3)

end:
  addi $v0, $zero, 10
  syscall
  j end
