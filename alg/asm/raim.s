.data
str1: .asciiz "Please enter first operand, A "
str2: .asciiz "Please enter second operand, B "
str3: .asciiz "A - B = "
newline: .asciiz "\n"  #This will cause the screen cursor to move to a newline

.text

.globl main

main:

  la $a0, str1
  li $v0, 4
  syscall

  addi $s0, $zero, 10     # X = 10; $s0 = X,  Value in $s0: 10

end:
  addi $v0, $zero, 10
  syscall
  j end
