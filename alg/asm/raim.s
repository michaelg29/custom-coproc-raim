##################################
##### Satellite vehicle data #####
##################################
.data

# strings
str_greeting:   .asciiz "Hello, world!\n"
.space 1 # pad to 4-byte boundary
str_fault_ndet: .asciiz "Fault not detected!\n"
.space 3 # pad to 4-byte boundary
str_fault_det:  .asciiz "Fault detected!\n"
.space 3 # pad to 4-byte boundary
str_fault_nloc: .asciiz "Fault not located!\n"
# .space 0 # already padded to 4-byte boundary
str_fault_loc:  .asciiz "Fault located!\n"
# .space 0 # already padded to 4-byte boundary

# Constant values for all SVs
data_alpha:    .word 0x3e051eb8 # 0.13
data_sig_ura2: .word 0x3f100000 # 0.75 * 0.75
data_sig_ure2: .word 0x3e800000 # 0.50 * 0.50
data_bias_nom: .word 0x3f000000 # 0.5
data_k_fa_1:   .word 0x40c127bb # 6.0361 = invq((P_FA,H = 9e-8)/2/(N_fault_modes=57))
data_k_fa_2:   .word 0x40c127bb # 6.0361 = invq((P_FA,H = 9e-8)/2/(N_fault_modes=57))
data_k_fa_3:   .word 0x40aca64c # 5.3953 = invq((P_FA,V = 3.9e-6)/2/(N_fault_modes=57))
data_k_fa_r:   .word 0x40939097 # 4.6114 = invq((P_FA = 4e-6)/2)

# individual satellite vehicle data
data_sv0:
  .word 0x3cb851ec # LOS_x = 0.022500
  .word 0x3f7ebee0 # LOS_y = 0.995100
  .word 0xbdc5d639 # LOS_z = -0.096600
  .word 0x00000000 # constellation
  .word 0x3fa2faeb # sig_tropo2 = 1.273282
  .word 0x40033f88 # sig_user2 = 2.050753

data_sv1:
  .word 0x3f2ccccd # LOS_x = 0.675000
  .word 0xbf30a3d7 # LOS_y = -0.690000
  .word 0xbe85bc02 # LOS_z = -0.261200
  .word 0x00000000 # constellation
  .word 0x3e5261d9 # sig_tropo2 = 0.205451
  .word 0x3f2b77e9 # sig_user2 = 0.669798

data_sv2:
  .word 0x3d941206 # LOS_x = 0.072300
  .word 0xbf28fc50 # LOS_y = -0.660100
  .word 0xbf3f6944 # LOS_z = -0.747700
  .word 0x00000000 # constellation
  .word 0x3cd2adba # sig_tropo2 = 0.025718
  .word 0x3e8b58bb # sig_user2 = 0.272161

data_sv3:
  .word 0xbf7096bc # LOS_x = -0.939800
  .word 0x3e82b6ae # LOS_y = 0.255300
  .word 0xbe685879 # LOS_z = -0.226900
  .word 0x00000000 # constellation
  .word 0x3e8a1c12 # sig_tropo2 = 0.269745
  .word 0x3f4e5b71 # sig_user2 = 0.806083

data_sv4:
  .word 0xbf17381d # LOS_x = -0.590700
  .word 0xbf40ff97 # LOS_y = -0.753900
  .word 0xbe934d6a # LOS_z = -0.287700
  .word 0x00000000 # constellation
  .word 0x3e2e4d6b # sig_tropo2 = 0.170217
  .word 0x3f17152d # sig_user2 = 0.590167

data_sv5:
  .word 0xbea5aee6 # LOS_x = -0.323600
  .word 0xbd10ff97 # LOS_y = -0.035400
  .word 0xbf720c4a # LOS_z = -0.945500
  .word 0x00000001 # constellation
  .word 0x3c83eabf # sig_tropo2 = 0.016103
  .word 0x3e878d6a # sig_user2 = 0.264751

data_sv6:
  .word 0xbf2cbfb1 # LOS_x = -0.674800
  .word 0x3edf06f7 # LOS_y = 0.435600
  .word 0xbf187fcc # LOS_z = -0.595700
  .word 0x00000001 # constellation
  .word 0x3d259b2c # sig_tropo2 = 0.040431
  .word 0x3e963948 # sig_user2 = 0.293406

data_sv7:
  .word 0x3dc01a37 # LOS_x = 0.093800
  .word 0xbf334d6a # LOS_y = -0.700400
  .word 0xbf351eb8 # LOS_z = -0.707500
  .word 0x00000001 # constellation
  .word 0x3ceb2dc7 # sig_tropo2 = 0.028708
  .word 0x3e8d2598 # sig_user2 = 0.275677

data_sv8:
  .word 0x3f0e9e1b # LOS_x = 0.557100
  .word 0x3e9e1b09 # LOS_y = 0.308800
  .word 0xbf4559b4 # LOS_z = -0.770900
  .word 0x00000001 # constellation
  .word 0x3cc63a90 # sig_tropo2 = 0.024198
  .word 0x3e8a885d # sig_user2 = 0.270572

data_sv9:
  .word 0x3f2985f0 # LOS_x = 0.662200
  .word 0x3f321ff3 # LOS_y = 0.695800
  .word 0xbe8e5604 # LOS_z = -0.278000
  .word 0x00000001 # constellation
  .word 0x3e3a577c # sig_tropo2 = 0.181974
  .word 0x3f1dfb07 # sig_user2 = 0.617112

data_subsets:
  .word 992  # 5 last satellites
  .word 31   # 5 first satellites

#######################
##### Main method #####
#######################
.text

.globl main
.globl __start
__start:
main:

  la $a0, str_greeting
  ori $v0, $zero, 4
  syscall

  ###########################
  ##### load in SV data #####
  ###########################
  la $t0, data_bias_nom
  nop # LWC2 $AL0, -12($t0)
  nop # LWC2 $SA2, -8($t0)
  nop # LWC2 $SE2, -4($t0)
  nop # LWC2 $KX, 4($t0)
  nop # LWC2 $KY, 8($t0)
  nop # LWC2 $KZ, 12($t0)
  nop # LWC2 $KR, 16($t0)

  # cursors
  la $t1, data_sv0 # initial cursor
  la $t2, data_sv9 # last SV cursor

  # loop
load_loop:
  nop # LWC2 $LX, 0($t1)
  nop # LWC2 $LY, 4($t1)
  nop # LWC2 $LZ, 8($t1)
  nop # LWC2 $C, 12($t1)
  nop # LWC2 $ST, 16($t1)
  nop # LWC2 $SR, 20($t1)
  nop # LWC2 $BN, 0($t0)
  nop # NEWSVC2
  slt $t3, $t1, $t2 # t3 <- t1 < t2
  addi $t1, $t1, 24 # move cursor to next SV
  bnez $t3, load_loop

  #################################
  ##### calculate LS matrices #####
  #################################
  nop # CALCUC2

  # all-in-view
  ori $t0, $zero, 1023
  nop # MTC2 $t0, $IDX
  nop # INITPC2
  nop # CALCPC2
  nop # WLSC2
  nop # NEWSSC2

  # cursors
  la $t0, data_subsets # initial subset
  addi $t1, $zero, 0   # subset cursor

  # loop through subsets
subset_loop:
  nop # LWXC2 $IDX, $t1($t0)
  nop # INITPC2
  nop # CALCPC2
  nop # WLSC2
  nop # POSVARC2
  nop # BIAS
  nop # CALCSS
  nop # SSVARC2
  nop # TSTGC2
  bgezl $zero, sv_local_test # BFDC2 sv_local_test
  la $a0, str_fault_ndet
  ori $v0, $zero, 4
  syscall
  j subset_loop_increment

  # run local test on satellites
sv_local_test:
  la $a0, str_fault_det
  ori $v0, $zero, 4
  syscall
  ori $t3, $zero, 0 # initial counter

  # loop through satellite vehicles
sv_local_test_loop:
  nop # MTC2 $t3, $TSTi
  nop # TSTLC2
  bgezl $zero, sv_local_test # BFLC2 faulty_sv_located
  la $a0, str_fault_nloc
  ori $v0, $zero, 4
  syscall
  j sv_local_test_loop_increment

faulty_sv_located:
  la $a0, str_fault_loc
  ori $v0, $zero, 4
  syscall

sv_local_test_loop_increment:
  slti $t4, $t3, 9 # t4 <- t3 < 9
  addi $t3, $t3, 1  # move cursor to next subset
  bnez $t4, sv_local_test_loop

  # increment cursor
subset_loop_increment:
  nop # NEWSSC2
  slti $t2, $t1, 4 # t2 <- t1 < 4
  addi $t1, $t1, 4  # move cursor to next subset
  bnez $t2, subset_loop

end:
  addi $v0, $zero, 10
  syscall
  j end
