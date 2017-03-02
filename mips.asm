###########################################################################
#                                                                         #
#                              data segment                               #
#                                                                         #
###########################################################################
.data
gc_x:          .word          0
gc_c1:         .word          97
gc_c2:         .word          122
_str0:         .asciiz        " = "
_str1:         .asciiz        "good!\n"

###########################################################################
#                                                                         #
#                              text segment                               #
#                                                                         #
###########################################################################
.text
subu $gp, $gp, 24
jal main                      # jump to main function
j _EOF                        # jump to the end of the program
_add:
subu $sp, $sp, 8              # decrement sp to make space to save ra, fp
sw $fp, 8($sp)                # save fp
sw $ra, 4($sp)                # save ra
addiu $fp, $sp, 8             # set up new fp
subu $sp, $sp, 8              # decrement sp to make space for locals/temps
addi $t7, $fp, 4              
beq $s6, $t7, _L6             
beq $s6, $zero, _L7           
sw $s1, 0($s6)                
_L7:
lw $s1, 4($fp)                
move $s6, $t7                 
_L6:
move $t8, $s1                 # $s1 = a
addi $t7, $fp, 8              
beq $s5, $t7, _L8             
beq $s5, $zero, _L9           
sw $s0, 0($s5)                
_L9:
lw $s0, 8($fp)                
move $s5, $t7                 
_L8:
move $t9, $s0                 # $s0 = b
add $t5, $t8, $t9             # $t5 = $t8 + $t9
sw $t5, -12($fp)              # _tmp0 = $t5
lw $t0, -12($fp)              # $t0 = _tmp0
sw $t0, -8($fp)               # t = $t0
lw $s3, -8($fp)               # $s3 = t
move $v0, $s3                 # assign return value into $v0
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
# (below handles reaching end of function body with no explicit return)
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
_fun:
subu $sp, $sp, 8              # decrement sp to make space to save ra, fp
sw $fp, 8($sp)                # save fp
sw $ra, 4($sp)                # save ra
addiu $fp, $sp, 8             # set up new fp
subu $sp, $sp, 8              # decrement sp to make space for locals/temps
li $s3, 'a'                   # $s3 = 'a'
sw $s3, -8($fp)               # _tmp1 = $s3
lw $t0, -8($fp)               # $t0 = _tmp1

move $t8, $t0                 # $t8 = $t0
li $s3, 5                     
move $t9, $s3                 # $s3 = 5
add $t5, $t8, $t9             # $t5 = $t8 + $t9
sw $t5, -12($fp)              # _tmp2 = $t5
lw $t1, -12($fp)              # $t1 = _tmp2
move $v0, $t1                 # assign return value into $v0
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
# (below handles reaching end of function body with no explicit return)
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
_empty:
subu $sp, $sp, 8              # decrement sp to make space to save ra, fp
sw $fp, 8($sp)                # save fp
sw $ra, 4($sp)                # save ra
addiu $fp, $sp, 8             # set up new fp
subu $sp, $sp, 0              # decrement sp to make space for locals/temps
# (below handles reaching end of function body with no explicit return)
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
_output:
subu $sp, $sp, 8              # decrement sp to make space to save ra, fp
sw $fp, 8($sp)                # save fp
sw $ra, 4($sp)                # save ra
addiu $fp, $sp, 8             # set up new fp
subu $sp, $sp, 0              # decrement sp to make space for locals/temps
addi $t7, $fp, 8              
beq $s6, $t7, _L10            
beq $s6, $zero, _L11          
sw $s1, 0($s6)                
_L11:
lw $s1, 8($fp)                
move $s6, $t7                 
_L10:
move $a0, $s1                 # $s1 = c
li $v0, 11                    # $v0 = 11 for syscall "PrintChar"
syscall                       
la $a0, _str0                 # $a0 = &(_str0)
li $v0, 4                     # $v0 = 4 for syscall "PrintString"
syscall                       
addi $t7, $fp, 4              
beq $s5, $t7, _L12            
beq $s5, $zero, _L13          
sw $s0, 0($s5)                
_L13:
lw $s0, 4($fp)                
move $s5, $t7                 
_L12:
move $a0, $s0                 # $s0 = n
li $v0, 1                     # $v0 = 1 for syscall "PrintInt"
syscall                       
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
# (below handles reaching end of function body with no explicit return)
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
main:
subu $sp, $sp, 8              # decrement sp to make space to save ra, fp
sw $fp, 8($sp)                # save fp
sw $ra, 4($sp)                # save ra
addiu $fp, $sp, 8             # set up new fp
subu $sp, $sp, 28             # decrement sp to make space for locals/temps
jal _empty
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L14            
li $s5, 0                     
_L14:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L15            
li $s6, 0                     
_L15:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L16            
li $s7, 0                     
_L16:                         
li $v0, 5                     # $v0 = 5 for syscall "ScanInt"
syscall                       
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L17            
li $s5, 0                     
_L17:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L18            
li $s6, 0                     
_L18:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L19            
li $s7, 0                     
_L19:                         
move $t9, $v0                 
addi $t7, $fp, -8             
beq $s6, $t7, _L20            
beq $s6, $zero, _L21          
sw $s1, 0($s6)                
_L21:
lw $s1, -8($fp)               
move $s6, $t7                 
_L20:
move $s1, $t9                 # i = $v0
addi $t7, $fp, -8             
beq $s6, $t7, _L22            
beq $s6, $zero, _L23          
sw $s1, 0($s6)                
_L23:
lw $s1, -8($fp)               
move $s6, $t7                 
_L22:
move $t8, $s1                 # $s1 = i
li $s3, 0                     
move $t9, $s3                 # $s3 = 0
beq $t8, $t9, _L1             # if $t8 == $t9 branch
li $s3, 0                     # $s3 = 0
move $t9, $s3                 
addi $t7, $fp, -8             
beq $s6, $t7, _L24            
beq $s6, $zero, _L25          
sw $s1, 0($s6)                
_L25:
lw $s1, -8($fp)               
move $s6, $t7                 
_L24:
move $s1, $t9                 # i = $s3
j _L0
_L1:
la $a0, _str1                 # $a0 = &(_str1)
li $v0, 4                     # $v0 = 4 for syscall "PrintString"
syscall                       
_L0:
addi $t7, $fp, -8             
beq $s6, $t7, _L26            
beq $s6, $zero, _L27          
sw $s1, 0($s6)                
_L27:
lw $s1, -8($fp)               
move $s6, $t7                 
_L26:
move $t8, $s1                 # $s1 = i
li $s3, 10                    
move $t9, $s3                 # $s3 = 10
bge $t8, $t9, _L3             # if $t8 >= $t9 branch
subu $sp, $sp, 4              # decrement sp to make space for param
addi $t7, $fp, -12            
beq $s5, $t7, _L28            
beq $s5, $zero, _L29          
sw $s0, 0($s5)                
_L29:
lw $s0, -12($fp)              
move $s5, $t7                 
_L28:# $s0 = s
sw $s0, 4($sp)                # copy param value to stack
subu $sp, $sp, 4              # decrement sp to make space for param
addi $t7, $fp, -8             
beq $s6, $t7, _L30            
beq $s6, $zero, _L31          
sw $s1, 0($s6)                
_L31:
lw $s1, -8($fp)               
move $s6, $t7                 
_L30:# $s1 = i
sw $s1, 4($sp)                # copy param value to stack
jal _add                      # call function _add
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L32            
li $s5, 0                     
_L32:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L33            
li $s6, 0                     
_L33:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L34            
li $s7, 0                     
_L34:                         
sw $v0, -16($fp)              # _tmp1 = $v0
add $sp, $sp, 8               # pop params off stack
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L35            
li $s5, 0                     
_L35:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L36            
li $s6, 0                     
_L36:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L37            
li $s7, 0                     
_L37:                         
lw $t0, -16($fp)              # $t0 = _tmp1
move $t9, $t0                 
addi $t7, $fp, -12            
beq $s5, $t7, _L38            
beq $s5, $zero, _L39          
sw $s0, 0($s5)                
_L39:
lw $s0, -12($fp)              
move $s5, $t7                 
_L38:
move $s0, $t9                 # s = $t0
addi $t7, $fp, -8             
beq $s6, $t7, _L40            
beq $s6, $zero, _L41          
sw $s1, 0($s6)                
_L41:
lw $s1, -8($fp)               
move $s6, $t7                 
_L40:
move $t8, $s1                 # $s1 = i
li $s3, 1                     
move $t9, $s3                 # $s3 = 1
add $t5, $t8, $t9             # $t5 = $t8 + $t9
sw $t5, -20($fp)              # _tmp3 = $t5
lw $t1, -20($fp)              # $t1 = _tmp3
move $t9, $t1                 
addi $t7, $fp, -8             
beq $s6, $t7, _L42            
beq $s6, $zero, _L43          
sw $s1, 0($s6)                
_L43:
lw $s1, -8($fp)               
move $s6, $t7                 
_L42:
move $s1, $t9                 # i = $t1
j _L0
_L3:
li $s3, 0                     # $s3 = 0
move $t9, $s3                 
addi $t7, $fp, -8             
beq $s6, $t7, _L44            
beq $s6, $zero, _L45          
sw $s1, 0($s6)                
_L45:
lw $s1, -8($fp)               
move $s6, $t7                 
_L44:
move $s1, $t9                 # i = $s3
_L4:
addi $t7, $fp, -8             
beq $s6, $t7, _L46            
beq $s6, $zero, _L47          
sw $s1, 0($s6)                
_L47:
lw $s1, -8($fp)               
move $s6, $t7                 
_L46:
move $t8, $s1                 # $s1 = i
li $s3, 5                     
move $t9, $s3                 # $s3 = 5
bge $t8, $t9, _L5             # if $t8 >= $t9 branch
addi $t7, $fp, -12            
beq $s5, $t7, _L48            
beq $s5, $zero, _L49          
sw $s0, 0($s5)                
_L49:
lw $s0, -12($fp)              
move $s5, $t7                 
_L48:
move $t8, $s0                 # $s0 = s
addi $t7, $fp, -12            
beq $s5, $t7, _L50            
beq $s5, $zero, _L51          
sw $s0, 0($s5)                
_L51:
lw $s0, -12($fp)              
move $s5, $t7                 
_L50:
move $t9, $s0                 # $s0 = s
mul $t5, $t8, $t9             # $t5 = $t8 * $t9
sw $t5, -24($fp)              # _tmp4 = $t5
addi $t7, $fp, -8             
beq $s6, $t7, _L52            
beq $s6, $zero, _L53          
sw $s1, 0($s6)                
_L53:
lw $s1, -8($fp)               
move $s6, $t7                 
_L52:
move $t8, $s1                 # $s1 = i
li $s3, 1                     
move $t9, $s3                 # $s3 = 1
add $t5, $t8, $t9             # $t5 = $t8 + $t9
sw $t5, -28($fp)              # _tmp5 = $t5
lw $t0, -24($fp)              # $t0 = _tmp4

move $t8, $t0                 # $t8 = $t0
lw $t1, -28($fp)              # $t1 = _tmp5

move $t9, $t1                 # $t9 = $t1
div $t5, $t8, $t9             # $t5 = $t8 / $t9
sw $t5, -32($fp)              # _tmp6 = $t5
lw $t0, -32($fp)              # $t0 = _tmp6

move $t8, $t0                 # $t8 = $t0
addi $t7, $fp, -8             
beq $s6, $t7, _L54            
beq $s6, $zero, _L55          
sw $s1, 0($s6)                
_L55:
lw $s1, -8($fp)               
move $s6, $t7                 
_L54:# $s1 = i
mul $t5, $s1, 4
addu $t6, $gp, $t5
sw $t8, 4($t6)                # g_y[i] = $t8
addi $t7, $fp, -8             
beq $s6, $t7, _L56            
beq $s6, $zero, _L57          
sw $s1, 0($s6)                
_L57:
lw $s1, -8($fp)               
move $s6, $t7                 
_L56:
move $t8, $s1                 # $s1 = i
li $s3, 1                     
move $t9, $s3                 # $s3 = 1
add $t5, $t8, $t9             # $t5 = $t8 + $t9
sw $t5, -24($fp)              # _tmp4 = $t5
lw $t1, -24($fp)              # $t1 = _tmp4
move $t9, $t1                 
addi $t7, $fp, -8             
beq $s6, $t7, _L58            
beq $s6, $zero, _L59          
sw $s1, 0($s6)                
_L59:
lw $s1, -8($fp)               
move $s6, $t7                 
_L58:
move $s1, $t9                 # i = $t1
j _L4
_L5:
li $s3, 's'                   # $s3 = 's'
sw $s3, -24($fp)              # _tmp4 = $s3
subu $sp, $sp, 4              # decrement sp to make space for param
lw $t0, -24($fp)              # $t0 = _tmp4
sw $t0, 4($sp)                # copy param value to stack
subu $sp, $sp, 4              # decrement sp to make space for param
addi $t7, $fp, -12            
beq $s5, $t7, _L60            
beq $s5, $zero, _L61          
sw $s0, 0($s5)                
_L61:
lw $s0, -12($fp)              
move $s5, $t7                 
_L60:# $s0 = s
sw $s0, 4($sp)                # copy param value to stack
jal _output
add $sp, $sp, 8               # pop params off stack
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L62            
li $s5, 0                     
_L62:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L63            
li $s6, 0                     
_L63:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L64            
li $s7, 0                     
_L64:                         
jal _fun                      # call function _fun
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L65            
li $s5, 0                     
_L65:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L66            
li $s6, 0                     
_L66:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L67            
li $s7, 0                     
_L67:                         
sw $v0, -28($fp)              # _tmp5 = $v0
# clear global register $s5 if it holds other function's value
bgt $s5, $sp, _L68            
li $s5, 0                     
_L68:                         
# clear global register $s6 if it holds other function's value
bgt $s6, $sp, _L69            
li $s6, 0                     
_L69:                         
# clear global register $s7 if it holds other function's value
bgt $s7, $sp, _L70            
li $s7, 0                     
_L70:                         
lw $t0, -28($fp)              # $t0 = _tmp5

move $a0, $t0                 # $a0 = $t0
li $v0, 1                     # $v0 = 1 for syscall "PrintInt"
syscall                       
# (below handles reaching end of function body with no explicit return)
move $sp, $fp                 # pop callee frame off stack
lw $ra, -4($fp)               # restore saved ra
lw $fp, 0($fp)                # restore saved fp
jr $ra                        # return from function
_EOF: 
li $v0, 10                    # $v0 = 10 for syscall "Exit"
syscall
## end of file
