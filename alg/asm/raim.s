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
str_fault_loc:  .asciiz ": index of faulty SV!\n"
.space 1 # pad to 4-byte boundary
str_pausing:    .asciiz "Pausing\n"
.space 3 # pad to 4-byte boundary

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
  .word 0xc1187ae1 # y = -9.530000

data_sv1:
  .word 0x3f2ccccd # LOS_x = 0.675000
  .word 0xbf30a3d7 # LOS_y = -0.690000
  .word 0xbe85bc02 # LOS_z = -0.261200
  .word 0x00000000 # constellation
  .word 0x3e5261d9 # sig_tropo2 = 0.205451
  .word 0x3f2b77e9 # sig_user2 = 0.669798
  .word 0x3f6147ae # y = 0.880000

data_sv2:
  .word 0x3d941206 # LOS_x = 0.072300
  .word 0xbf28fc50 # LOS_y = -0.660100
  .word 0xbf3f6944 # LOS_z = -0.747700
  .word 0x00000000 # constellation
  .word 0x3cd2adba # sig_tropo2 = 0.025718
  .word 0x3e8b58bb # sig_user2 = 0.272161
  .word 0xbc23d70a # y = -0.010000

data_sv3:
  .word 0xbf7096bc # LOS_x = -0.939800
  .word 0x3e82b6ae # LOS_y = 0.255300
  .word 0xbe685879 # LOS_z = -0.226900
  .word 0x00000000 # constellation
  .word 0x3e8a1c12 # sig_tropo2 = 0.269745
  .word 0x3f4e5b71 # sig_user2 = 0.806083
  .word 0xbf028f5c # y = -0.510000

data_sv4:
  .word 0xbf17381d # LOS_x = -0.590700
  .word 0xbf40ff97 # LOS_y = -0.753900
  .word 0xbe934d6a # LOS_z = -0.287700
  .word 0x00000000 # constellation
  .word 0x3e2e4d6b # sig_tropo2 = 0.170217
  .word 0x3f17152d # sig_user2 = 0.590167
  .word 0xbee66666 # y = -0.450000

data_sv5:
  .word 0xbea5aee6 # LOS_x = -0.323600
  .word 0xbd10ff97 # LOS_y = -0.035400
  .word 0xbf720c4a # LOS_z = -0.945500
  .word 0x00000001 # constellation
  .word 0x3c83eabf # sig_tropo2 = 0.016103
  .word 0x3e878d6a # sig_user2 = 0.264751
  .word 0xbf87ae14 # y = -1.060000

data_sv6:
  .word 0xbf2cbfb1 # LOS_x = -0.674800
  .word 0x3edf06f7 # LOS_y = 0.435600
  .word 0xbf187fcc # LOS_z = -0.595700
  .word 0x00000001 # constellation
  .word 0x3d259b2c # sig_tropo2 = 0.040431
  .word 0x3e963948 # sig_user2 = 0.293406
  .word 0x40028f5c # y = 2.040000

data_sv7:
  .word 0x3dc01a37 # LOS_x = 0.093800
  .word 0xbf334d6a # LOS_y = -0.700400
  .word 0xbf351eb8 # LOS_z = -0.707500
  .word 0x00000001 # constellation
  .word 0x3ceb2dc7 # sig_tropo2 = 0.028708
  .word 0x3e8d2598 # sig_user2 = 0.275677
  .word 0xbf5c28f6 # y = -0.860000

data_sv8:
  .word 0x3f0e9e1b # LOS_x = 0.557100
  .word 0x3e9e1b09 # LOS_y = 0.308800
  .word 0xbf4559b4 # LOS_z = -0.770900
  .word 0x00000001 # constellation
  .word 0x3cc63a90 # sig_tropo2 = 0.024198
  .word 0x3e8a885d # sig_user2 = 0.270572
  .word 0x404ccccd # y = 3.200000

data_sv9:
  .word 0x3f2985f0 # LOS_x = 0.662200
  .word 0x3f321ff3 # LOS_y = 0.695800
  .word 0xbe8e5604 # LOS_z = -0.278000
  .word 0x00000001 # constellation
  .word 0x3e3a577c # sig_tropo2 = 0.181974
  .word 0x3f1dfb07 # sig_user2 = 0.617112
  .word 0x40666666 # y = 3.600000

data_subsets:
  .word 1023 # all-in-view
  .word 31   # 5 first satellites
  .word 992  # 5 last satellites

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
  nop # LWC2 $Y15, 24($t1)
  nop # LWC2 $BN, 0($t0)
  nop # NEWSVC2
  slt $t3, $t1, $t2 # t3 <- t1 < t2
  addi $t1, $t1, 28 # move cursor to next SV
  bnez $t3, load_loop

  #################################
  ##### calculate LS matrices #####
  #################################

  # all-in-view
  nop # CALCUC2
  ori $t0, $zero, 1023
  nop # MTC2 $t0, $IDX
  nop # INITPC2 $zero
  nop # CALCPC2 $zero
  nop # WLSC2 $zero
aiv_wait_wls:
  bgezl $zero, aiv_wait_wls # BMMC2 aiv_wait_wls
  nop # NEWSSC2

subset_loop_init:
  # cursors
  la $t0, data_subsets # initial subset
  addi $t1, $zero, 4   # subset cursor

  # loop through subsets
subset_loop:
  # compute least-squares matrix
  nop # LWXC2 $IDX, $t1($t0)
  nop # INITPC2 $t1
  nop # CALCPC2 $t1
  nop # WLSC2 $t1
ss_wait_wls:
  bgezl $zero, ss_wait_wls # BMMC2 ss_wait_wls

  # compute statistics
  nop # POSVARC2 $t1
  nop # BIAS $t1
  nop # CALCSS $t1
  nop # SSVARC2 $t1
ss_wait_var:
  bgezl $zero, ss_wait_var # BMMC2 ss_wait_var
  nop # MULY $t1 0
  nop # TSTGC2 $t1

  la $a0, str_pausing
  ori $v0, $zero, 4
  syscall

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
  nop # TSTLC2 $t3, $I

  la $a0, str_pausing
  ori $v0, $zero, 4
  syscall

  bgezl $zero, faulty_sv_located # BFLC2 faulty_sv_located
  la $a0, str_fault_nloc
  ori $v0, $zero, 4
  syscall
  j sv_local_test_loop_increment

faulty_sv_located:
  addi $a0, $t3, 0
  ori $v0, $zero, 1
  syscall
  la $a0, str_fault_loc
  ori $v0, $zero, 4
  syscall

sv_local_test_loop_increment:
  slti $t4, $t3, 9 # t4 <- t3 < 9
  addi $t3, $t3, 1 # move cursor to next subset
  bnez $t4, sv_local_test_loop

  # increment cursor
subset_loop_increment:
  nop # NEWSSC2
  slti $t2, $t1, 8 # t2 <- t1 < 8
  addi $t1, $t1, 4  # move cursor to next subset
  bnez $t2, subset_loop

end:
  addi $v0, $zero, 10
  syscall
  j end
