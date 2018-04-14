.global GET
GET:
mov %rdi, %rax

mov %rdi, (%rax)
mov %rsi, 8(%rax)
mov %rbp, 16(%rax)
mov %rbx, 24(%rax)
mov %rdx, 32(%rax)
mov %rcx, 40(%rax)
movl $1,   48(%rax) #eax

mov (%rsp), %rcx
mov %rcx, 56(%rax) #eip
lea 8(%rsp), %rcx
mov %rcx, 64(%rax) #esp

mov 40(%rax), %rcx #restore
mov $0, %eax  #return 0
ret

.global SET
SET:
mov %rdi, %rax

mov (%rax), %rdi
mov 8(%rax), %rsi
mov 16(%rax), %rbp
mov 24(%rax), %rbx
mov 32(%rax), %rdx
mov 40(%rax), %rcx

mov 64(%rax), %rsp #restore esp
push 56(%rax) # restore eip
mov 48(%rax), %eax #return (restore rax)
ret  # excute eip 
